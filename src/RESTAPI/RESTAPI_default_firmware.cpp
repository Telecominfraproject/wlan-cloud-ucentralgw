//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
// Created by stephane bourque on 2023-07-11.
//	Arilia Wireless Inc.
//

#include "RESTAPI/RESTAPI_default_firmware.h"

#include "RESTObjects/RESTAPI_GWobjects.h"
#include "StorageService.h"
#include "framework/orm.h"
#include "framework/ow_constants.h"
#include "framework/utils.h"

namespace OpenWifi {
	void RESTAPI_default_firmware::DoGet() {
		std::string deviceType = ORM::Escape(GetBinding(RESTAPI::Protocol::DEVICETYPE, ""));
		GWObjects::DefaultFirmware Firmware;
		if (StorageService()->GetDefaultFirmware(deviceType, Firmware)) {
			return Object(Firmware);
		}
		NotFound();
	}

	void RESTAPI_default_firmware::DoDelete() {
		std::string deviceType = ORM::Escape(GetBinding(RESTAPI::Protocol::DEVICETYPE, ""));
		if (deviceType.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (StorageService()->DeleteDefaultFirmware(deviceType)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::CouldNotBeDeleted);
	}

	void RESTAPI_default_firmware::DoPost() {
		std::string deviceType = GetBinding(RESTAPI::Protocol::DEVICETYPE, "");

		if (deviceType.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (StorageService()->DefaultFirmwareAlreadyExists(deviceType)) {
			return BadRequest(RESTAPI::Errors::DefFirmwareNameExists);
		}

		const auto &Obj = ParsedBody_;
		GWObjects::DefaultFirmware Firmware;
		if (!Firmware.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if(Firmware.uri.empty() || Firmware.revision.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		try {
			Poco::URI FirmwareURI(Firmware.uri);
		} catch (...) {
			return BadRequest(RESTAPI::Errors::InvalidURI);
		}

		Firmware.Created = Firmware.LastModified = Utils::Now();
		if (StorageService()->CreateDefaultFirmware(Firmware)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_default_firmware::DoPut() {
		std::string deviceType = GetBinding(RESTAPI::Protocol::DEVICETYPE, "");

		const auto &Obj = ParsedBody_;
		GWObjects::DefaultFirmware NewFirmware;
		if (!NewFirmware.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		GWObjects::DefaultFirmware Existing;
		if (!StorageService()->GetDefaultFirmware(deviceType, Existing)) {
			return NotFound();
		}

		Existing.LastModified = Utils::Now();
		AssignIfPresent(Obj, "description", Existing.Description);
		AssignIfPresent(Obj, "imageCreationDate", Existing.imageCreationDate);
		AssignIfPresent(Obj, "revision", Existing.revision);


		if (Obj->has("uri")) {
			if(NewFirmware.uri.empty()) {
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}
			try {
				Poco::URI FirmwareURI(NewFirmware.uri);
			} catch (...) {
				return BadRequest(RESTAPI::Errors::InvalidURI);
			}
			Existing.uri = NewFirmware.uri;
		}


		if (StorageService()->UpdateDefaultFirmware(Existing)) {
			GWObjects::DefaultFirmware ModifiedFirmware;

			StorageService()->GetDefaultFirmware(deviceType, ModifiedFirmware);
			return Object(ModifiedFirmware);
		}

		BadRequest(RESTAPI::Errors::RecordNotUpdated);
	}
} // namespace OpenWifi