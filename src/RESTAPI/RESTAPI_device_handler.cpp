//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_device_handler.h"
#include "CentralConfig.h"
#include "ConfigurationCache.h"
#include "Poco/JSON/Parser.h"
#include "StorageService.h"
#include "framework/ConfigurationValidator.h"
#include "framework/MicroService.h"
#include "framework/ow_constants.h"
#include "RESTAPI_device_helper.h"

namespace OpenWifi {
	void RESTAPI_device_handler::DoGet() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		GWObjects::Device Device;
		if (StorageService()->GetDevice(SerialNumber, Device)) {
			Poco::JSON::Object	Answer;
			if(GetBoolParameter("completeInfo",false)) {
				CompleteDeviceInfo(Device, Answer);
				return ReturnObject(Answer);
			} else {
				Poco::JSON::Object Obj;
				Device.to_json(Obj);
				return ReturnObject(Obj);
			}
		}
		NotFound();
	}

	void RESTAPI_device_handler::DoDelete() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		std::string Arg;
		if(HasParameter("oui",Arg) && Arg=="true" && SerialNumber.size()==6) {

			std::set<std::string>	Set;
			std::vector<GWObjects::Device>	Devices;

			bool Done = false;
			uint64_t Offset=1;
			while(!Done) {

				StorageService()->GetDevices(Offset,500,Devices);
				for(const auto &i:Devices) {
					if(i.SerialNumber.substr(0,6) == SerialNumber) {
						Set.insert(i.SerialNumber);
					}
				}

				if(Devices.size()<500)
					Done=true;

				Offset += Devices.size();
			}

			for(auto &i:Set) {
				std::string SNum{i};
				StorageService()->DeleteDevice(SNum);
			}

			return OK();

		} else if (StorageService()->DeleteDevice(SerialNumber)) {
			return OK();
		}

		NotFound();
	}

	void RESTAPI_device_handler::DoPost() {

		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");
		if(SerialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		std::string Arg;
		if(HasParameter("validateOnly",Arg) && Arg=="true") {
			auto Body = ParseStream();
			if(!Body->has("configuration")) {
				return BadRequest("Must have 'configuration' element.");
			}
			auto Config=Body->get("configuration").toString();
			Poco::JSON::Object  Answer;
			std::string 		Error;
			auto Res = ValidateUCentralConfiguration(Config, Error);
			Answer.set("valid",Res);
			if(!Error.empty())
				Answer.set("error",Error);
			return ReturnObject(Answer);
		}

		if (!Utils::ValidSerialNumber(SerialNumber)) {
			Logger_.warning(fmt::format("CREATE-DEVICE({}): Illegal serial number.", SerialNumber));
			return BadRequest( RESTAPI::Errors::InvalidSerialNumber);
		}

		auto Obj = ParseStream();
		GWObjects::Device Device;
		if (!Device.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if(SerialNumber!=Device.SerialNumber) {
			return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
		}

		std::string Error;
		if(Device.Configuration.empty() || (!Device.Configuration.empty() && !ValidateUCentralConfiguration(Device.Configuration,Error))) {
			return BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
		}

		for(auto &i:Device.Notes) {
			i.createdBy = UserInfo_.userinfo.email;
			i.created = OpenWifi::Now();
		}

		Config::Config NewConfig(Device.Configuration);
		Device.UUID = std::time(nullptr);
		NewConfig.SetUUID(Device.UUID);
		Device.Configuration = NewConfig.get();

		Poco::toLowerInPlace(Device.SerialNumber);

		if (StorageService()->CreateDevice(Device)) {
			SetCurrentConfigurationID(SerialNumber, Device.UUID);
			Poco::JSON::Object DevObj;
			Device.to_json(DevObj);
			return ReturnObject(DevObj);
		}
		InternalError(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_device_handler::DoPut() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		auto Obj = ParseStream();
		GWObjects::Device NewDevice;
		if (!NewDevice.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		GWObjects::Device	Existing;
		if(!StorageService()->GetDevice(SerialNumber, Existing)) {
			return NotFound();
		}

		if(!NewDevice.Configuration.empty()) {
			std::string Error;
			if (!ValidateUCentralConfiguration(NewDevice.Configuration, Error)) {
				return BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
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
		AssignIfPresent(Obj, "subscriber", Existing.subscriber);
		AssignIfPresent(Obj, "entity", Existing.entity);

		for(auto &i:NewDevice.Notes) {
			i.createdBy = UserInfo_.userinfo.email;
			i.created = OpenWifi::Now();
			Existing.Notes.push_back(i);
		}

		Existing.LastConfigurationChange = std::time(nullptr);
		if (StorageService()->UpdateDevice(Existing)) {
			SetCurrentConfigurationID(SerialNumber, Existing.UUID);
			Poco::JSON::Object DevObj;
			NewDevice.to_json(DevObj);
			return ReturnObject(DevObj);
		}
		InternalError(RESTAPI::Errors::RecordNotUpdated);
	}
}