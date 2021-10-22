//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"

#include "RESTAPI_default_configuration.h"

#include "RESTObjects/RESTAPI_GWobjects.h"
#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "framework/RESTAPI_protocol.h"

namespace OpenWifi {
	void RESTAPI_default_configuration::DoGet() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");
		GWObjects::DefaultConfiguration DefConfig;
		if (StorageService()->GetDefaultConfiguration(Name, DefConfig)) {
			Poco::JSON::Object Obj;
			DefConfig.to_json(Obj);
			return ReturnObject(Obj);
		}
		NotFound();
	}

	void RESTAPI_default_configuration::DoDelete() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");
		if(Name.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (StorageService()->DeleteDefaultConfiguration(Name)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::CouldNotBeDeleted);
	}

	void RESTAPI_default_configuration::DoPost() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");

		if(Name.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		auto Obj = ParseStream();
		GWObjects::DefaultConfiguration DefConfig;
		if (!DefConfig.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if (StorageService()->CreateDefaultConfiguration(Name, DefConfig)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_default_configuration::DoPut() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");

		auto  Obj = ParseStream();
			GWObjects::DefaultConfiguration DefConfig;

		if (!DefConfig.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if (StorageService()->UpdateDefaultConfiguration(Name, DefConfig)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::RecordNotUpdated);
	}
}