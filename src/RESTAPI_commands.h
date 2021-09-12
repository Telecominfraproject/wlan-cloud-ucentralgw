//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_COMMANDS_H
#define UCENTRAL_RESTAPI_COMMANDS_H

#include "RESTAPI_handler.h"

namespace OpenWifi {
class RESTAPI_commands : public RESTAPIHandler {
  public:
	RESTAPI_commands(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
												  Poco::Net::HTTPRequest::HTTP_DELETE,
												  Poco::Net::HTTPRequest::HTTP_OPTIONS},
						 Internal) {}
	static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/commands"};}
	void DoGet() final;
	void DoDelete() final;
	void DoPost() final {};
	void DoPut() final {};
};
}
#endif //UCENTRAL_RESTAPI_COMMANDS_H
