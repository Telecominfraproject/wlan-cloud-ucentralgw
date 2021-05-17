//
// Created by stephane bourque on 2021-05-12.
//

#include <iostream>
#include <fstream>

#include "uFirmwareManager.h"

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/StringTokenizer.h"
#include "Poco/LocalDateTime.h"

#include "uCentral.h"
#include "uStorageService.h"
#include "uUtils.h"

namespace uCentral::FirmwareManager {
	Service *Service::instance_ = nullptr;

	Service::Service() noexcept: uSubSystemServer("FirmwareManager", "FW-MGR", "firmware")
	{
	}

	int Start() {
		return Service::instance()->Start();
	}

	void Stop() {
		Service::instance()->Stop();
	}

	bool SetManifest(const std::string & Manifest) {
		return Service::instance()->SetManifest(Manifest);
	}

	int Service::Start() {
		Logger_.notice("Starting...");

		ManifestFileName_ = uCentral::instance()->DataDir() + "/firmware_manifest.json";
		DefaultPolicy_= uCentral::ServiceConfig::GetString("firmware.autoupdate.policy.default","auto");

		std::string Manifest;
		{
			std::ifstream File(ManifestFileName_, std::ifstream::in);
			while (File.good()) {
				std::string L;
				File >> L;
				Manifest += L;
			}
		}

		SetManifest(Manifest);
		Mgr_.start(*this);

		return 0;
	}

	void Service::run() {
		Running_ = true;

		uint64_t LastContact = time(nullptr);

		bool FirstRun = true;

		while(Running_) {
			if(!FirstRun)
				Poco::Thread::trySleep(20000);
			FirstRun = false;

			for(auto &Entry:Firmwares_) {
				std::vector<std::string>	SerialNumbers;

				if(uCentral::Storage::GetDevicesWithoutFirmware(Entry.second.Compatible,Entry.second.Version,SerialNumbers)
					&& !SerialNumbers.empty()) {
						DoUpgrade(Entry.second,SerialNumbers);
				}
			}
		}
	}

	//	schedule string is something like this
	//	daily@time				: daily@03:00    daily @3am
	//	weekly@<dow>@time		: weekly@5@2:00	 every week, on Saturday@2am 0=Monday.
	//	monthly@<dom>@time		: monthly@15@4:00 every month on the 15th @ 4am
	//	date@date@time			: date@2021-05-21@5:00
	uint64_t Service::CalculateWhen(std::string &SerialNumber) {

		Poco::LocalDateTime	Now;

		int Hours, Minutes, Seconds=0;
		int Year, Month, Day = 0 ;

		std::string UpdatePolicy;
		if(!uCentral::Storage::GetDeviceFWUpdatePolicy(SerialNumber, UpdatePolicy) ||
			UpdatePolicy.empty())
			UpdatePolicy = DefaultPolicy_;

		if(UpdatePolicy=="auto")
			return 0;

		if(UpdatePolicy=="off")
			return std::numeric_limits<uint64_t>::max();

		try {
			Poco::StringTokenizer Tokens(UpdatePolicy, "@", Poco::StringTokenizer::TOK_TRIM);
			if (Tokens.count() == 2 && Tokens[0] == "daily") {
				uCentral::Utils::ParseTime(Tokens[1], Hours, Minutes, Seconds);
				Poco::LocalDateTime T(Now.year(), Now.month(), Now.day(), Hours, Minutes, Seconds);
				if (T > Now)
					return T.timestamp().epochTime();
				//	Time tomorrow...
				return T.timestamp().epochTime() + (24 * 60 * 60);
			} else if (Tokens.count() == 3) {
				uCentral::Utils::ParseTime(Tokens[2], Hours, Minutes, Seconds);
				if (Tokens[0] == "date") {
					uCentral::Utils::ParseDate(Tokens[1], Year, Month, Day);
					Poco::LocalDateTime T(Year, Month, Day, Hours, Minutes, Seconds);
					return T.timestamp().epochTime();
				} else if (Tokens[0] == "monthly") {
					int DOM = std::stoi(Tokens[1]);
					if (Now.day() < DOM ||
						(Now.day() == DOM &&
						 uCentral::Utils::CompareTime(Now.hour(), Hours, Now.minute(), Minutes,
													  Now.second(), Seconds))) {
						Poco::LocalDateTime T(Now.year(), Now.month(), DOM, Hours, Minutes,
											  Seconds);
						return T.timestamp().epochTime();
					}
					// we must add a month.
					Poco::LocalDateTime T(Now.month() == 12 ? Now.year() + 1 : Now.year(),
										  Now.month() == 12 ? 1 : Now.month() + 1, DOM, Hours,
										  Minutes, Seconds);
					return T.timestamp().epochTime();
				} else if (Tokens[0] == "weekly") {
					auto DOW = std::stol(Tokens[1]);
					//	this is today, we just need to set to the configured hour.
					if (DOW == Now.dayOfWeek() &&
						uCentral::Utils::CompareTime(Now.hour(), Hours, Now.minute(), Minutes,
													 Now.second(), Seconds)) {
						Poco::LocalDateTime T(Now.year(), Now.month(), Now.day(), Hours, Minutes,
											  Seconds);
						return T.timestamp().epochTime();
					}
					//	this is ahead, just add the number of seconds in days to today's date at the config time.
					if (DOW > Now.dayOfWeek()) {
						Poco::LocalDateTime T(Now.year(), Now.month(), Now.day(), Hours, Minutes,
											  Seconds);
						return T.timestamp().epochTime() + ((DOW - Now.dayOfWeek()) * 24 * 60 * 60);
					}
					Poco::LocalDateTime T(Now.year(), Now.month(), Now.day(), Hours, Minutes,
										  Seconds);
					return T.timestamp().epochTime() + (7 - (Now.dayOfWeek() - DOW) * 24 * 60 * 60);
				}
			}
		} catch(const Poco::Exception &E) {
			Logger_.log(E);
		}
		return 0;
	}

