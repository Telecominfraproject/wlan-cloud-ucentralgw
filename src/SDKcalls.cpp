//
// Created by stephane bourque on 2021-12-20.
//

#include "SDKcalls.h"

namespace OpenWifi {

	bool SDKCalls::GetProvisioningConfiguration(const std::string &SerialNumber, std::string & Config) {

		Types::StringPairVec QD { {"config","true"}};
		OpenAPIRequestGet	API(uSERVICE_PROVISIONING,
							  	"/api/v1/inventory/" + SerialNumber + "?config=true",
							  	QD,20000);

		Poco::JSON::Object::Ptr ResponseObject;

		if(API.Do(ResponseObject)==Poco::Net::HTTPResponse::HTTP_OK) {
			if(ResponseObject->has("config")) {
				Config = ResponseObject->get("config").toString();
				return true;
			}
		}
		return false;
	}
}
