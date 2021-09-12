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

		auto Obj = ParseStream();
		GWObjects::Device Device;
		if (!Device.from_json(Obj)) {
			BadRequest("Ill-formed JSON doc.");
			return;
		}

		for(auto &i:Device.Notes)
			i.createdBy = UserInfo_.userinfo.email;

		if (!Utils::ValidSerialNumber(Device.SerialNumber)) {
			Logger_.warning(Poco::format("CREATE-DEVICE(%s): Illegal name.", Device.SerialNumber));
			BadRequest("Invalid serial number.");
			return;
		}

		Device.UUID = time(nullptr);
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
		GWObjects::Device Device;
		if (!Device.from_json(Obj)) {
			BadRequest("Ill-formed JSON document.");
			return;
		}

		for(auto &i:Device.Notes)
			i.createdBy = UserInfo_.userinfo.email;

		if (Storage()->UpdateDevice(Device)) {
			Poco::JSON::Object DevObj;
			Device.to_json(DevObj);
			ReturnObject(DevObj);
		} else {
			BadRequest("Could not update device.");
		}
	}
}