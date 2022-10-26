//
// Created by stephane bourque on 2022-10-25.
//

#include "framework/WebSocketClientNotifications.h"
#include "framework/UI_WebSocketClientServer.h"

namespace OpenWifi {


	void  WebSocketClientNotificationNumberOfConnections(std::uint64_t numberOfDevices,
														   std::uint64_t averageConnectedTime,
														   std::uint64_t numberOfConnectingDevices) {
		WebSocketNotification<WebSocketClientNotificationNumberOfConnection>	N;
		N.content.numberOfDevices = numberOfDevices;
		N.content.averageConnectedTime = averageConnectedTime;
		N.content.numberOfConnectingDevices = numberOfConnectingDevices;
		N.type = "device_connections_statistics";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketClientNotificationDeviceConfigurationChange(const std::string &SerialNumber, uint64_t oldUUID, uint64_t newUUID) {
		WebSocketNotification<WebNotificationSingleDeviceConfigurationChange>	N;
		N.content.serialNumber = SerialNumber;
		N.content.oldUUID = oldUUID;
		N.content.newUUID = newUUID;
		N.type = "device_configuration_upgrade";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketClientNotificationDeviceFirmwareUpdated(const std::string &SerialNumber, const std::string &Firmware) {
		WebSocketNotification<WebNotificationSingleDeviceFirmwareChange>	N;
		N.content.serialNumber = SerialNumber;
		N.content.newFirmware = Firmware;
		N.type = "device_firmware_upgrade";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketClientNotificationDeviceConnected(const std::string &SerialNumber) {
		WebSocketNotification<WebNotificationSingleDevice>	N;
		N.content.serialNumber = SerialNumber;
		N.type = "device_connection";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketClientNotificationDeviceDisconnected(const std::string & SerialNumber) {
		WebSocketNotification<WebNotificationSingleDevice>	N;
		N.content.serialNumber = SerialNumber;
		N.type = "device_disconnection";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketNotificationJobContent::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj,"title",title);
		RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
		RESTAPI_utils::field_to_json(Obj,"success",success);
		RESTAPI_utils::field_to_json(Obj,"error",error);
		RESTAPI_utils::field_to_json(Obj,"warning",warning);
		RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
		RESTAPI_utils::field_to_json(Obj,"details",details);
	}

	bool WebSocketNotificationJobContent::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj,"title",title);
			RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
			RESTAPI_utils::field_from_json(Obj,"success",success);
			RESTAPI_utils::field_from_json(Obj,"error",error);
			RESTAPI_utils::field_from_json(Obj,"warning",warning);
			RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
			RESTAPI_utils::field_from_json(Obj,"details",details);
			return true;
		} catch(...) {

		}
		return false;
	}

	void WebSocketClientNotificationVenueUpdateJobCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpdateJob_t &N) {
		N.type = "venue_configuration_update";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}

	void WebSocketNotificationRebootList::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj,"title",title);
		RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
		RESTAPI_utils::field_to_json(Obj,"success",success);
		RESTAPI_utils::field_to_json(Obj,"warning",warning);
		RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
		RESTAPI_utils::field_to_json(Obj,"details",details);
	}

	bool WebSocketNotificationRebootList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj,"title",title);
			RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
			RESTAPI_utils::field_from_json(Obj,"success",success);
			RESTAPI_utils::field_from_json(Obj,"warning",warning);
			RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
			RESTAPI_utils::field_from_json(Obj,"details",details);
			return true;
		} catch(...) {

		}
		return false;
	}

	void WebSocketClientNotificationVenueRebootCompletionToUser( const std::string & User, WebSocketClientNotificationVenueRebootList_t &N) {
		N.type = "venue_rebooter";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}

	void WebSocketNotificationUpgradeList::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj,"title",title);
		RESTAPI_utils::field_to_json(Obj,"jobId",jobId);
		RESTAPI_utils::field_to_json(Obj,"success",success);
		RESTAPI_utils::field_to_json(Obj,"notConnected",not_connected);
		RESTAPI_utils::field_to_json(Obj,"noFirmware",no_firmware);
		RESTAPI_utils::field_to_json(Obj,"skipped",skipped);
		RESTAPI_utils::field_to_json(Obj,"timeStamp",timeStamp);
		RESTAPI_utils::field_to_json(Obj,"details",details);
	}

	bool WebSocketNotificationUpgradeList::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj,"title",title);
			RESTAPI_utils::field_from_json(Obj,"jobId",jobId);
			RESTAPI_utils::field_from_json(Obj,"success",success);
			RESTAPI_utils::field_from_json(Obj,"notConnected",not_connected);
			RESTAPI_utils::field_from_json(Obj,"noFirmware",no_firmware);
			RESTAPI_utils::field_from_json(Obj,"skipped",skipped);
			RESTAPI_utils::field_from_json(Obj,"timeStamp",timeStamp);
			RESTAPI_utils::field_from_json(Obj,"details",details);
			return true;
		} catch(...) {

		}
		return false;
	}

	void WebSocketClientNotificationVenueUpgradeCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpgradeList_t &N) {
		N.type = "venue_upgrader";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}
}

