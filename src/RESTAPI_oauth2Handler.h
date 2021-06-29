//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_OAUTH2HANDLER_H
#define UCENTRAL_RESTAPI_OAUTH2HANDLER_H

#include "RESTAPI_handler.h"

namespace uCentral {
	class RESTAPI_oauth2Handler : public RESTAPIHandler {
	  public:
		RESTAPI_oauth2Handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
													  Poco::Net::HTTPRequest::HTTP_DELETE,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
		void handleRequest(Poco::Net::HTTPServerRequest &request,
						   Poco::Net::HTTPServerResponse &response) override;
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/oauth2/{token}","/api/v1/oauth2"}; };
	};
}
#endif //UCENTRAL_RESTAPI_OAUTH2HANDLER_H
