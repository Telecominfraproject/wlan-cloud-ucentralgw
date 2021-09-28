//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <ctime>

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "RESTAPI_BlackList.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"
#include "RESTAPI_errors.h"

namespace OpenWifi {

	void RESTAPI_BlackList::DoDelete() {
		auto SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			BadRequest(RESTAPI::Errors::MissingSerialNumber);
			return;
		}

		GWObjects::BlackListedDevice	D;
		if(!Storage()->GetBlackListDevice(SerialNumber, D)) {
			NotFound();
			return;
		}

		if (Storage()->DeleteBlackListDevice(SerialNumber)) {
			OK();
			return;
		}
		BadRequest(RESTAPI::Errors::CouldNotBeDeleted);
	}

	void RESTAPI_BlackList::DoGet() {
		std::vector<GWObjects::BlackListedDevice> Devices;
		Poco::JSON::Array Objects;
		if (Storage()->GetBlackListDevices(QB_.Offset, QB_.Limit, Devices)) {
			for (const auto &i : Devices) {
				Poco::JSON::Object Obj;
				i.to_json(Obj);
				Objects.add(Obj);
			}
		}
		Poco::JSON::Object RetObj;
		RetObj.set(RESTAPI::Protocol::DEVICES, Objects);
		ReturnObject(RetObj);
	}

	void RESTAPI_BlackList::DoPost() {
		auto Obj = ParseStream();
		if (Obj->has(RESTAPI::Protocol::DEVICES) &&
			Obj->isArray(RESTAPI::Protocol::DEVICES)) {
			std::vector<GWObjects::BlackListedDevice> Devices;
			auto DeviceArray = Obj->getArray(RESTAPI::Protocol::DEVICES);
			for (const auto &i : *DeviceArray) {
				Poco::JSON::Parser pp;
				auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
				Poco::DynamicStruct Vars = *InnerObj;
				if (Vars.contains(RESTAPI::Protocol::SERIALNUMBER) &&
					Vars.contains(RESTAPI::Protocol::REASON)) {
					auto SerialNumber = Vars[RESTAPI::Protocol::SERIALNUMBER].toString();
					auto Reason = Vars[RESTAPI::Protocol::REASON].toString();
					GWObjects::BlackListedDevice D{.SerialNumber = SerialNumber,
														   .Reason = Reason,
														   .Author = UserInfo_.webtoken.username_,
														   .Created = (uint64_t)time(nullptr)};
					Devices.push_back(D);
				}
			}
			if (!Devices.empty()) {
				if (Storage()->AddBlackListDevices(Devices)) {
					OK();
					return;
				}
			} else {
				BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}
		} else {
			BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}
	}
}