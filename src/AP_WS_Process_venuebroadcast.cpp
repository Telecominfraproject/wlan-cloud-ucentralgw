//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "AP_WS_Server.h"
#include "StorageService.h"
#include "FindCountry.h"
#include "framework/WebSocketClientNotifications.h"
#include "Daemon.h"
#include "VenueBroadcaster.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_venuebroadcast(Poco::JSON::Object::Ptr ParamsObj) {
		if(ParamsObj->has("data") && ParamsObj->has("serial") && ParamsObj->has("timestamp")) {
			VenueBroadcaster()->Broadcast(
				ParamsObj->get("serial").toString(),
				ParamsObj->get("data").toString(),
				ParamsObj->get("timestamp"));
		}
	}
}