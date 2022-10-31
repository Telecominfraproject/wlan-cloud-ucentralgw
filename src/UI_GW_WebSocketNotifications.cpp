//
// Created by stephane bourque on 2022-10-29.
//

#include "UI_GW_WebSocketNotifications.h"

namespace OpenWifi {
	inline void WebNotificationSingleDevice::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
	}

	inline bool WebNotificationSingleDevice::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
			return true;
		} catch (...) {

		}
		return false;
	}

	inline void WebNotificationSingleDeviceConfigurationChange::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
		RESTAPI_utils::field_to_json(Obj,"oldUUID", oldUUID);
		RESTAPI_utils::field_to_json(Obj,"newUUID", newUUID);
	}

	inline bool WebNotificationSingleDeviceConfigurationChange::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
			RESTAPI_utils::field_from_json(Obj,"oldUUID", oldUUID);
			RESTAPI_utils::field_from_json(Obj,"newUUID", newUUID);
			return true;
		} catch (...) {

		}
		return false;
	}

	inline void WebNotificationSingleDeviceFirmwareChange::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
		RESTAPI_utils::field_to_json(Obj,"newFirmware", newFirmware);
	}

	inline bool WebNotificationSingleDeviceFirmwareChange::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
			RESTAPI_utils::field_from_json(Obj,"newFirmware", newFirmware);
			return true;
		} catch (...) {

		}
		return false;
	}

	inline void WebSocketClientNotificationNumberOfConnection::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj,"numberOfDevices", numberOfDevices);
		RESTAPI_utils::field_to_json(Obj,"averageConnectedTime", averageConnectedTime);
		RESTAPI_utils::field_to_json(Obj,"numberOfConnectingDevices", numberOfConnectingDevices);
	}

	inline bool WebSocketClientNotificationNumberOfConnection::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj,"numberOfDevices", numberOfDevices);
			RESTAPI_utils::field_from_json(Obj,"averageConnectedTime", averageConnectedTime);
			RESTAPI_utils::field_from_json(Obj,"numberOfConnectingDevices", numberOfConnectingDevices);
			return true;
		} catch (...) {

		}
		return false;
	}

	void WebSocketClientNotificationNumberOfConnections(WebSocketClientNotificationNumberOfConnection_t &N) {
		N.type = "device_connections_statistics";
		UI_WebSocketClientServer()->SendNotification(N);
	}
	void WebSocketClientNotificationNumberOfConnections(const std::string & User, WebSocketClientNotificationNumberOfConnection_t &N) {
		N.type = "device_connections_statistics";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}

	void WebSocketClientNotificationDeviceConfigurationChange(WebNotificationSingleDeviceConfigurationChange_t &N) {
		N.type = "device_configuration_upgrade";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketClientNotificationDeviceConfigurationChange(const std::string & User, WebNotificationSingleDeviceConfigurationChange_t &N) {
		N.type = "device_configuration_upgrade";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}

	void WebSocketClientNotificationDeviceFirmwareUpdated(WebNotificationSingleDeviceFirmwareChange_t &N) {
		N.type = "device_firmware_upgrade";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketClientNotificationDeviceFirmwareUpdated(const std::string & User, WebNotificationSingleDeviceFirmwareChange_t &N){
		N.type = "device_firmware_upgrade";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}

	void WebSocketClientNotificationDeviceConnected(WebNotificationSingleDevice_t &N){
		N.type = "device_connection";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketClientNotificationDeviceConnected(const std::string & User, WebNotificationSingleDevice_t &N){
		N.type = "device_connection";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}

	void WebSocketClientNotificationDeviceDisconnected(const std::string & User, WebNotificationSingleDevice_t &N){
		N.type = "device_disconnection";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}

	void WebSocketClientNotificationDeviceDisconnected(WebNotificationSingleDevice_t &N){
		N.type = "device_disconnection";
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void WebSocketClientNotificationDeviceStatistics(const std::string & User, WebNotificationSingleDevice_t &N){
		N.type = "device_statistics";
		UI_WebSocketClientServer()->SendUserNotification(User,N);
	}

	void WebSocketClientNotificationDeviceStatistics(WebNotificationSingleDevice_t &N){
		N.type = "device_statistics";
		UI_WebSocketClientServer()->SendNotification(N);
	}


}