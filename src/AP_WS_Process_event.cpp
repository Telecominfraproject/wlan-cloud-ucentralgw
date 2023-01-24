//
// Created by stephane bourque on 2023-01-22.
//
#include "AP_WS_Connection.h"
#include "StorageService.h"

#include "framework/ow_constants.h"
#include "fmt/format.h"
#include "framework/KafkaManager.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_event(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(
				Logger_,
				fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol", CId_, CN_));
			Errors_++;
			return;
		}
		poco_trace(Logger_, fmt::format("Event data received for {}", SerialNumber_));

		if (ParamsObj->has(uCentralProtocol::SERIAL) && ParamsObj->has(uCentralProtocol::DATA)) {
			if (KafkaManager()->Enabled()) {
				auto Data = ParamsObj->get(uCentralProtocol::DATA);
				Poco::JSON::Stringifier Stringify;
				std::ostringstream OS;
				Stringify.condense(ParamsObj, OS);
				KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, SerialNumber_, OS.str());
			}
		}
	}
}