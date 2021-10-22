//
// Created by stephane bourque on 2021-07-21.
//

#include "Dashboard.h"
#include "DeviceRegistry.h"
#include "StorageService.h"

namespace OpenWifi {
	void DeviceDashboard::Create() {
		uint64_t Now = std::time(nullptr);

		if(LastRun_==0 || (Now-LastRun_)>120) {
			DB_.reset();
			StorageService()->AnalyzeCommands(DB_.commands);
			StorageService()->AnalyzeDevices(DB_);
			LastRun_ = Now;
		}
	}
}
