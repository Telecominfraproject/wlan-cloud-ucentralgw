//
// Created by stephane bourque on 2021-12-05.
//

#pragma once

#include "RESTObjects/RESTAPI_GWobjects.h"
#include "StorageService.h"
#include "DeviceRegistry.h"

namespace OpenWifi {

	inline void CompleteDeviceInfo(const GWObjects::Device & Device, Poco::JSON::Object & Answer) {
		GWObjects::ConnectionState	CS;
		DeviceRegistry()->GetState(Device.SerialNumber,CS);
		GWObjects::HealthCheck		HC;
		DeviceRegistry()->GetHealthcheck(Device.SerialNumber, HC);
		std::string 	Stats;
		DeviceRegistry()->GetStatistics(Device.SerialNumber, Stats);

		Poco::JSON::Object	DeviceInfo;
		Device.to_json(DeviceInfo);
		Answer.set("deviceInfo", DeviceInfo);
		Poco::JSON::Object	CSInfo;
		CS.to_json(CSInfo);
		Answer.set("connectionInfo",CSInfo);
		Poco::JSON::Object	HCInfo;
		HC.to_json(HCInfo);
		Answer.set("healthCheckInfo",HCInfo);
		Poco::JSON::Parser	P;
		auto StatsInfo = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
		Answer.set("statsInfo",StatsInfo);
	}

}