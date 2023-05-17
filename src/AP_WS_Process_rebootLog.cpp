//
// Created by stephane bourque on 2023-05-16.
//
#include "AP_WS_Connection.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/ow_constants.h"

namespace OpenWifi {

	void StripNulls(std::string &S) {
		for(std::size_t i=0;i<S.size();++i) {
			if(S[i]==0)
				S[i]=' ';
		}
	}

	void AP_WS_Connection::Process_rebootLog(Poco::JSON::Object::Ptr ParamsObj) {
		if (ParamsObj->has(uCentralProtocol::UUID)
			&& ParamsObj->isArray(uCentralProtocol::INFO)
			&& ParamsObj->has(uCentralProtocol::TYPE)
			&& ParamsObj->has(uCentralProtocol::DATE) ) {
			poco_warning(Logger_, fmt::format("REBOOT-LOG({}): new entry.", CId_));

			auto InfoLines = ParamsObj->getArray(uCentralProtocol::INFO);
			std::ostringstream os;
			InfoLines->stringify(os);

			GWObjects::DeviceLog DeviceLog{.SerialNumber = SerialNumber_,
										   .Log = ParamsObj->get(uCentralProtocol::TYPE).toString(),
										   .Data = "{" + os.str() + "}",
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