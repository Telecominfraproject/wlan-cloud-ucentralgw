//
// Created by stephane bourque on 2021-07-21.
//

#ifndef UCENTRALGW_DASHBOARD_H
#define UCENTRALGW_DASHBOARD_H

#include "OpenWifiTypes.h"
#include "RESTAPI_GWobjects.h"

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

#endif // UCENTRALGW_DASHBOARD_H
