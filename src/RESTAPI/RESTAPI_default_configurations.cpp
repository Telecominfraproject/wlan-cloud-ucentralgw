//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"

#include "RESTAPI_default_configurations.h"
#include "StorageService.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void RESTAPI_default_configurations::DoGet() {

		if (QB_.CountOnly) {
			auto Count = StorageService()->GetDefaultConfigurationsCount();
			return ReturnCountOnly(Count);
		}

		std::vector<GWObjects::DefaultConfiguration> DefConfigs;
		StorageService()->GetDefaultConfigurations(QB_.Offset, QB_.Limit, DefConfigs);
		return Object(RESTAPI::Protocol::CONFIGURATIONS, DefConfigs);
	}
} // namespace OpenWifi
