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
#include "framework/ConfigurationValidator.h"
#include "framework/orm.h"
#include "framework/ow_constants.h"
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
		if (Name.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (StorageService()->DeleteDefaultConfiguration(Name)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::CouldNotBeDeleted);
	}

	void RESTAPI_default_configuration::DoPost() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");

		if (Name.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (StorageService()->DefaultConfigurationAlreadyExists(Name)) {
			return BadRequest(RESTAPI::Errors::DefConfigNameExists);
		}

		const auto &Obj = ParsedBody_;
		GWObjects::DefaultConfiguration DefConfig;
		if (!DefConfig.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if (DefConfig.models.empty()) {
			return BadRequest(RESTAPI::Errors::ModelIDListCannotBeEmpty);
		}

		DefConfig.platform = DefConfig.platform.empty() ? Platforms::AP : DefConfig.platform;
		if(DefConfig.platform != Platforms::AP && DefConfig.platform != Platforms::SWITCH) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		std::string Error;
		if (!ValidateUCentralConfiguration(ConfigurationValidator::GetType(DefConfig.platform),
										   DefConfig.configuration, Error,
										   GetBoolParameter("strict", false))) {
			return BadRequest(RESTAPI::Errors::ConfigBlockInvalid, Error);
		}

		DefConfig.created = DefConfig.lastModified = Utils::Now();
		if (StorageService()->CreateDefaultConfiguration(Name, DefConfig)) {
			return OK();
		}

		BadRequest(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_default_configuration::DoPut() {
		std::string Name = GetBinding(RESTAPI::Protocol::NAME, "");

		const auto &Obj = ParsedBody_;
		GWObjects::DefaultConfiguration NewConfig;
		if (!NewConfig.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		GWObjects::DefaultConfiguration Existing;
		if (!StorageService()->GetDefaultConfiguration(Name, Existing)) {
			return NotFound();
		}

		if(Existing.platform.empty()) {
			Existing.platform = Platforms::AP;
		}

		if(ParsedBody_->has("platform")) {
			if(NewConfig.platform.empty() || (NewConfig.platform != Platforms::AP && NewConfig.platform != Platforms::SWITCH)) {
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}
			Existing.platform = NewConfig.platform;
		}

		if (!NewConfig.configuration.empty()) {
			std::string Error;
			if (!ValidateUCentralConfiguration(ConfigurationValidator::GetType(Existing.platform),
											   NewConfig.configuration, Error,
											   GetBoolParameter("strict", false))) {
				return BadRequest(RESTAPI::Errors::ConfigBlockInvalid, Error);
			}
			Existing.configuration = NewConfig.configuration;
		}

		Existing.lastModified = Utils::Now();
		AssignIfPresent(Obj, "description", Existing.description);
		if (Obj->has("modelIds"))
			Existing.models = NewConfig.models;

		if (StorageService()->UpdateDefaultConfiguration(Name, Existing)) {
			GWObjects::DefaultConfiguration ModifiedConfig;

			StorageService()->GetDefaultConfiguration(Name, ModifiedConfig);
			return Object(ModifiedConfig);
		}

		BadRequest(RESTAPI::Errors::RecordNotUpdated);
	}
} // namespace OpenWifi