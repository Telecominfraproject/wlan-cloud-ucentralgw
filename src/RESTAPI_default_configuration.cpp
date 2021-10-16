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
#include "RESTAPI_errors.h"

namespace OpenWifi {
	void RESTAPI_default_configuration::DoGet() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");
		GWObjects::DefaultConfiguration DefConfig;
		if (Storage()->GetDefaultConfiguration(Name, DefConfig)) {
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

		if (Storage()->DeleteDefaultConfiguration(Name)) {
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

		if (Storage()->CreateDefaultConfiguration(Name, DefConfig)) {
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

		if (Storage()->UpdateDefaultConfiguration(Name, DefConfig)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::RecordNotUpdated);
	}
}