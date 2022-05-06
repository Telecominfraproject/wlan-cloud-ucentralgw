//
// Created by stephane bourque on 2022-05-05.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

	struct WebNotificationSingleDevice {
		std::string		serialNumber;
		inline void to_json(Poco::JSON::Object &Obj) const {
			RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
		}

		inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
			try {
				RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
				return true;
			} catch (...) {

			}
			return false;
		}
	};

	struct WebNotificationSingleDeviceFirmwareChange {
		std::string		serialNumber;
		std::string		newFirmware;
		inline void to_json(Poco::JSON::Object &Obj) const {
			RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
			RESTAPI_utils::field_to_json(Obj,"newFirmware", newFirmware);
		}

		inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
			try {
				RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
				RESTAPI_utils::field_from_json(Obj,"newFirmware", newFirmware);
				return true;
			} catch (...) {

			}
			return false;
		}
	};

	inline void WebSocketClientNotificationDeviceFirmwareUpdated(const std::string &SerialNumber, const std::string &Firmware) {
		WebSocketNotification<WebNotificationSingleDeviceFirmwareChange>	N;
		N.content.serialNumber = SerialNumber;
		N.content.newFirmware = Firmware;
		N.type = "device_firmware_upgrade";
		WebSocketClientServer()->SendNotification(N);
	}

	inline void WebSocketClientNotificationDeviceConnected(const std::string &SerialNumber) {
		WebSocketNotification<WebNotificationSingleDevice>	N;
		N.content.serialNumber = SerialNumber;
		N.type = "device_connection";
		WebSocketClientServer()->SendNotification(N);
	}

	inline void WebSocketClientNotificationDeviceDisconnected(const std::string & SerialNumber) {
		WebSocketNotification<WebNotificationSingleDevice>	N;
		N.content.serialNumber = SerialNumber;
		N.type = "device_disconnection";
		WebSocketClientServer()->SendNotification(N);
	}

} // namespace OpenWifi

