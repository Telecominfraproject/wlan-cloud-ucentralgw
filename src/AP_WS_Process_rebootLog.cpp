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
			poco_warning(Logger_, fmt::format("REBOOT-LOG({}): new entry.", CId_));
			std::string LogText;
			auto InfoLines = ParamsObj->getArray(uCentralProtocol::INFO);
			for (const auto &InfoLine : *InfoLines) {
				LogText += InfoLine.toString() + "\r\n";
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
			poco_warning(Logger_, fmt::format("REBOOT-LOG({}): Missing parameters.", CId_));
		}
	}
} // namespace OpenWifi