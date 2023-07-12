//
// Created by stephane bourque on 2023-07-12.
//

#pragma once


#include "RESTObjects/RESTAPI_FMSObjects.h"
#include "framework/MicroServiceNames.h"
#include "framework/OpenAPIRequests.h"
#include "framework/RESTAPI_utils.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Logger.h"

#include "fmt/format.h"

namespace OpenWifi::SDK::FMS {
	inline bool GetFirmwareAge( const std::string &deviceType, const std::string &revision, FMSObjects::FirmwareAgeDetails Age, Poco::Logger &Logger) {
		OpenAPIRequestGet GetFirmwareAgeAPI(
			uSERVICE_FIRMWARE, "/api/v1/firmwareAge" ,
			{
				{ RESTAPI::Protocol::DEVICETYPE, deviceType },
				{ RESTAPI::Protocol::REVISION, revision }
			}, 30000);

		auto CallResponse = Poco::makeShared<Poco::JSON::Object>();
		if (!GetFirmwareAgeAPI.Do(CallResponse, "")) {
			Logger.error(fmt::format("{}: Cannot find revision.", revision));
			return false;
		}

		if(Age.from_json(CallResponse)) {
			return true;
		}
		return false;
	}

} // namespace OpenWifi::SDK::Prov