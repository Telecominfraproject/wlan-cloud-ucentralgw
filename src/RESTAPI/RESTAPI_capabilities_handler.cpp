//
// Created by stephane bourque on 2021-10-17.
//

#include "RESTAPI_capabilities_handler.h"
#include "CapabilitiesCache.h"

namespace OpenWifi {

	void RESTAPI_capabilities_handler::DoGet() {
		const CapabilitiesCache_t & Caps = CapabilitiesCache().AllCapabilities();

		Poco::JSON::Array	ObjArr;
		for(const auto &[deviceType,capabilities]:Caps) {
			Poco::JSON::Object	Inner;
			Inner.set("deviceType",deviceType);
			Poco::JSON::Parser	P;
			auto R = P.parse(to_string(capabilities)).extract<Poco::JSON::Object::Ptr>();
			Inner.set("capabilities", R);
			ObjArr.add(Inner);
		}
		Poco::JSON::Object	Answer;

		Answer.set("devices",ObjArr);
		return ReturnObject(Answer);
	}
}