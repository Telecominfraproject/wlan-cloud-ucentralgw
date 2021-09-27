//
// Created by stephane bourque on 2021-07-12.
//

#include <fstream>

#include "StorageArchiver.h"
#include "StorageService.h"

#include "Daemon.h"
#include "Utils.h"

namespace OpenWifi {

	class StorageArchiver *StorageArchiver::instance_ = nullptr;

	void StorageArchiver::run() {
		Running_ = true ;
		GetLastRun();
		while(Running_) {
			Poco::Thread::trySleep(2000);
			Poco::DateTime	Now;

			if(!Running_)
				break;

			if(LastRun_!=Now.day()) {
				if(Now.hour()>=RunAtHour_ && Now.minute()>=RunAtMin_) {
					std::lock_guard G(Mutex_);
					for(const auto &i:DBs_) {
						if (!Poco::icompare(i.DBName, "healthchecks")) {
							Logger_.information("Archiving HealthChecks...");
							Storage()->RemoveHealthChecksRecordsOlderThan(
								std::time(nullptr) - (i.HowManyDays * 24 * 60 * 60));
						} else if (!Poco::icompare(i.DBName, "statistics")) {
							Logger_.information("Archiving Statistics...");
							Storage()->RemoveStatisticsRecordsOlderThan(
								std::time(nullptr) - (i.HowManyDays * 24 * 60 * 60));
						} else if (!Poco::icompare(i.DBName, "devicelogs")) {
							Logger_.information("Archiving Device Logs...");
							Storage()->RemoveDeviceLogsRecordsOlderThan(
								std::time(nullptr) - (i.HowManyDays * 24 * 60 * 60));
						} else if (!Poco::icompare(i.DBName, "commandlist")) {
							Logger_.information("Archiving Command History...");
							Storage()->RemoveCommandListRecordsOlderThan(
								std::time(nullptr) - (i.HowManyDays * 24 * 60 * 60));
						} else {
						}
					}
					LastRun_ = Now.day();
					SetLastRun();
				}
			}
		}
	}

	void StorageArchiver::GetLastRun() {
		try {
			Poco::File	LR(LastRunFileName_);
			try {
				if (LR.exists()) {
					std::ifstream I;
					I.open(LR.path());
					I >> LastRun_;
					I.close();
				} else {
					LastRun_ = 0;
				}
			} catch(...) {
				LastRun_=0;
			}
		} catch(...) {
			LastRun_=0;
		}
	}

	void StorageArchiver::SetLastRun() {
		try {
			std::ofstream O;
			O.open(LastRunFileName_,std::ios::binary | std::ios::trunc);
			O << LastRun_;
			O.close();
		} catch(const Poco::Exception &E) {
			Logger_.log(E);
		}
	}

	int StorageArchiver::Start() {

		Enabled_ = Daemon()->ConfigGetBool("archiver.enabled",false);

		if(!Enabled_) {
			Logger_.information("Archiver is disabled.");
			return 0;
		}

		auto Schedule = Daemon()->ConfigGetString("archiver.schedule","03:00");
		Types::StringVec S = Utils::Split(Schedule,':');
		RunAtHour_ = std::atoi(S[0].c_str());
		RunAtMin_ = std::atoi(S[1].c_str());

		for(int i=0;i<20;i++) {
			std::string key = "archiver.db." + std::to_string(i) + ".name";
			auto DBName = Daemon()->ConfigGetString(key,"");
			if(!DBName.empty()) {
				for(auto const &DB:AllInternalDBNames) {
					if(Poco::icompare(DBName,DB)==0) {
						std::string Key = "archiver.db." + std::to_string(i) + ".keep";
						auto Keep = Daemon()->ConfigGetInt(Key,7);
						DBs_.push_back(ArchiverDBEntry{
							.DBName = DB,
							.HowManyDays = Keep
						});
					}
				}
			}
		}
		LastRunFileName_ = Daemon()->DataDir() + "/archiver_lastrun.txt";
		Janitor_.start(*this);
		return 0;
	}

	void StorageArchiver::Stop() {

		if(Running_) {
			Running_=false;
			Janitor_.wakeUp();
			Janitor_.join();
		}
	}

};
