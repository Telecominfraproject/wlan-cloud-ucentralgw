//
// Created by stephane bourque on 2021-07-21.
//

#pragma once

#include <mutex>

#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/OpenWifiTypes.h"
#include "Poco/Logger.h"

namespace OpenWifi {
	class DeviceDashboard {
	  public:
			bool Get(GWObjects::Dashboard &D, Poco::Logger & Logger);
	  private:
			std::mutex					DataMutex_;
			volatile std::atomic_bool 	GeneratingDashboard_=false;
			volatile bool 				ValidDashboard_=false;
			GWObjects::Dashboard 		DB_;
			uint64_t 					LastRun_=0;

			void Generate(GWObjects::Dashboard &D, Poco::Logger & Logger);
	};
}

