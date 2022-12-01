//
// Created by stephane bourque on 2021-10-14.
//

#include "RESTAPI_blacklist_list.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_blacklist_list::DoGet() {

		std::vector<GWObjects::BlackListedDevice>	Devices;

		poco_debug(Logger(),fmt::format("BLACKLIST-GET: Device serial number list"));

		Poco::JSON::Array	Arr;
		Poco::JSON::Object	Answer;

		if(QB_.CountOnly) {
			auto Count = StorageService()->GetBlackListDeviceCount();
			return ReturnCountOnly(Count);
		} else if(StorageService()->GetBlackListDevices(QB_.Offset, QB_.Limit, Devices)) {
			for(const auto &i:Devices) {
				Poco::JSON::Object O;
				i.to_json(O);
				Arr.add(O);
			}
		}
		Answer.set("devices", Arr);
		return ReturnObject(Answer);
	}
}