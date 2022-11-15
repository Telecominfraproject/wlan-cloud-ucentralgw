//
// Created by stephane bourque on 2021-10-17.
//

#include "RESTAPI_capabilities_handler.h"
#include "CapabilitiesCache.h"

namespace OpenWifi {

	void RESTAPI_capabilities_handler::DoGet() {
		CapabilitiesCache_t Caps = CapabilitiesCache()->AllCapabilities();

		Poco::JSON::Array	ObjArr;
		for(const auto &[deviceType,capabilities]:Caps) {
			// std::cout << "C:" << to_string(capabilities) << std::endl;
			Poco::JSON::Object	Inner;
			Inner.set("deviceType",deviceType);
			Poco::JSON::Parser	P;
			auto R = P.parse(to_string(capabilities)).extract<Poco::JSON::Object::Ptr>();
			Inner.set("capabilities", R);
			ObjArr.add(Inner);
		}
		Poco::JSON::Object	Answer;

		Answer.set("device_types",ObjArr);
		return ReturnObject(Answer);
	}
}