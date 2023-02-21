//
// Created by stephane bourque on 2023-02-03.
//

#include "GWKafkaEvents.h"

namespace OpenWifi {

	void GWKafkaEvents::Send() {
		if (KafkaManager()->Enabled()) {
			Poco::JSON::Object Event;
			Event.set("type", type_);
			Event.set("timestamp", timestamp_);
			Event.set("payload", payload_);
			std::ostringstream OS;
			Event.stringify(OS);
			KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, serialNumber_, OS.str());
		}
	}

} // namespace OpenWifi