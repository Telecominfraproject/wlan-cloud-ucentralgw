//
// Created by stephane bourque on 2021-07-21.
//

#include "RESTAPI_deviceDashboardHandler.h"
#include "Daemon.h"
#include "Dashboard.h"

namespace OpenWifi {
	void RESTAPI_deviceDashboardHandler::DoGet() {
		Daemon()->GetDashboard().Create();
		Poco::JSON::Object	Answer;
		Daemon()->GetDashboard().Report().to_json(Answer);
		ReturnObject(Answer);
	}
}