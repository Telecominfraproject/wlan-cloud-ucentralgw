//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_COMMAND_H
#define UCENTRAL_RESTAPI_COMMAND_H

#include "framework/MicroService.h"

namespace OpenWifi {
class RESTAPI_command : public RESTAPIHandler {
  public:
	RESTAPI_command(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
												  Poco::Net::HTTPRequest::HTTP_DELETE,
												  Poco::Net::HTTPRequest::HTTP_OPTIONS},
						 Server,
						 Internal) {}
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/command/{commandUUID}"};}
		void DoGet() final;
		void DoDelete() final;
		void DoPost() final {};
		void DoPut() final {};
	};
}

#endif //UCENTRAL_RESTAPI_COMMAND_H
