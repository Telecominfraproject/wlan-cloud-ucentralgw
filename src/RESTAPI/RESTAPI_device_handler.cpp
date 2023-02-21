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
#include "StorageService.h"

#include "framework/ConfigurationValidator.h"
#include "framework/ow_constants.h"
#include "framework/utils.h"

#include "RESTAPI_device_helper.h"

namespace OpenWifi {
	void RESTAPI_device_handler::DoGet() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if (!Utils::NormalizeMac(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		GWObjects::Device Device;
		if (StorageService()->GetDevice(SerialNumber, Device)) {
			Poco::JSON::Object Answer;
			if (GetBoolParameter("completeInfo", false)) {
				CompleteDeviceInfo(Device, Answer);
				return ReturnObject(Answer);
			} else {
				return Object(Device);
			}
		}
		NotFound();
	}

	void RESTAPI_device_handler::DoDelete() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if (!Utils::NormalizeMac(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		std::string Arg;
		if (HasParameter("oui", Arg) && Arg == "true" && SerialNumber.size() == 6) {

			std::set<std::string> Set;
			std::vector<GWObjects::Device> Devices;

			bool Done = false;
			uint64_t Offset = 1;
			while (!Done) {

				StorageService()->GetDevices(Offset, 500, Devices);
				for (const auto &i : Devices) {
					if (i.SerialNumber.substr(0, 6) == SerialNumber) {
						Set.insert(i.SerialNumber);
					}
				}

				if (Devices.size() < 500)
					Done = true;

				Offset += Devices.size();
			}

			for (auto &i : Set) {
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
		if (!Utils::NormalizeMac(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		const auto &Obj = ParsedBody_;
		std::string Arg;
		if (HasParameter("validateOnly", Arg) && Arg == "true") {
			if (!Obj->has("configuration")) {
				return BadRequest(RESTAPI::Errors::MustHaveConfigElement);
			}
			auto Config = Obj->get("configuration").toString();
			Poco::JSON::Object Answer;
			std::vector<std::string> Error;
			auto Res =
				ValidateUCentralConfiguration(Config, Error, GetBoolParameter("strict", false));
			Answer.set("valid", Res);
			if (!Error.empty())
				Answer.set("error", Error);
			return ReturnObject(Answer);
		}

		GWObjects::Device Device;
		if (!Device.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if (!Utils::NormalizeMac(Device.SerialNumber)) {
			return BadRequest(RESTAPI::Errors::InvalidSerialNumber);
		}

		if (SerialNumber != Device.SerialNumber) {
			return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
		}

		std::vector<std::string> Error;
		if (Device.Configuration.empty() ||
			(!Device.Configuration.empty() &&
			 !ValidateUCentralConfiguration(Device.Configuration, Error,
											GetBoolParameter("strict", false)))) {
			return BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
		}

		for (auto &i : Device.Notes) {
			i.createdBy = UserInfo_.userinfo.email;
			i.created = Utils::Now();
		}

		Config::Config NewConfig(Device.Configuration);
		Device.UUID = Utils::Now();
		NewConfig.SetUUID(Device.UUID);
		Device.Configuration = NewConfig.get();

		Poco::toLowerInPlace(Device.SerialNumber);

		if (StorageService()->CreateDevice(Device)) {
			SetCurrentConfigurationID(SerialNumber, Device.UUID);
			return Object(Device);
		}
		InternalError(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_device_handler::DoPut() {
		std::string SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if (!Utils::ValidSerialNumber(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		const auto &Obj = ParsedBody_;
		GWObjects::Device NewDevice;
		if (!NewDevice.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		GWObjects::Device Existing;
		if (!StorageService()->GetDevice(SerialNumber, Existing)) {
			return NotFound();
		}

		if (!NewDevice.Configuration.empty()) {
			std::vector<std::string> Error;
			if (!ValidateUCentralConfiguration(NewDevice.Configuration, Error,
											   GetBoolParameter("strict", false))) {
				return BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
			}
			Config::Config NewConfig(NewDevice.Configuration);
			uint64_t NewConfigUUID = Utils::Now();
			NewConfig.SetUUID(NewConfigUUID);
			Existing.Configuration = NewConfig.get();
			Existing.UUID = NewConfigUUID;
		}

		AssignIfPresent(Obj, "venue", Existing.Venue);
		AssignIfPresent(Obj, "owner", Existing.Owner);
		AssignIfPresent(Obj, "location", Existing.Location);
		AssignIfPresent(Obj, "subscriber", Existing.subscriber);
		AssignIfPresent(Obj, "entity", Existing.entity);

		for (auto &i : NewDevice.Notes) {
			i.createdBy = UserInfo_.userinfo.email;
			i.created = Utils::Now();
			Existing.Notes.push_back(i);
		}

		Existing.LastConfigurationChange = Utils::Now();
		if (StorageService()->UpdateDevice(Existing)) {
			SetCurrentConfigurationID(SerialNumber, Existing.UUID);
			GWObjects::Device UpdatedDevice;
			StorageService()->GetDevice(SerialNumber, UpdatedDevice);
			return Object(UpdatedDevice);
		}
		InternalError(RESTAPI::Errors::RecordNotUpdated);
	}
} // namespace OpenWifi