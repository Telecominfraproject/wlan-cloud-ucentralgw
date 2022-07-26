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
		std::lock_guard		Guard(Mutex_);
		StorageClass::Start();

		Create_Tables();
        InitializeBlackListCache();

		return 0;
    }

    void Storage::Stop() {
    	std::lock_guard		Guard(Mutex_);
        Logger().notice("Stopping...");
		StorageClass::Stop();
    }
}
// namespace