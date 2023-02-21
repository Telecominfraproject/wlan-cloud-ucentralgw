//
// Created by stephane bourque on 2022-10-29.
//

#include "UI_GW_WebSocketNotifications.h"

namespace OpenWifi::GWWebSocketNotifications {

	inline void SingleDevice::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj, "serialNumber", serialNumber);
	}

	inline bool SingleDevice::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj, "serialNumber", serialNumber);
			return true;
		} catch (...) {
		}
		return false;
	}

	inline void SingleDeviceConfigurationChange::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj, "serialNumber", serialNumber);
		RESTAPI_utils::field_to_json(Obj, "oldUUID", oldUUID);
		RESTAPI_utils::field_to_json(Obj, "newUUID", newUUID);
	}

	inline bool SingleDeviceConfigurationChange::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj, "serialNumber", serialNumber);
			RESTAPI_utils::field_from_json(Obj, "oldUUID", oldUUID);
			RESTAPI_utils::field_from_json(Obj, "newUUID", newUUID);
			return true;
		} catch (...) {
		}
		return false;
	}

	inline void SingleDeviceFirmwareChange::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj, "serialNumber", serialNumber);
		RESTAPI_utils::field_to_json(Obj, "newFirmware", newFirmware);
	}

	inline bool SingleDeviceFirmwareChange::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj, "serialNumber", serialNumber);
			RESTAPI_utils::field_from_json(Obj, "newFirmware", newFirmware);
			return true;
		} catch (...) {
		}
		return false;
	}

	inline void NumberOfConnection::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj, "numberOfDevices", numberOfDevices);
		RESTAPI_utils::field_to_json(Obj, "averageConnectedTime", averageConnectedTime);
		RESTAPI_utils::field_to_json(Obj, "numberOfConnectingDevices", numberOfConnectingDevices);
	}

	inline bool NumberOfConnection::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj, "numberOfDevices", numberOfDevices);
			RESTAPI_utils::field_from_json(Obj, "averageConnectedTime", averageConnectedTime);
			RESTAPI_utils::field_from_json(Obj, "numberOfConnectingDevices",
										   numberOfConnectingDevices);
			return true;
		} catch (...) {
		}
		return false;
	}

	void NumberOfConnections(NumberOfConnection_t &N) {
		// N.type = "device_connections_statistics";
		N.type_id = 1000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void NumberOfConnections(const std::string &User, NumberOfConnection_t &N) {
		// N.type = "device_connections_statistics";
		N.type_id = 1000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

	void DeviceConfigurationChange(SingleDeviceConfigurationChange_t &N) {
		// N.type = "device_configuration_upgrade";
		N.type_id = 2000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void DeviceConfigurationChange(const std::string &User, SingleDeviceConfigurationChange_t &N) {
		// N.type = "device_configuration_upgrade";
		N.type_id = 2000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

	void DeviceFirmwareUpdated(SingleDeviceFirmwareChange_t &N) {
		// N.type = "device_firmware_upgrade";
		N.type_id = 3000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void DeviceFirmwareUpdated(const std::string &User, SingleDeviceFirmwareChange_t &N) {
		// N.type = "device_firmware_upgrade";
		N.type_id = 3000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

	void DeviceConnected(SingleDevice_t &N) {
		// N.type = "device_connection";
		N.type_id = 4000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void DeviceConnected(const std::string &User, SingleDevice_t &N) {
		// N.type = "device_connection";
		N.type_id = 4000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

	void DeviceDisconnected(const std::string &User, SingleDevice_t &N) {
		// N.type = "device_disconnection";
		N.type_id = 5000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

	void DeviceDisconnected(SingleDevice_t &N) {
		// N.type = "device_disconnection";
		N.type_id = 5000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void DeviceStatistics(const std::string &User, SingleDevice_t &N) {
		// N.type = "device_statistics";
		N.type_id = 6000;
		UI_WebSocketClientServer()->SendUserNotification(User, N);
	}

	void DeviceStatistics(SingleDevice_t &N) {
		// N.type = "device_statistics";
		N.type_id = 6000;
		UI_WebSocketClientServer()->SendNotification(N);
	}

	void Register() {
		static const UI_WebSocketClientServer::NotificationTypeIdVec Notifications = {
			{1000, "device_connections_statistics"}, {2000, "device_configuration_upgrade"},
			{3000, "device_firmware_upgrade"},		 {4000, "device_connection"},
			{5000, "device_disconnection"},			 {6000, "device_statistics"}};

		UI_WebSocketClientServer()->RegisterNotifications(Notifications);
	}

} // namespace OpenWifi::GWWebSocketNotifications