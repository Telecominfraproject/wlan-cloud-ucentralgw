//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

namespace OpenWifi {

	int Storage::Start() {
		std::lock_guard Guard(Mutex_);
		StorageClass::Start();

		Create_Tables();
		InitializeBlackListCache();

		ScriptDB_ =
			std::make_unique<OpenWifi::ScriptDB>("Scripts", "scr", dbType_, *Pool_, Logger());
		ScriptDB_->Create();
		ScriptDB_->Initialize();

		return 0;
	}

	void Storage::Stop() {
		std::lock_guard Guard(Mutex_);
		poco_notice(Logger(), "Stopping...");
		StorageClass::Stop();
		poco_notice(Logger(), "Stopped...");
	}
} // namespace OpenWifi
  // namespace