//
// Created by stephane bourque on 2022-10-29.
//

#pragma once

#include "framework/UI_WebSocketClientNotifications.h"
#include "framework/UI_WebSocketClientServer.h"

namespace OpenWifi {
	struct WebNotificationSingleDevice {
		std::string		serialNumber;
		inline void to_json(Poco::JSON::Object &Obj) const ;
		inline bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct WebNotificationSingleDeviceConfigurationChange {
		std::string		serialNumber;
		uint64_t 		oldUUID;
		uint64_t 		newUUID;

		inline void to_json(Poco::JSON::Object &Obj) const ;
		inline bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct WebNotificationSingleDeviceFirmwareChange {
		std::string		serialNumber;
		std::string		newFirmware;
		inline void to_json(Poco::JSON::Object &Obj) const ;
		inline bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct WebSocketClientNotificationNumberOfConnection {
		std::uint64_t 	numberOfDevices=0;
		std::uint64_t 	averageConnectedTime=0;
		std::uint64_t 	numberOfConnectingDevices=0;

		inline void to_json(Poco::JSON::Object &Obj) const ;
		inline bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	typedef WebSocketNotification<WebNotificationSingleDevice> WebNotificationSingleDevice_t;
	typedef WebSocketNotification<WebNotificationSingleDeviceConfigurationChange> WebNotificationSingleDeviceConfigurationChange_t;
	typedef WebSocketNotification<WebNotificationSingleDeviceFirmwareChange> WebNotificationSingleDeviceFirmwareChange_t;
	typedef WebSocketNotification<WebSocketClientNotificationNumberOfConnection> WebSocketClientNotificationNumberOfConnection_t;

	void WebSocketClientNotificationNumberOfConnections(WebSocketClientNotificationNumberOfConnection_t &N);
	void WebSocketClientNotificationDeviceConfigurationChange(WebNotificationSingleDeviceConfigurationChange_t &N);
	void WebSocketClientNotificationDeviceFirmwareUpdated(WebNotificationSingleDeviceFirmwareChange_t &);
	void WebSocketClientNotificationDeviceConnected(WebNotificationSingleDevice_t &N);
	void WebSocketClientNotificationDeviceDisconnected(WebNotificationSingleDevice_t &N);
	void WebSocketClientNotificationDeviceStatistics(WebNotificationSingleDevice_t &N);

	void WebSocketClientNotificationNumberOfConnections(const std::string & User, WebSocketClientNotificationNumberOfConnection_t &N);
	void WebSocketClientNotificationDeviceConfigurationChange(const std::string & User, WebNotificationSingleDeviceConfigurationChange_t &N);
	void WebSocketClientNotificationDeviceFirmwareUpdated(const std::string & User, WebNotificationSingleDeviceFirmwareChange_t &);
	void WebSocketClientNotificationDeviceConnected(const std::string & User, WebNotificationSingleDevice_t &N);
	void WebSocketClientNotificationDeviceDisconnected(const std::string & User, WebNotificationSingleDevice_t &N);
	void WebSocketClientNotificationDeviceStatistics(const std::string & User, WebNotificationSingleDevice_t &N);

};