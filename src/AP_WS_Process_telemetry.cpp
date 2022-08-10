//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "TelemetryStream.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_telemetry(Poco::JSON::Object::Ptr ParamsObj) {
		if (!Connected_) {
			poco_warning(Logger(), fmt::format(
									   "INVALID-PROTOCOL({}): Device '{}' is not following protocol", CId_, CN_));
			Errors_++;
			return;
		}
		if (TelemetryReporting_) {
			if (ParamsObj->has("data")) {
				auto Payload = ParamsObj->get("data").extract<Poco::JSON::Object::Ptr>();
				Payload->set("timestamp", OpenWifi::Now());
				std::ostringstream SS;
				Payload->stringify(SS);
				auto now=OpenWifi::Now();
				if (TelemetryWebSocketRefCount_) {
					if(now<TelemetryWebSocketTimer_) {
						// std::cout << SerialNumber_ << ": Updating WebSocket telemetry" << std::endl;
						TelemetryWebSocketPackets_++;
						Session_->State_.websocketPackets = TelemetryWebSocketPackets_;
						TelemetryStream()->UpdateEndPoint(SerialNumberInt_, SS.str());
					} else {
						StopWebSocketTelemetry();
					}
				}
				if (TelemetryKafkaRefCount_) {
					if(KafkaManager()->Enabled() && now<TelemetryKafkaTimer_) {
						// std::cout << SerialNumber_ << ": Updating Kafka telemetry" << std::endl;
						TelemetryKafkaPackets_++;
						Session_->State_.kafkaPackets = TelemetryKafkaPackets_;
						KafkaManager()->PostMessage(KafkaTopics::DEVICE_TELEMETRY, SerialNumber_,
													SS.str());
					} else {
						StopKafkaTelemetry();
					}
				}
			} else {
				poco_debug(Logger(),fmt::format("TELEMETRY({}): Invalid telemetry packet.",SerialNumber_));
			}
		} else {
			// if we are ignoring telemetry, then close it down on the device.
			poco_debug(Logger(),fmt::format("TELEMETRY({}): Stopping runaway telemetry.",SerialNumber_));
			StopTelemetry();
		}
	}
}