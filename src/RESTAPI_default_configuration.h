//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_DEFAULT_CONFIGURATION_H
#define UCENTRAL_RESTAPI_DEFAULT_CONFIGURATION_H

#include "RESTAPI_handler.h"

namespace uCentral {
class RESTAPI_default_configuration : public RESTAPIHandler {
  public:
	RESTAPI_default_configuration(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{
							 Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
							 Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
							 Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
	void handleRequest(Poco::Net::HTTPServerRequest &request,
					   Poco::Net::HTTPServerResponse &response) override;
	static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/default_configuration/{name}"};}
};
}
#endif //UCENTRAL_RESTAPI_DEFAULT_CONFIGURATION_H
