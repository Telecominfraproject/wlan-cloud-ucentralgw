//
// Created by stephane bourque on 2021-07-21.
//

#pragma once

#include <mutex>

#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/OpenWifiTypes.h"

namespace OpenWifi {
	class DeviceDashboard {
	  public:
			DeviceDashboard() { DB_.reset(); }
			bool Get(GWObjects::Dashboard &D) {
				uint64_t Now = Utils::Now();
				if(!ValidDashboard_ || LastRun_==0 || (Now-LastRun_)>120) {
					Generate(D);
				} else {
					D = DB_;
				}
				return ValidDashboard_;
			};
			// [[nodiscard]] const GWObjects::Dashboard & Report() const { return DB_;}
	  private:
			std::mutex					DataMutex_;
			volatile std::atomic_bool 	GeneratingDashboard_=false;
			volatile bool 				ValidDashboard_=false;
			GWObjects::Dashboard 		DB_;
			uint64_t 					LastRun_=0;
			inline void Reset() { DB_.reset(); }
			void Generate(GWObjects::Dashboard &D);
	};
}

