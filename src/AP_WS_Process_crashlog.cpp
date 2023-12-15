//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/ow_constants.h"
#include <GWKafkaEvents.h>

namespace OpenWifi {
	void AP_WS_Connection::Process_crashlog(Poco::JSON::Object::Ptr ParamsObj) {
		if (ParamsObj->has(uCentralProtocol::UUID)
			&& ParamsObj->has(uCentralProtocol::LOGLINES)) {
			poco_trace(Logger_, fmt::format("CRASH-LOG({}): new entry.", CId_));
			auto LogLines = ParamsObj->get(uCentralProtocol::LOGLINES);
			std::string LogText;
			if (LogLines.isArray()) {
				auto LogLinesArray = LogLines.extract<Poco::JSON::Array::Ptr>();
				for (const auto &i : *LogLinesArray)
					LogText += i.toString() + "\r\n";
			}

			GWObjects::DeviceLog DeviceLog{.SerialNumber = SerialNumber_,
										   .Log = LogText,
										   .Data = "",
										   .Severity = GWObjects::DeviceLog::LOG_EMERG,
										   .Recorded = Utils::Now(),
										   .LogType = 1,
										   .UUID = ParamsObj->get(uCentralProtocol::UUID)};
			StorageService()->AddLog(*DbSession_, DeviceLog);
			DeviceLogKafkaEvent	E(DeviceLog);
		} else {
			poco_warning(Logger_, fmt::format("LOG({}): Missing parameters.", CId_));
			return;
		}
	}
} // namespace OpenWifi