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
#include "RESTAPI_protocol.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_default_configurations::DoGet() {
		std::vector<GWObjects::DefaultConfiguration> DefConfigs;
		Storage()->GetDefaultConfigurations(QB_.Offset, QB_.Limit, DefConfigs);

		Poco::JSON::Array Objects;
		for (const auto &i : DefConfigs) {
			Poco::JSON::Object Obj;
			i.to_json(Obj);
			Objects.add(Obj);
		}

		Poco::JSON::Object RetObj;
		RetObj.set(RESTAPI::Protocol::CONFIGURATIONS, Objects);
		ReturnObject(RetObj);
	}
}
