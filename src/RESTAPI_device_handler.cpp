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
#include "RESTAPI_errors.h"

namespace OpenWifi {
	void RESTAPI_device_handler::DoGet() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			BadRequest(RESTAPI::Errors::MissingSerialNumber);
			return;
		}

		GWObjects::Device Device;

		if (Storage()->GetDevice(SerialNumber, Device)) {
			Poco::JSON::Object Obj;
			Device.to_json(Obj);
			ReturnObject(Obj);
			return;
		}
		NotFound();
	}

	void RESTAPI_device_handler::DoDelete() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			BadRequest(RESTAPI::Errors::MissingSerialNumber);
			return;
		}

		if (Storage()->DeleteDevice(SerialNumber)) {
			OK();
			return;
		}
		NotFound();
	}

	void RESTAPI_device_handler::DoPost() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			BadRequest(RESTAPI::Errors::MissingSerialNumber);
			return;
		}

		if (!Utils::ValidSerialNumber(SerialNumber)) {
			Logger_.warning(Poco::format("CREATE-DEVICE(%s): Illegal serial number.", SerialNumber));
			BadRequest( RESTAPI::Errors::InvalidSerialNumber);
			return;
		}

		auto Obj = ParseStream();
		GWObjects::Device Device;
		if (!Device.from_json(Obj)) {
			BadRequest(RESTAPI::Errors::InvalidJSONDocument);
			return;
		}

		if(SerialNumber!=Device.SerialNumber) {
			BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			return;
		}

		if(Device.Configuration.empty() || (!Device.Configuration.empty() && !ValidateUCentralConfiguration(Device.Configuration))) {
			BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
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
			SetCurrentConfigurationID(SerialNumber, Device.UUID);
			Poco::JSON::Object DevObj;
			Device.to_json(DevObj);
			ReturnObject(DevObj);
			return;
		}
		InternalError(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_device_handler::DoPut() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			BadRequest(RESTAPI::Errors::MissingSerialNumber);
			return;
		}

		auto Obj = ParseStream();
		GWObjects::Device NewDevice;
		if (!NewDevice.from_json(Obj)) {
			BadRequest(RESTAPI::Errors::InvalidJSONDocument);
			return;
		}

		GWObjects::Device	Existing;
		if(!Storage()->GetDevice(SerialNumber, Existing)) {
			NotFound();
			return;
		}

		if(!NewDevice.Configuration.empty()) {
			if (!ValidateUCentralConfiguration(NewDevice.Configuration)) {
				BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
				return;
			}
			Config::Config NewConfig(NewDevice.Configuration);
			uint64_t NewConfigUUID = std::time(nullptr);
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
			SetCurrentConfigurationID(SerialNumber, Existing.UUID);
			Poco::JSON::Object DevObj;
			NewDevice.to_json(DevObj);
			ReturnObject(DevObj);
			return;
		}
		InternalError(RESTAPI::Errors::RecordNotUpdated);
	}
}