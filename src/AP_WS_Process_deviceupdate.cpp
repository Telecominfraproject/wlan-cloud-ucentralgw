//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "AP_WS_Server.h"
#include "StorageService.h"
#include "FindCountry.h"
#include "framework/WebSocketClientNotifications.h"
#include "Daemon.h"

namespace OpenWifi {

	void AP_WS_Connection::Process_deviceupdate(Poco::JSON::Object::Ptr ParamsObj, std::string &Serial) {
		if (!Connected_) {
			poco_warning(Logger(), fmt::format(
									   "INVALID-PROTOCOL({}): Device '{}' is not following protocol", CId_, CN_));
			Errors_++;
			return;
		}
		if (ParamsObj->has("currentPassword")) {
			auto Password = ParamsObj->get("currentPassword").toString();

			StorageService()->SetDevicePassword(Serial, Password);
			poco_trace(Logger(), fmt::format("DEVICEUPDATE({}): Device is updating its login password.", Serial));
		}
	}

}