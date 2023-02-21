//
// Created by stephane bourque on 2021-10-14.
//

#include "RESTAPI_blacklist_list.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_blacklist_list::DoGet() {

		poco_debug(Logger(), fmt::format("BLACKLIST-GET: Device serial number list"));

		std::vector<GWObjects::BlackListedDevice> Devices;

		if (QB_.CountOnly) {
			auto Count = StorageService()->GetBlackListDeviceCount();
			return ReturnCountOnly(Count);
		} else if (StorageService()->GetBlackListDevices(QB_.Offset, QB_.Limit, Devices)) {
			return Object("devices", Devices);
		}
		NotFound();
	}
} // namespace OpenWifi