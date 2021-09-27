//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"

#include "RESTAPI_devices_handler.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"
#include "Utils.h"

#include "Utils.h"

namespace OpenWifi {
	void RESTAPI_devices_handler::DoGet() {

		auto serialOnly = GetBoolParameter(RESTAPI::Protocol::SERIALONLY, false);
		auto deviceWithStatus = GetBoolParameter(RESTAPI::Protocol::DEVICEWITHSTATUS, false);

		Poco::JSON::Object RetObj;
		if (!QB_.Select.empty()) {
			Poco::JSON::Array Objects;
			std::vector<std::string> Numbers = Utils::Split(QB_.Select);
			for (auto &i : Numbers) {
				GWObjects::Device D;
				if (Storage()->GetDevice(i, D)) {
					Poco::JSON::Object Obj;
					if (deviceWithStatus)
						D.to_json_with_status(Obj);
					else
						D.to_json(Obj);
					Objects.add(Obj);
				} else {
					Logger_.error(
						Poco::format("DEVICE(%s): device in select cannot be found.", i));
				}
			}
			if (deviceWithStatus)
				RetObj.set(RESTAPI::Protocol::DEVICESWITHSTATUS, Objects);
			else
				RetObj.set(RESTAPI::Protocol::DEVICES, Objects);

		} else if (QB_.CountOnly == true) {
			uint64_t Count = 0;
			if (Storage()->GetDeviceCount(Count)) {
				ReturnCountOnly(Count);
				return;
			}
		} else if (serialOnly) {
			std::vector<std::string> SerialNumbers;
			Storage()->GetDeviceSerialNumbers(QB_.Offset, QB_.Limit, SerialNumbers);
			Poco::JSON::Array Objects;
			for (const auto &i : SerialNumbers) {
				Objects.add(i);
			}
			RetObj.set(RESTAPI::Protocol::SERIALNUMBERS, Objects);
		} else {
			std::vector<GWObjects::Device> Devices;
			Storage()->GetDevices(QB_.Offset, QB_.Limit, Devices);
			Poco::JSON::Array Objects;
			for (const auto &i : Devices) {
				Poco::JSON::Object Obj;
				if (deviceWithStatus)
					i.to_json_with_status(Obj);
				else
					i.to_json(Obj);
				Objects.add(Obj);
			}
			if (deviceWithStatus)
				RetObj.set(RESTAPI::Protocol::DEVICESWITHSTATUS, Objects);
			else
				RetObj.set(RESTAPI::Protocol::DEVICES, Objects);
		}
		ReturnObject(RetObj);
	}
}