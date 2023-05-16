//
// Created by stephane bourque on 2023-05-16.
//
#include "AP_WS_Connection.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_rebootLog(Poco::JSON::Object::Ptr ParamsObj) {
		if (ParamsObj->has(uCentralProtocol::UUID)
			&& ParamsObj->isArray(uCentralProtocol::INFO)
			&& ParamsObj->has(uCentralProtocol::TYPE)
			&& ParamsObj->has(uCentralProtocol::DATE) ) {
			poco_trace(Logger_, fmt::format("REBOOT-LOG({}): new entry.", CId_));
			auto LogLines = ParamsObj->get(uCentralProtocol::INFO);
			std::string LogText;
			if (LogLines.isArray()) {
				auto LogLinesArray = LogLines.extract<Poco::JSON::Array::Ptr>();
				for (const auto &i : *LogLinesArray)
					LogText += i.toString() + "\r\n";
			}

			GWObjects::DeviceLog DeviceLog{.SerialNumber = SerialNumber_,
										   .Log = ParamsObj->get(uCentralProtocol::TYPE).toString(),
										   .Data = LogText,
										   .Severity = GWObjects::DeviceLog::LOG_INFO,
										   .Recorded = ParamsObj->get(uCentralProtocol::DATE),
										   .LogType = 2,
										   .UUID = ParamsObj->get(uCentralProtocol::UUID)};
			StorageService()->AddLog(DeviceLog);

		} else {
			poco_warning(Logger_, fmt::format("LOG({}): Missing parameters.", CId_));
			return;
		}
	}
} // namespace OpenWifi