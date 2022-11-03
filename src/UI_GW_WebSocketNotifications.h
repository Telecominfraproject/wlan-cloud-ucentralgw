//
// Created by stephane bourque on 2022-10-29.
//

#pragma once

#include "framework/UI_WebSocketClientNotifications.h"
#include "framework/UI_WebSocketClientServer.h"

namespace OpenWifi::GWWebSocketNotifications {

	struct SingleDevice {
		std::string		serialNumber;
		inline void to_json(Poco::JSON::Object &Obj) const ;
		inline bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct SingleDeviceConfigurationChange {
		std::string		serialNumber;
		uint64_t 		oldUUID;
		uint64_t 		newUUID;

		inline void to_json(Poco::JSON::Object &Obj) const ;
		inline bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct SingleDeviceFirmwareChange {
		std::string		serialNumber;
		std::string		newFirmware;
		inline void to_json(Poco::JSON::Object &Obj) const ;
		inline bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	struct NumberOfConnection {
		std::uint64_t 	numberOfDevices=0;
		std::uint64_t 	averageConnectedTime=0;
		std::uint64_t 	numberOfConnectingDevices=0;

		inline void to_json(Poco::JSON::Object &Obj) const ;
		inline bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	void Register();

	typedef WebSocketNotification<SingleDevice> SingleDevice_t;
	typedef WebSocketNotification<SingleDeviceConfigurationChange> SingleDeviceConfigurationChange_t;
	typedef WebSocketNotification<SingleDeviceFirmwareChange> SingleDeviceFirmwareChange_t;
	typedef WebSocketNotification<NumberOfConnection> NumberOfConnection_t;

	void NumberOfConnections(NumberOfConnection_t &N);
	void DeviceConfigurationChange(SingleDeviceConfigurationChange_t &N);
	void DeviceFirmwareUpdated(SingleDeviceFirmwareChange_t &);
	void DeviceConnected(SingleDevice_t &N);
	void DeviceDisconnected(SingleDevice_t &N);
	void DeviceStatistics(SingleDevice_t &N);

	void NumberOfConnections(const std::string & User, NumberOfConnection_t &N);
	void DeviceConfigurationChange(const std::string & User, SingleDeviceConfigurationChange_t &N);
	void DeviceFirmwareUpdated(const std::string & User, SingleDeviceFirmwareChange_t &);
	void DeviceConnected(const std::string & User, SingleDevice_t &N);
	void DeviceDisconnected(const std::string & User, SingleDevice_t &N);
	void DeviceStatistics(const std::string & User, SingleDevice_t &N);

};