//
// Created by stephane bourque on 2021-07-21.
//

#include "Dashboard.h"
#include "StorageService.h"
#include "framework/utils.h"

namespace OpenWifi {
	void DeviceDashboard::Generate(GWObjects::Dashboard &D) {
		if (GeneratingDashboard_.load()) {
			while(GeneratingDashboard_.load()) {
				Poco::Thread::trySleep(100);
			}
			std::lock_guard	G(DataMutex_);
			D = DB_;
		} else {
			GeneratingDashboard_ = true;
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
			GeneratingDashboard_ = false;
		}
	}
}
