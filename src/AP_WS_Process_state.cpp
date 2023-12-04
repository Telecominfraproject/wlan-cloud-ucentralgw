//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "AP_WS_Server.h"
#include "StateUtils.h"
#include "StorageService.h"

#include "UI_GW_WebSocketNotifications.h"

#include "framework/KafkaManager.h"
#include "framework/utils.h"

#include "fmt/format.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_state(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_,
						 fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol",
									 CId_, CN_));
			Errors_++;
			return;
		}

		if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::STATE)) {
			uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
			auto StateStr = ParamsObj->get(uCentralProtocol::STATE).toString();
			auto StateObj = ParamsObj->getObject(uCentralProtocol::STATE);

			std::string request_uuid;
			if (ParamsObj->has(uCentralProtocol::REQUEST_UUID))
				request_uuid = ParamsObj->get(uCentralProtocol::REQUEST_UUID).toString();

			if (request_uuid.empty()) {
				poco_trace(Logger_, fmt::format("STATE({}): UUID={} Updating.", CId_, UUID));
			} else {
				poco_trace(Logger_, fmt::format("STATE({}): UUID={} Updating for CMD={}.", CId_,
												UUID, request_uuid));
			}

			std::lock_guard	Guard(*DbSession_->Mutex);
			if(!Simulated_) {
				uint64_t UpgradedUUID;
				LookForUpgrade(*DbSession_->Session, UUID, UpgradedUUID);
				State_.UUID = UpgradedUUID;
			}

			SetLastStats(StateStr);

			GWObjects::Statistics Stats{
				.SerialNumber = SerialNumber_, .UUID = UUID, .Data = StateStr};
			Stats.Recorded = Utils::Now();
			StorageService()->AddStatisticsData(*DbSession_->Session,Stats);
			if (!request_uuid.empty()) {
				StorageService()->SetCommandResult(request_uuid, StateStr);
			}

			StateUtils::ComputeAssociations(StateObj, State_.Associations_2G,
											State_.Associations_5G, State_.Associations_6G);

			if (KafkaManager()->Enabled() && !AP_WS_Server()->KafkaDisableState()) {
				KafkaManager()->PostMessage(KafkaTopics::STATE, SerialNumber_, *ParamsObj);
			}

			GWWebSocketNotifications::SingleDevice_t N;
			N.content.serialNumber = SerialNumber_;
			GWWebSocketNotifications::DeviceStatistics(N);

		} else {
			poco_warning(
				Logger_,
				fmt::format("STATE({}): Invalid request. Missing serial, uuid, or state", CId_));
		}
	}
} // namespace OpenWifi