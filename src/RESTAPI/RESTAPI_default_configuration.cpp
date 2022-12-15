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
#include "framework/ow_constants.h"
#include "framework/ConfigurationValidator.h"
#include "framework/orm.h"
#include "framework/utils.h"

namespace OpenWifi {
	void RESTAPI_default_configuration::DoGet() {
		std::string Name = ORM::Escape(GetBinding(RESTAPI::Protocol::NAME, ""));
		GWObjects::DefaultConfiguration DefConfig;
		if (StorageService()->GetDefaultConfiguration(Name, DefConfig)) {
			return Object(DefConfig);
		}
		NotFound();
	}

	void RESTAPI_default_configuration::DoDelete() {
		std::string Name = ORM::Escape(GetBinding(RESTAPI::Protocol::NAME, ""));
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

		if(StorageService()->DefaultConfigurationAlreadyExists(Name)) {
			return BadRequest(RESTAPI::Errors::DefConfigNameExists);
		}

		const auto &Obj = ParsedBody_;
		GWObjects::DefaultConfiguration DefConfig;
		if (!DefConfig.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if(DefConfig.Models.empty()) {
			return BadRequest(RESTAPI::Errors::ModelIDListCannotBeEmpty);
		}

		std::string Error;
		if (!ValidateUCentralConfiguration(DefConfig.Configuration, Error)) {
			return BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
		}

		DefConfig.Created = DefConfig.LastModified = Utils::Now();
		if (StorageService()->CreateDefaultConfiguration(Name, DefConfig)) {
			return OK();
		}

		BadRequest(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_default_configuration::DoPut() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");

		const auto &Obj = ParsedBody_;
		GWObjects::DefaultConfiguration 		NewConfig;
		if (!NewConfig.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		GWObjects::DefaultConfiguration 		Existing;
		if(!StorageService()->GetDefaultConfiguration(Name,Existing)) {
			return NotFound();
		}

		if (!NewConfig.Configuration.empty()) {
			std::string Error;
			if(!ValidateUCentralConfiguration(NewConfig.Configuration, Error)) {
				return BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
			}
			Existing.Configuration = NewConfig.Configuration;
		}

		Existing.LastModified = Utils::Now();
		AssignIfPresent(Obj,"description",Existing.Description);
		if(Obj->has("modelIds"))
			Existing.Models = NewConfig.Models;

		if (StorageService()->UpdateDefaultConfiguration(Name, Existing)) {
			GWObjects::DefaultConfiguration	ModifiedConfig;

			StorageService()->GetDefaultConfiguration(Name,ModifiedConfig);
			return Object(ModifiedConfig);
		}

		BadRequest(RESTAPI::Errors::RecordNotUpdated);
	}
}