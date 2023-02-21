//
// Created by stephane bourque on 2021-12-05.
//

#pragma once

#include "AP_WS_Server.h"
#include "RESTObjects/RESTAPI_GWobjects.h"
#include "StorageService.h"

namespace OpenWifi {

	inline void CompleteDeviceInfo(const GWObjects::Device &Device, Poco::JSON::Object &Answer) {
		GWObjects::ConnectionState CS;
		AP_WS_Server()->GetState(Device.SerialNumber, CS);
		GWObjects::HealthCheck HC;
		AP_WS_Server()->GetHealthcheck(Device.SerialNumber, HC);
		std::string Stats;
		AP_WS_Server()->GetStatistics(Device.SerialNumber, Stats);

		Poco::JSON::Object DeviceInfo;
		Device.to_json(DeviceInfo);
		Answer.set("deviceInfo", DeviceInfo);
		Poco::JSON::Object CSInfo;
		CS.to_json(CSInfo);
		Answer.set("connectionInfo", CSInfo);
		Poco::JSON::Object HCInfo;
		HC.to_json(HCInfo);
		Answer.set("healthCheckInfo", HCInfo);
		try {
			Poco::JSON::Parser P;
			auto StatsInfo = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
			Answer.set("statsInfo", StatsInfo);
		} catch (...) {
			Poco::JSON::Object Empty;
			Answer.set("statsInfo", Empty);
		}
	}

} // namespace OpenWifi