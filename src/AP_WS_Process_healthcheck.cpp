//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "StorageService.h"

namespace OpenWifi {

void AP_WS_Connection::Process_healthcheck(Poco::JSON::Object::Ptr ParamsObj, const std::string &Serial) {
	if (!Session_->State_.Connected) {
		poco_warning(Logger(), fmt::format(
								   "INVALID-PROTOCOL({}): Device '{}' is not following protocol", CId_, CN_));
		Errors_++;
		return;
	}
	if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::SANITY) &&
		ParamsObj->has(uCentralProtocol::DATA)) {
		uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
		auto Sanity = ParamsObj->get(uCentralProtocol::SANITY);
		auto CheckData = ParamsObj->get(uCentralProtocol::DATA).toString();
		if (CheckData.empty())
			CheckData = uCentralProtocol::EMPTY_JSON_DOC;

		std::string request_uuid;
		if (ParamsObj->has(uCentralProtocol::REQUEST_UUID))
			request_uuid = ParamsObj->get(uCentralProtocol::REQUEST_UUID).toString();

		if (request_uuid.empty()) {
			poco_trace(Logger(),
					   fmt::format("HEALTHCHECK({}): UUID={} Updating.", CId_, UUID));
		} else {
			poco_trace(Logger(),
					   fmt::format("HEALTHCHECK({}): UUID={} Updating for CMD={}.", CId_,
								   UUID, request_uuid));
		}

		uint64_t UpgradedUUID;
		LookForUpgrade(UUID,UpgradedUUID);
		Session_->State_.UUID = UpgradedUUID;

		GWObjects::HealthCheck Check;

		Check.SerialNumber = SerialNumber_;
		Check.Recorded = OpenWifi::Now();
		Check.UUID = UUID;
		Check.Data = CheckData;
		Check.Sanity = Sanity;

		StorageService()->AddHealthCheckData(Check);

		if (!request_uuid.empty()) {
			StorageService()->SetCommandResult(request_uuid, CheckData);
		}

		DeviceRegistry()->SetHealthcheck(Serial, Check);
		if (KafkaManager()->Enabled()) {
			Poco::JSON::Stringifier Stringify;
			std::ostringstream OS;
			ParamsObj->set("timestamp", OpenWifi::Now());
			Stringify.condense(ParamsObj, OS);
			KafkaManager()->PostMessage(KafkaTopics::HEALTHCHECK, SerialNumber_, OS.str());
		}
	} else {
		poco_warning(Logger(), fmt::format("HEALTHCHECK({}): Missing parameter", CId_));
		return;
	}
}


}