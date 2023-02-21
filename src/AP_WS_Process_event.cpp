//
// Created by stephane bourque on 2023-01-22.
//
#include "AP_WS_Connection.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/KafkaManager.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_event(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_,
						 fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol",
									 CId_, CN_));
			Errors_++;
			return;
		}
		poco_trace(Logger_, fmt::format("Event data received for {}", SerialNumber_));

		try {
			if (ParamsObj->has(uCentralProtocol::SERIAL) &&
				ParamsObj->has(uCentralProtocol::DATA)) {
				if (KafkaManager()->Enabled()) {
					auto Data = ParamsObj->getObject(uCentralProtocol::DATA);
					auto Event = Data->getArray("event");
					auto EventTimeStamp = Event->getElement<std::uint64_t>(0);
					auto EventDetails = Event->getObject(1);
					auto EventType = EventDetails->get("type").extract<std::string>();
					auto EventPayload = EventDetails->getObject("payload");

					Poco::JSON::Object FullEvent;
					FullEvent.set("type", EventType);
					FullEvent.set("timestamp", EventTimeStamp);
					FullEvent.set("payload", EventPayload);

					std::ostringstream OS;
					FullEvent.stringify(OS);
					KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, SerialNumber_,
												OS.str());
				}
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		} catch (...) {
		}
	}
} // namespace OpenWifi