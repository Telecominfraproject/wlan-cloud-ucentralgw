//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "CommandManager.h"
#include "TelemetryStream.h"

#include "fmt/format.h"
#include "framework/KafkaManager.h"
#include "framework/utils.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_telemetry(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_,
						 fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol",
									 CId_, CN_));
			Errors_++;
			return;
		}
		poco_trace(Logger_, fmt::format("Telemetry data received for {}", SerialNumber_));
		if (TelemetryReporting_ || ParamsObj->has("adhoc")) {
			if (ParamsObj->has("data")) {
				auto Payload = ParamsObj->get("data").extract<Poco::JSON::Object::Ptr>();
				Payload->set("timestamp", Utils::Now());
				std::ostringstream SS;
				Payload->stringify(SS);
				auto now = Utils::Now();
				if (ParamsObj->has("adhoc")) {
					KafkaManager()->PostMessage(KafkaTopics::DEVICE_TELEMETRY, SerialNumber_,
												SS.str());
					return;
				}
				if (TelemetryWebSocketRefCount_) {
					if (now < TelemetryWebSocketTimer_) {
						// std::cout << SerialNumber_ << ": Updating WebSocket telemetry" <<
						// std::endl;
						TelemetryWebSocketPackets_++;
						State_.websocketPackets = TelemetryWebSocketPackets_;
						TelemetryStream()->NotifyEndPoint(SerialNumberInt_, SS.str());
					} else {
						StopWebSocketTelemetry(CommandManager()->Next_RPC_ID());
					}
				}
				if (TelemetryKafkaRefCount_) {
					if (KafkaManager()->Enabled() && now < TelemetryKafkaTimer_) {
						// std::cout << SerialNumber_ << ": Updating Kafka telemetry" << std::endl;
						TelemetryKafkaPackets_++;
						State_.kafkaPackets = TelemetryKafkaPackets_;
						KafkaManager()->PostMessage(KafkaTopics::DEVICE_TELEMETRY, SerialNumber_,
													SS.str());
					} else {
						StopKafkaTelemetry(CommandManager()->Next_RPC_ID());
					}
				}
			} else {
				poco_debug(Logger_,
						   fmt::format("TELEMETRY({}): Invalid telemetry packet.", SerialNumber_));
			}
		} else {
			// if we are ignoring telemetry, then close it down on the device.
			poco_debug(Logger_,
					   fmt::format("TELEMETRY({}): Stopping runaway telemetry.", SerialNumber_));
			StopTelemetry(CommandManager()->Next_RPC_ID());
		}
	}
} // namespace OpenWifi