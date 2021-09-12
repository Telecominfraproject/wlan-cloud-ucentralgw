//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"

#include "RESTAPI_default_configuration.h"

#include "RESTAPI_GWobjects.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_default_configuration::DoGet() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");
		GWObjects::DefaultConfiguration DefConfig;
		if (Storage()->GetDefaultConfiguration(Name, DefConfig)) {
			Poco::JSON::Object Obj;
			DefConfig.to_json(Obj);
			ReturnObject(Obj);
		} else {
			NotFound();
		}
	}

	void RESTAPI_default_configuration::DoDelete() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");
		if (Storage()->DeleteDefaultConfiguration(Name)) {
			OK();
		} else {
			NotFound();
		}
	}

	void RESTAPI_default_configuration::DoPost() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");
		auto Obj = ParseStream();
		GWObjects::DefaultConfiguration DefConfig;
			if (!DefConfig.from_json(Obj)) {
			BadRequest("Ill-formed JSON document");
			return;
		}

		if (Storage()->CreateDefaultConfiguration(Name, DefConfig)) {
			OK();
		} else {
			BadRequest("Could not create default configuration.");
		}
	}

	void RESTAPI_default_configuration::DoPut() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");

		auto  Obj = ParseStream();
			GWObjects::DefaultConfiguration DefConfig;
		if (!DefConfig.from_json(Obj)) {
			BadRequest("Ill-formed JSON document.");
			return;
		}

		if (Storage()->UpdateDefaultConfiguration(Name, DefConfig)) {
			OK();
		} else {
			BadRequest("Could not update configuration.");
		}
	}
}