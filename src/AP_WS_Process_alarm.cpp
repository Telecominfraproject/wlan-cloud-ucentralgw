//
// Created by stephane bourque on 2023-01-22.
//
#include "AP_WS_Connection.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/KafkaManager.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_alarm(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_,
						 fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol",
									 CId_, CN_));
			Errors_++;
			return;
		}
		poco_trace(Logger_, fmt::format("Alarm data received for {}", SerialNumber_));

		if (ParamsObj->has(uCentralProtocol::SERIAL) && ParamsObj->has(uCentralProtocol::DATA)) {
			if (KafkaManager()->Enabled()) {
				auto Data = ParamsObj->get(uCentralProtocol::DATA);
				Poco::JSON::Stringifier Stringify;
				std::ostringstream OS;
				Stringify.condense(ParamsObj, OS);
				KafkaManager()->PostMessage(KafkaTopics::ALERTS, SerialNumber_, OS.str());
			}
		}
	}
} // namespace OpenWifi