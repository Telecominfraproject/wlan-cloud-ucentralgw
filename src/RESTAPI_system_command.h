//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_RESTAPI_SYSTEM_COMMAND_H
#define UCENTRALGW_RESTAPI_SYSTEM_COMMAND_H

#include "RESTAPI_handler.h"

namespace OpenWifi {
class RESTAPI_system_command : public RESTAPIHandler {
  public:
	RESTAPI_system_command(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
														  Poco::Net::HTTPRequest::HTTP_GET,
														  Poco::Net::HTTPRequest::HTTP_OPTIONS},
						 Internal) {}
	static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/system"};}

	void DoGet();
	void DoPost();
	void DoPut();
	void DoDelete();
	};
}
#endif // UCENTRALGW_RESTAPI_SYSTEM_COMMAND_H
