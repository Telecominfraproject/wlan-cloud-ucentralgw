//
// Created by euphokumiko on 2025-05-19.
//

#include "AP_WS_Connection.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/ow_constants.h"
#include <GWKafkaEvents.h>

namespace OpenWifi {
	void AP_WS_Connection::Process_packagelist(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_,
						 fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol",
									 CId_, CN_));
			Errors_++;
			return;
		}

		poco_trace(Logger_, fmt::format("PACKAGE_LIST({}): new entry.", CId_));
		return;
	}

	void AP_WS_Connection::Process_packageinstall(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_,
						 fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol",
									 CId_, CN_));
			Errors_++;
			return;
		}

		if (ParamsObj->has(uCentralProtocol::PACKAGE) && ParamsObj->has(uCentralProtocol::CATEGORY)) {
			poco_trace(Logger_, fmt::format("PACKAGE_INSTALL({}): new entry.", CId_));
			
		} else {
			poco_warning(Logger_, fmt::format("LOG({}): Missing parameters.", CId_));
			return;
		}
	}

	void AP_WS_Connection::Process_packageremove(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(Logger_,
						 fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol",
									 CId_, CN_));
			Errors_++;
			return;
		}

		if (ParamsObj->has(uCentralProtocol::PACKAGE)) {
			poco_trace(Logger_, fmt::format("PACKAGE_REMOVE({}): new entry.", CId_));
			
		} else {
			poco_warning(Logger_, fmt::format("LOG({}): Missing parameters.", CId_));
			return;
		}
	}
} // namespace OpenWifi