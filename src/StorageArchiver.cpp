//
// Created by stephane bourque on 2021-07-12.
//

#include <fstream>

#include "StorageArchiver.h"
#include "StorageService.h"
#include "framework/MicroService.h"

namespace OpenWifi {

	void Archiver::onTimer(Poco::Timer &timer){
		auto Now = std::time(nullptr);
		for(const auto &i:DBs_) {
			if (!Poco::icompare(i.DBName, "healthchecks")) {
				Logger().information("Archiving HealthChecks...");
				StorageService()->RemoveHealthChecksRecordsOlderThan(
					Now - (i.HowManyDays * 24 * 60 * 60));
			} else if (!Poco::icompare(i.DBName, "statistics")) {
				Logger().information("Archiving Statistics...");
				StorageService()->RemoveStatisticsRecordsOlderThan(
					Now - (i.HowManyDays * 24 * 60 * 60));
			} else if (!Poco::icompare(i.DBName, "devicelogs")) {
				Logger().information("Archiving Device Logs...");
				StorageService()->RemoveDeviceLogsRecordsOlderThan(
					Now - (i.HowManyDays * 24 * 60 * 60));
			} else if (!Poco::icompare(i.DBName, "commandlist")) {
				Logger().information("Archiving Command History...");
				StorageService()->RemoveCommandListRecordsOlderThan(
					Now - (i.HowManyDays * 24 * 60 * 60));
			} else {
				Logger().information(Poco::format("Cannot archive DB '%s'", i.DBName));
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
			Logger().information("Archiver is disabled.");
			return 0;
		}

		ArchiverCallback_ = std::make_unique<Poco::TimerCallback<Archiver>>(Archiver_,&Archiver::onTimer);

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
						Archiver_.AddDb(Archiver::ArchiverDBEntry{
							.DBName = DB,
							.HowManyDays = Keep
						});
					}
				}
			}
		}

		int NextRun = CalculateDelta(RunAtHour_,RunAtMin_);

		Logger().information(Poco::format("Next run in %d seconds.",NextRun));

		Timer_.setStartInterval( NextRun * 1000);
		Timer_.setPeriodicInterval(24 * 60 * 60 * 1000); // 1 hours
		Timer_.start(*ArchiverCallback_);

		return 0;
	}

	void StorageArchiver::Stop() {
		if(Enabled_) {
			Timer_.stop();
		}
	}

};
