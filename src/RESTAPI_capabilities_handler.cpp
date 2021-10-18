//
// Created by stephane bourque on 2021-10-17.
//

#include "RESTAPI_capabilities_handler.h"
#include "StorageService.h"

namespace OpenWifi {

	void RESTAPI_capabilities_handler::DoGet() {
		Storage::DeviceCapabilitiesCache	DevCaps;

		Storage()->GetDeviceCapabilitiesCache(DevCaps);
		Poco::JSON::Array	ObjArr;
		for(const auto &[deviceType,capabilities]:DevCaps) {
			Poco::JSON::Object	Inner;
			Inner.set("deviceType",deviceType);
			Poco::JSON::Parser	P;
			auto R = P.parse(capabilities).extract<Poco::JSON::Object::Ptr>();
			Inner.set("capabilities", R);
			ObjArr.add(Inner);
		}
		Poco::JSON::Object	Answer;

		Answer.set("devices",ObjArr);
		return ReturnObject(Answer);
	}
}