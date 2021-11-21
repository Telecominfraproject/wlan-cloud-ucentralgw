//
// Created by stephane bourque on 2021-07-21.
//

#pragma once

#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/OpenWifiTypes.h"

namespace OpenWifi {
	class DeviceDashboard {
	  public:
			DeviceDashboard() { DB_.reset(); }
			void Create();
			[[nodiscard]] const GWObjects::Dashboard & Report() const { return DB_;}
	  private:
			GWObjects::Dashboard 	DB_;
			uint64_t 				LastRun_=0;
			inline void Reset() { DB_.reset(); }
	};
}

