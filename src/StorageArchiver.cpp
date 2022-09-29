//
// Created by stephane bourque on 2021-07-12.
//

#include <fstream>

#include "framework/MicroService.h"

#include "StorageArchiver.h"
#include "StorageService.h"

namespace OpenWifi {

	void Archiver::onTimer([[maybe_unused]] Poco::Timer &timer){
		Utils::SetThreadName("strg-archiver");
		auto now = OpenWifi::Now();
		for(const auto &i:DBs_) {
			if (!Poco::icompare(i.DBName, "healthchecks")) {
				poco_information(Logger(),"Archiving HealthChecks...");
				StorageService()->RemoveHealthChecksRecordsOlderThan(
					now - (i.HowManyDays * 24 * 60 * 60));
			} else if (!Poco::icompare(i.DBName, "statistics")) {
				poco_information(Logger(),"Archiving Statistics...");
				StorageService()->RemoveStatisticsRecordsOlderThan(
					now - (i.HowManyDays * 24 * 60 * 60));
			} else if (!Poco::icompare(i.DBName, "devicelogs")) {
				poco_information(Logger(),"Archiving Device Logs...");
				StorageService()->RemoveDeviceLogsRecordsOlderThan(
					now - (i.HowManyDays * 24 * 60 * 60));
			} else if (!Poco::icompare(i.DBName, "commandlist")) {
				poco_information(Logger(),"Archiving Command History...");
				StorageService()->RemoveCommandListRecordsOlderThan(
					now - (i.HowManyDays * 24 * 60 * 60));
			} else {
				poco_information(Logger(),fmt::format("Cannot archive DB '{}'", i.DBName));
			}
		}
		AppServiceRegistry().Set("lastStorageArchiverRun", (uint64_t) Now);
	}

	static auto CalculateDelta(int H, int M) {
		Poco::LocalDateTime dt;
		Poco::LocalDateTime scheduled(dt.year(), dt.month(), dt.day(), H, M, 0);

		size_t delta = 0;
		if ((dt.hour() < H) || (dt.hour()==H && dt.minute()<M)) {
			delta = scheduled.timestamp().epochTime() - dt.timestamp().epochTime();
		} else {
			delta = (24*60*60) - (dt.timestamp().epochTime() - scheduled.timestamp().epochTime());
		}
		return delta;
	}

	int StorageArchiver::Start() {

		Enabled_ = MicroService::instance().ConfigGetBool("archiver.enabled",false);
		if(!Enabled_) {
			poco_information(Logger(),"Archiver is disabled.");
			return 0;
		}

		Archiver_ = std::make_unique<Archiver>(Logger());
		ArchiverCallback_ = std::make_unique<Poco::TimerCallback<Archiver>>(*Archiver_,&Archiver::onTimer);

		auto Schedule = MicroService::instance().ConfigGetString("archiver.schedule","03:00");
		auto S = Poco::StringTokenizer(Schedule,":");

		int RunAtHour_, RunAtMin_;
		if(S.count()!=2) {
			RunAtHour_ = 3 ;
			RunAtMin_ = 0;
		} else {
			RunAtHour_ = std::atoi(S[0].c_str());
			RunAtMin_ = std::atoi(S[1].c_str());
		}

		for(int i=0;i<20;i++) {
			std::string key = "archiver.db." + std::to_string(i) + ".name";
			auto DBName = MicroService::instance().ConfigGetString(key,"");
			if(!DBName.empty()) {
				for(auto const &DB:AllInternalDBNames) {
					if(Poco::icompare(DBName,DB)==0) {
						std::string Key = "archiver.db." + std::to_string(i) + ".keep";
						auto Keep = MicroService::instance().ConfigGetInt(Key,7);
						Archiver_->AddDb(Archiver::ArchiverDBEntry{
							.DBName = DB,
							.HowManyDays = Keep
						});
					}
				}
			}
		}

		int NextRun = CalculateDelta(RunAtHour_,RunAtMin_);

		poco_information(Logger(),fmt::format("Next run in {} seconds.",NextRun));

		Timer_.setStartInterval( NextRun * 1000);
		Timer_.setPeriodicInterval(24 * 60 * 60 * 1000); // 1 hours
		Timer_.start(*ArchiverCallback_, MicroService::instance().TimerPool());

		return 0;
	}

	void StorageArchiver::Stop() {
		poco_information(Logger(),"Stopping...");
		if(Enabled_) {
			Timer_.stop();
		}
		poco_information(Logger(),"Stopped...");
	}

};
