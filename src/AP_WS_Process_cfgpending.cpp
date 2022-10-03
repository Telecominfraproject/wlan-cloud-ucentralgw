//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "fmt/format.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_cfgpending(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_, fmt::format(
									   "INVALID-PROTOCOL({}): Device '{}' is not following protocol", CId_, CN_));
			Errors_++;
			return;
		}
		if (ParamsObj->has(uCentralProtocol::UUID) && ParamsObj->has(uCentralProtocol::ACTIVE)) {

			[[maybe_unused]] uint64_t UUID = ParamsObj->get(uCentralProtocol::UUID);
			[[maybe_unused]] uint64_t Active = ParamsObj->get(uCentralProtocol::ACTIVE);
			poco_trace(Logger_, fmt::format("CFG-PENDING({}): Active: {} Target: {}", CId_, Active, UUID));
		} else {
			poco_warning(Logger_, fmt::format("CFG-PENDING({}): Missing some parameters", CId_));
		}
	}
}