	bool Service::DoUpgrade(const FirmwareEntry &Firmware, const std::vector<std::string> &SerialNumbers) {

		for(auto i:SerialNumbers) {
			uCentral::Objects::CommandDetails  Cmd;

			uint64_t When = CalculateWhen(i);

			//	do we need to skip the update for this device???
			if(When == std::numeric_limits<uint64_t>::max())
				continue;

			Cmd.SerialNumber = i;
			Cmd.UUID = uCentral::instance()->CreateUUID();
			Cmd.SubmittedBy = "ucentralfws";
			Cmd.Custom = 0;
			Cmd.Command = "upgrade";
			Cmd.RunAt = CalculateWhen(i);
			Cmd.WaitingForFile = 0;

			Poco::JSON::Object  Params;

			Params.set( "serial" , i );
			Params.set( "uri", Firmware.URI);
			Params.set( "when", 0);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			uCentral::Storage::AddCommand(i,Cmd);

			Logger_.information(Poco::format("UPGRADING(%s): to %s",i,Firmware.Version));
		}

		return true;
	}


void Service::Stop() {
		SubMutexGuard Guard(Mutex_);

		Logger_.notice("Stopping...");
		Running_ = false ;
		Mgr_.wakeUp();
		Mgr_.join();
	}

	bool Service::SetManifest(const std::string & Manifest){
		SubMutexGuard Guard(Mutex_);
		if(Manifest != Manifest_) {
			Manifest_ = Manifest;

			std::ofstream File(ManifestFileName_, std::ofstream::out | std::ofstream::trunc);
			File << Manifest_;
			File.close();

			ParseManifest();
			if(Running_)
				Mgr_.wakeUp();
		}
		return true;
	}

	bool Service::ParseManifest() {
		SubMutexGuard Guard(Mutex_);

		Poco::JSON::Parser parser;
		Poco::JSON::Object Obj = parser.parse(Manifest_).extract<Poco::JSON::Object>();
		Poco::DynamicStruct ds = Obj;

		Poco::JSON::Array	Elements = ds["firmwares"].extract<Poco::JSON::Array>();

		Firmwares_.clear();

		//	Pn.y keep the latest...
		for(const auto &i:Elements) {
			if(i["latest"]) {
				FirmwareEntry F{.Compatible = i["compatible"].toString(),
								.Uploader = i["uploader"].toString(),
								.Version = i["version"].toString(),
								.URI = i["uri"].toString(),
								.Uploaded = uCentral::Utils::from_RFC3339(i["uploaded"].toString()),
								.Size = i["size"],
								.FirmwareDate = uCentral::Utils::from_RFC3339(i["date"].toString()),
								.Latest = i["latest"]};

				Firmwares_[F.Compatible]=F;
			}
		}

		return true;
	}

}  // end of namespace
