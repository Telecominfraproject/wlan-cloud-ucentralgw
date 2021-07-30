//
// Created by stephane bourque on 2021-07-21.
//

#include "Dashboard.h"
#include "DeviceRegistry.h"
#include "StorageService.h"

namespace uCentral {
	void DeviceDashboard::Create() {
		uint64_t Now = std::time(nullptr);

		if(LastRun_==0 || (Now-LastRun_)>120) {
			DB_.reset();
			Storage()->AnalyzeCommands(DB_.commands);
			// DeviceRegistry()->AnalyzeRegistry(DB_);
			Storage()->AnalyzeDevices(DB_);
			LastRun_ = Now;
		}
	}
}
