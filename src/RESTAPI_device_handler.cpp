//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_device_handler.h"
#include "Poco/JSON/Parser.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"
#include "Utils.h"
#include "ConfigurationValidator.h"
#include "ConfigurationCache.h"
#include "CentralConfig.h"

namespace OpenWifi {
	void RESTAPI_device_handler::DoGet() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");
		GWObjects::Device Device;

		if (Storage()->GetDevice(SerialNumber, Device)) {
			Poco::JSON::Object Obj;
			Device.to_json(Obj);
			ReturnObject(Obj);
		} else {
			NotFound();
		}
	}

	void RESTAPI_device_handler::DoDelete() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");
		if (Storage()->DeleteDevice(SerialNumber)) {
			OK();
		} else {
			NotFound();
		}
	}

	void RESTAPI_device_handler::DoPost() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");
		if(SerialNumber.empty()) {
			BadRequest("Missing Serial number.");
			return;
		}

		if (!Utils::ValidSerialNumber(SerialNumber)) {
			Logger_.warning(Poco::format("CREATE-DEVICE(%s): Illegal serial number.", SerialNumber));
			BadRequest("Invalid serial number.");
			return;
		}

		auto Obj = ParseStream();
		GWObjects::Device Device;
		if (!Device.from_json(Obj)) {
			BadRequest("Ill-formed JSON doc.");
			return;
		}

		if(SerialNumber!=Device.SerialNumber) {
			BadRequest("Serial Number mismatch.");
			return;
		}

		if(Device.Configuration.empty() || (!Device.Configuration.empty() && !ValidateUCentralConfiguration(Device.Configuration))) {
			BadRequest("Illegal configuration.");
			return;
		}

		for(auto &i:Device.Notes)
			i.createdBy = UserInfo_.userinfo.email;

		Config::Config NewConfig(Device.Configuration);
		Device.UUID = std::time(nullptr);
		NewConfig.SetUUID(Device.UUID);
		Device.Configuration = NewConfig.get();

		Poco::toLowerInPlace(Device.SerialNumber);

		if (Storage()->CreateDevice(Device)) {
			Poco::JSON::Object DevObj;
			Device.to_json(DevObj);
			ReturnObject(DevObj);
		} else {
			BadRequest("Internal error.");
		}
	}

	void RESTAPI_device_handler::DoPut() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");
		if(SerialNumber.empty()) {
			BadRequest("Missing Serial number.");
			return;
		}

		auto Obj = ParseStream();
		GWObjects::Device NewDevice;
		if (!NewDevice.from_json(Obj)) {
			BadRequest("Ill-formed JSON document.");
			return;
		}

		GWObjects::Device	Existing;
		if(!Storage()->GetDevice(SerialNumber, Existing)) {
			NotFound();
			return;
		}

		uint64_t NewConfigUUID=0;
		if(!NewDevice.Configuration.empty()) {
			if (!ValidateUCentralConfiguration(NewDevice.Configuration)) {
				BadRequest("Illegal configuration.");
				return;
			}
			Config::Config NewConfig(NewDevice.Configuration);
			NewConfigUUID = std::time(nullptr);
			NewConfig.SetUUID(NewConfigUUID);
			Existing.Configuration = NewConfig.get();
			Existing.UUID = NewConfigUUID;
		}

		AssignIfPresent(Obj, "venue", Existing.Venue);
		AssignIfPresent(Obj, "owner", Existing.Owner);
		AssignIfPresent(Obj, "location", Existing.Location);

		for(auto &i:NewDevice.Notes) {
			i.createdBy = UserInfo_.userinfo.email;
			Existing.Notes.push_back(i);
		}

		Existing.LastConfigurationChange = std::time(nullptr);
		if (Storage()->UpdateDevice(Existing)) {
			if(NewConfigUUID)
				SetCurrentConfigurationID(SerialNumber, NewConfigUUID);
			Poco::JSON::Object DevObj;
			NewDevice.to_json(DevObj);
			ReturnObject(DevObj);
		} else {
			BadRequest("Could not update device.");
		}
	}
}