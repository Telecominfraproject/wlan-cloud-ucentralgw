//
// Created by stephane bourque on 2021-07-21.
//

#ifndef UCENTRALGW_RESTAPI_DEVICEDASHBOARDHANDLER_H
#define UCENTRALGW_RESTAPI_DEVICEDASHBOARDHANDLER_H

#include "RESTAPI_handler.h"

namespace OpenWifi {
class RESTAPI_deviceDashboardHandler : public RESTAPIHandler {
  public:
	RESTAPI_deviceDashboardHandler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{
							 Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
							 Poco::Net::HTTPRequest::HTTP_OPTIONS}, Internal) {}
	static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/deviceDashboard"};}
	void DoGet() final;
	void DoDelete() final {};
	void DoPost() final {};
	void DoPut() final {};
};
}

#endif // UCENTRALGW_RESTAPI_DEVICEDASHBOARDHANDLER_H
