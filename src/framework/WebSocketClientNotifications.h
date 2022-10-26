//
// Created by stephane bourque on 2022-05-05.
//

#pragma once

#include "framework/RESTAPI_utils.h"
#include "framework/utils.h"

namespace OpenWifi {

	template <typename ContentStruct> struct WebSocketNotification {
		inline static uint64_t          xid=1;
		uint64_t                        notification_id=++xid;
		std::string						type;
		ContentStruct    				content;

		void to_json(Poco::JSON::Object &Obj) const;
		bool from_json(const Poco::JSON::Object::Ptr &Obj);
	};

	template <typename ContentStruct> void WebSocketNotification<ContentStruct>::to_json(Poco::JSON::Object &Obj) const {
		RESTAPI_utils::field_to_json(Obj,"notification_id",notification_id);
		RESTAPI_utils::field_to_json(Obj,"type",type);
		RESTAPI_utils::field_to_json(Obj,"content",content);
	}

	template <typename ContentStruct> bool WebSocketNotification<ContentStruct>::from_json(const Poco::JSON::Object::Ptr &Obj) {
		try {
			RESTAPI_utils::field_from_json(Obj,"notification_id",notification_id);
			RESTAPI_utils::field_from_json(Obj,"content",content);
			RESTAPI_utils::field_from_json(Obj,"type",type);
			return true;
		} catch(...) {

		}
		return false;
	}

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

	struct WebNotificationSingleDeviceConfigurationChange {
		std::string		serialNumber;
		uint64_t 		oldUUID;
		uint64_t 		newUUID;

		inline void to_json(Poco::JSON::Object &Obj) const {
			RESTAPI_utils::field_to_json(Obj,"serialNumber", serialNumber);
			RESTAPI_utils::field_to_json(Obj,"oldUUID", oldUUID);
			RESTAPI_utils::field_to_json(Obj,"newUUID", newUUID);
		}

		inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
			try {
				RESTAPI_utils::field_from_json(Obj,"serialNumber", serialNumber);
				RESTAPI_utils::field_from_json(Obj,"oldUUID", oldUUID);
				RESTAPI_utils::field_from_json(Obj,"newUUID", newUUID);
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

	struct WebSocketClientNotificationNumberOfConnection {
		std::uint64_t 	numberOfDevices=0;
		std::uint64_t 	averageConnectedTime=0;
		std::uint64_t 	numberOfConnectingDevices=0;

		inline void to_json(Poco::JSON::Object &Obj) const {
			RESTAPI_utils::field_to_json(Obj,"numberOfDevices", numberOfDevices);
			RESTAPI_utils::field_to_json(Obj,"averageConnectedTime", averageConnectedTime);
			RESTAPI_utils::field_to_json(Obj,"numberOfConnectingDevices", numberOfConnectingDevices);
		}

		inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
			try {
				RESTAPI_utils::field_from_json(Obj,"numberOfDevices", numberOfDevices);
				RESTAPI_utils::field_from_json(Obj,"averageConnectedTime", averageConnectedTime);
				RESTAPI_utils::field_from_json(Obj,"numberOfConnectingDevices", numberOfConnectingDevices);
				return true;
			} catch (...) {

			}
			return false;
		}
	};

	void WebSocketClientNotificationNumberOfConnections(std::uint64_t numberOfDevices,
													   std::uint64_t averageConnectedTime,
													   std::uint64_t numberOfConnectingDevices);
	void WebSocketClientNotificationDeviceConfigurationChange(const std::string &SerialNumber, uint64_t oldUUID, uint64_t newUUID);
	void WebSocketClientNotificationDeviceFirmwareUpdated(const std::string &SerialNumber, const std::string &Firmware);
	void WebSocketClientNotificationDeviceConnected(const std::string &SerialNumber);
	void WebSocketClientNotificationDeviceDisconnected(const std::string & SerialNumber);

    struct WebSocketNotificationJobContent {
        std::string                 title,
                                    details,
                                    jobId;
        std::vector<std::string>    success,
                                    error,
                                    warning;
        uint64_t                    timeStamp=OpenWifi::Utils::Now();

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef WebSocketNotification<WebSocketNotificationJobContent>  WebSocketClientNotificationVenueUpdateJob_t;

    void WebSocketClientNotificationVenueUpdateJobCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpdateJob_t &N);

    /////
    /////
    /////

    struct WebSocketNotificationRebootList {
        std::string                 title,
                details,
                jobId;
        std::vector<std::string>    success,
                warning;
        uint64_t                    timeStamp=OpenWifi::Utils::Now();

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef WebSocketNotification<WebSocketNotificationRebootList> WebSocketClientNotificationVenueRebootList_t;

    void WebSocketClientNotificationVenueRebootCompletionToUser( const std::string & User, WebSocketClientNotificationVenueRebootList_t &N);

    struct WebSocketNotificationUpgradeList {
        std::string                 title,
                details,
                jobId;
        std::vector<std::string>    success,
                                    skipped,
                                    no_firmware,
                                    not_connected;
        uint64_t                    timeStamp=OpenWifi::Utils::Now();

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef WebSocketNotification<WebSocketNotificationUpgradeList> WebSocketClientNotificationVenueUpgradeList_t;


    void WebSocketClientNotificationVenueUpgradeCompletionToUser( const std::string & User, WebSocketClientNotificationVenueUpgradeList_t &N);

} // namespace OpenWifi

