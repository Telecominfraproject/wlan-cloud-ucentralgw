//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "StorageService.h"

#include "framework/ow_constants.h"
#include "fmt/format.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_log(Poco::JSON::Object::Ptr ParamsObj) {
		if (!State_.Connected) {
			poco_warning(
				Logger_,
				fmt::format("INVALID-PROTOCOL({}): Device '{}' is not following protocol", CId_, CN_));
			Errors_++;
			return;
		}
		if (ParamsObj->has(uCentralProtocol::LOG) && ParamsObj->has(uCentralProtocol::SEVERITY)) {
			poco_trace(Logger_, fmt::format("LOG({}): new entry.", CId_));
			auto Log = ParamsObj->get(uCentralProtocol::LOG).toString();
			auto Severity = ParamsObj->get(uCentralProtocol::SEVERITY);
			std::string DataStr = uCentralProtocol::EMPTY_JSON_DOC;
			if (ParamsObj->has(uCentralProtocol::DATA)) {
				auto DataObj = ParamsObj->get(uCentralProtocol::DATA);
				if (DataObj.isStruct())
					DataStr = DataObj.toString();
			}

			GWObjects::DeviceLog DeviceLog{.SerialNumber = SerialNumber_,
										   .Log = Log,
										   .Data = DataStr,
										   .Severity = Severity,
										   .Recorded = (uint64_t)time(nullptr),
										   .LogType = 0,
										   .UUID = State_.UUID};
			StorageService()->AddLog(DeviceLog);
		} else {
			poco_warning(Logger_, fmt::format("LOG({}): Missing parameters.", CId_));
			return;
		}
	}
}