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
	static const std::string HEALTHCHECK{"healthcheck"};
	static const std::string STATE{"state"};
	static const std::string CONNECTION{"connection"};
	static const std::string WIFISCAN{"wifiscan"};
	static const std::string ALERTS{"alerts"};
	static const std::string COMMAND{"command"};
	static const std::string SERVICE_EVENTS{"service_events"};
	static const std::string DEVICE_EVENT_QUEUE{"device_event_queue"};
	static const std::string DEVICE_TELEMETRY{"device_telemetry"};
    static const std::string PROVISIONING_CHANGE{"provisioning_change"};

	namespace ServiceEvents {
		static const std::string EVENT_JOIN{"join"};
		static const std::string EVENT_LEAVE{"leave"};
		static const std::string EVENT_KEEP_ALIVE{"keep-alive"};
		static const std::string EVENT_REMOVE_TOKEN{"remove-token"};

		namespace Fields {
			static const std::string EVENT{"event"};
			static const std::string ID{"id"};
			static const std::string TYPE{"type"};
			static const std::string PUBLIC{"publicEndPoint"};
			static const std::string PRIVATE{"privateEndPoint"};
			static const std::string KEY{"key"};
			static const std::string VRSN{"version"};
			static const std::string TOKEN{"token"};
		}
	}
}

