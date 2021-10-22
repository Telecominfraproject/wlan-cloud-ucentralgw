//
// Created by stephane bourque on 2021-10-17.
//

#ifndef UCENTRALGW_RESTAPI_CAPABILITIES_HANDLER_H
#define UCENTRALGW_RESTAPI_CAPABILITIES_HANDLER_H

#include "framework/MicroService.h"

namespace OpenWifi {
	class RESTAPI_capabilities_handler : public RESTAPIHandler {
	  public:
		RESTAPI_capabilities_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
												  Poco::Net::HTTPRequest::HTTP_OPTIONS},
												  Server,
												  Internal) {}
												  static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/capabilities"};}
		void DoGet() final;
		void DoDelete() final {};
		void DoPost() final {};
		void DoPut() final {};
	};
}
#endif // UCENTRALGW_RESTAPI_CAPABILITIES_HANDLER_H
