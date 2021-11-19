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
#include "RESTAPI_blacklist.h"
#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "framework/RESTAPI_protocol.h"

namespace OpenWifi {
	void RESTAPI_blacklist::DoDelete() {
		auto SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		GWObjects::BlackListedDevice	D;
		if(!StorageService()->GetBlackListDevice(SerialNumber, D)) {
			return NotFound();
		}

		if (StorageService()->DeleteBlackListDevice(SerialNumber)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::CouldNotBeDeleted);
	}

	void RESTAPI_blacklist::DoGet() {
		auto SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		GWObjects::BlackListedDevice	D;
		if(!StorageService()->GetBlackListDevice(SerialNumber, D)) {
			return NotFound();
		}

		Poco::JSON::Object	Answer;
		D.to_json(Answer);
		return ReturnObject(Answer);
	}

	void RESTAPI_blacklist::DoPost() {
		auto Obj = ParseStream();

		GWObjects::BlackListedDevice	D;
		if(!D.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if(D.serialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		Poco::toLowerInPlace(D.serialNumber);
		if(StorageService()->IsBlackListed(D.serialNumber)) {
			return BadRequest(RESTAPI::Errors::SerialNumberExists);
		}

		D.author = UserInfo_.userinfo.email;
		D.created = std::time(nullptr);

		if(StorageService()->AddBlackListDevice(D)) {
			GWObjects::BlackListedDevice	CreatedDevice;

			StorageService()->GetBlackListDevice(D.serialNumber,CreatedDevice);
			Poco::JSON::Object	Answer;

			CreatedDevice.to_json(Answer);
			return ReturnObject(Answer);
		}
		return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_blacklist::DoPut() {
		auto SerialNumber = Poco::toLower(GetBinding(RESTAPI::Protocol::SERIALNUMBER, ""));
		if(SerialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		auto Obj = ParseStream();

		GWObjects::BlackListedDevice	Existing;
		if(!StorageService()->GetBlackListDevice(SerialNumber, Existing)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		GWObjects::BlackListedDevice	NewDevice;
		if(!NewDevice.from_json(Obj)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		Existing.reason = NewDevice.reason;
		Existing.author = UserInfo_.userinfo.email;

		if(StorageService()->UpdateBlackListDevice(SerialNumber, Existing)) {
			GWObjects::BlackListedDevice	CreatedDevice;

			StorageService()->GetBlackListDevice(SerialNumber,CreatedDevice);
			Poco::JSON::Object	Answer;

			CreatedDevice.to_json(Answer);
			return ReturnObject(Answer);
		}
		return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

}