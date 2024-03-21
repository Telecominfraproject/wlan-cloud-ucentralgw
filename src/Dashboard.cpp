//
// Created by stephane bourque on 2021-07-21.
//

#include "Dashboard.h"
#include "StorageService.h"
#include "framework/utils.h"

namespace OpenWifi {

	bool DeviceDashboard::Get(GWObjects::Dashboard &D, Poco::Logger &Logger) {
		uint64_t Now = Utils::Now();
		if (!ValidDashboard_ || LastRun_ == 0 || (Now - LastRun_) > 120) {
			Generate(D, Logger);
		} else {
			std::lock_guard G(DataMutex_);
			D = DB_;
		}
		return ValidDashboard_;
	};

	void DeviceDashboard::Generate(GWObjects::Dashboard &D, Poco::Logger &Logger) {
		if (GeneratingDashboard_.load()) {
			while (GeneratingDashboard_.load()) {
				Poco::Thread::trySleep(100);
			}
			std::lock_guard G(DataMutex_);
			D = DB_;
		} else {
			GeneratingDashboard_ = true;
			ValidDashboard_ = false;
			try {
				poco_information(Logger, "DASHBOARD: Generating a new dashboard.");
				GWObjects::Dashboard NewData;
				StorageService()->AnalyzeCommands(NewData.commands);
				StorageService()->AnalyzeDevices(NewData);
				LastRun_ = Utils::Now();
				NewData.snapshot = LastRun_;
				D = NewData;
				std::lock_guard G(DataMutex_);
				DB_ = NewData;
				ValidDashboard_ = true;
			} catch (...) {
			}
			GeneratingDashboard_ = false;
		}
	}
} // namespace OpenWifi
