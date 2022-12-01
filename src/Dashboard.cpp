//
// Created by stephane bourque on 2021-07-21.
//

#include "Dashboard.h"
#include "StorageService.h"
#include "framework/utils.h"

namespace OpenWifi {
	void DeviceDashboard::Generate(GWObjects::Dashboard &D) {
		if (std::atomic_flag_test_and_set(&GeneratingDashboard_)) {
			while(std::atomic_flag_test(&GeneratingDashboard_)) {
				Poco::Thread::trySleep(100);
			}
			std::lock_guard	G(DataMutex_);
			D = DB_;
		} else {
			ValidDashboard_ = false;
			try {
				GWObjects::Dashboard	NewData;
				StorageService()->AnalyzeCommands(NewData.commands);
				StorageService()->AnalyzeDevices(NewData);
				LastRun_ = Utils::Now();
				D = NewData;
				std::lock_guard	G(DataMutex_);
				DB_ = NewData;
				ValidDashboard_=true;
			} catch(...) {

			}
			std::atomic_flag_clear(&GeneratingDashboard_);
		}
	}
}
