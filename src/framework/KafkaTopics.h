//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <string>
namespace OpenWifi::KafkaTopics {
	inline const char * HEALTHCHECK = "healthcheck";
	inline const char * STATE = "state";
	inline const char * CONNECTION = "connection";
	inline const char * WIFISCAN = "wifiscan";
	inline const char * ALERTS = "alerts";
	inline const char * COMMAND = "command";
	inline const char * SERVICE_EVENTS = "service_events";
	inline const char * DEVICE_EVENT_QUEUE = "device_event_queue";
	inline const char * DEVICE_TELEMETRY = "device_telemetry";
	inline const char * PROVISIONING_CHANGE = "provisioning_change";
	inline const char * RRM = "rrm";

	namespace ServiceEvents {
		inline const char * EVENT_JOIN = "join";
		inline const char * EVENT_LEAVE = "leave";
		inline const char * EVENT_KEEP_ALIVE = "keep-alive";
		inline const char * EVENT_REMOVE_TOKEN = "remove-token";

		namespace Fields {
			inline const char * EVENT = "event";
			inline const char * ID = "id";
			inline const char * TYPE = "type";
			inline const char * PUBLIC = "publicEndPoint";
			inline const char * PRIVATE = "privateEndPoint";
			inline const char * KEY = "key";
			inline const char * VRSN = "version";
			inline const char * TOKEN = "token";
		} // namespace Fields
	}	  // namespace ServiceEvents
} // namespace OpenWifi::KafkaTopics
