//
// Created by stephane bourque on 2021-07-12.
//

#include <fstream>

#include "StorageArchiver.h"
#include "StorageService.h"

#include "framework/AppServiceRegistry.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/utils.h"

#include "fmt/format.h"

namespace OpenWifi {

	void Archiver::onTimer([[maybe_unused]] Poco::Timer &timer) {
		Utils::SetThreadName("strg-archiver");
		auto now = Utils::Now();
		for (const auto &[DBName, Keep] : DBs_) {
			if (!Poco::icompare(DBName, "healthchecks")) {
				poco_information(Logger(), "Archiving HealthChecks...");
				StorageService()->RemoveHealthChecksRecordsOlderThan(now - (Keep * 24 * 60 * 60));
			} else if (!Poco::icompare(DBName, "statistics")) {
				poco_information(Logger(), "Archiving Statistics...");
				StorageService()->RemoveStatisticsRecordsOlderThan(now - (Keep * 24 * 60 * 60));
			} else if (!Poco::icompare(DBName, "devicelogs")) {
				poco_information(Logger(), "Archiving Device Logs...");
				StorageService()->RemoveDeviceLogsRecordsOlderThan(now - (Keep * 24 * 60 * 60));
			} else if (!Poco::icompare(DBName, "commandlist")) {
				poco_information(Logger(), "Archiving Command History...");
				StorageService()->RemoveCommandListRecordsOlderThan(now - (Keep * 24 * 60 * 60));
			} else if (!Poco::icompare(DBName, "fileuploads")) {
				poco_information(Logger(), "Archiving Upload files...");
				StorageService()->RemoveUploadedFilesRecordsOlderThan(now - (Keep * 24 * 60 * 60));
			} else {
				poco_information(Logger(), fmt::format("Cannot archive DB '{}'", DBName));
			}
		}
		AppServiceRegistry().Set("lastStorageArchiverRun", (uint64_t)now);
	}

	static auto CalculateDelta(std::uint64_t H, std::uint64_t M) {
		Poco::LocalDateTime dt;
		Poco::LocalDateTime scheduled(dt.year(), dt.month(), dt.day(), (int)H, (int)M, 0);

		std::uint64_t delta = 0;
		if ((dt.hour() < (int)H) || (dt.hour() == (int)H && dt.minute() < (int)M)) {
			delta = scheduled.timestamp().epochTime() - dt.timestamp().epochTime();
		} else {
			delta =
				(24 * 60 * 60) - (dt.timestamp().epochTime() - scheduled.timestamp().epochTime());
		}
		return delta;
	}

	int StorageArchiver::Start() {

		Enabled_ = MicroServiceConfigGetBool("archiver.enabled", false);
		if (!Enabled_) {
			poco_information(Logger(), "Archiver is disabled.");
			return 0;
		}

		Archiver_ = std::make_unique<Archiver>(Logger());
		ArchiverCallback_ =
			std::make_unique<Poco::TimerCallback<Archiver>>(*Archiver_, &Archiver::onTimer);

		auto Schedule = MicroServiceConfigGetString("archiver.schedule", "03:00");
		auto S = Poco::StringTokenizer(Schedule, ":");

		std::uint64_t RunAtHour_, RunAtMin_;
		if (S.count() != 2) {
			RunAtHour_ = 3;
			RunAtMin_ = 0;
		} else {
			RunAtHour_ = std::strtoull(S[0].c_str(), nullptr, 10);
			RunAtMin_ = std::strtoull(S[1].c_str(), nullptr, 10);
		}

		for (int i = 0; i < 20; i++) {
			std::string key = "archiver.db." + std::to_string(i) + ".name";
			auto DBName = MicroServiceConfigGetString(key, "");
			if (!DBName.empty()) {
				for (auto const &DB : AllInternalDBNames) {
					if (Poco::icompare(DBName, DB) == 0) {
						std::string Key = "archiver.db." + std::to_string(i) + ".keep";
						auto Keep = MicroServiceConfigGetInt(Key, 7);
						Archiver_->AddDb(DB, Keep);
					}
				}
			}
		}

		int NextRun = CalculateDelta(RunAtHour_, RunAtMin_);

		poco_information(Logger(), fmt::format("Next run in {} seconds.", NextRun));

		Timer_.setStartInterval(NextRun * 1000);
		Timer_.setPeriodicInterval(24 * 60 * 60 * 1000); // 1 hours
		Timer_.start(*ArchiverCallback_, MicroServiceTimerPool());

		auto F = MicroServiceConfigGetString("example.test" , "none");
		std::cout << Poco::Path::expand(F) << std::endl;

		return 0;
	}

	void StorageArchiver::Stop() {
		poco_information(Logger(), "Stopping...");
		if (Enabled_) {
			Timer_.stop();
		}
		poco_information(Logger(), "Stopped...");
	}

}; // namespace OpenWifi
