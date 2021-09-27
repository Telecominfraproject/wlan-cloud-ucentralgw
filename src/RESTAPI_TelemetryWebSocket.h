//
// Created by stephane bourque on 2021-09-16.
//

#ifndef OWGW_RESTAPI_TELEMETRYWEBSOCKET_H
#define OWGW_RESTAPI_TELEMETRYWEBSOCKET_H

#include "RESTAPI_handler.h"

namespace OpenWifi {
class RESTAPI_TelemetryWebSocket : public RESTAPIHandler {
  public:
	RESTAPI_TelemetryWebSocket(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
	: RESTAPIHandler(bindings, L,
					 std::vector<std::string>{	Poco::Net::HTTPRequest::HTTP_GET,
												  Poco::Net::HTTPRequest::HTTP_OPTIONS},
												  Server, Internal,false) {}
												  static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/ws_telemetry"};}
  	void DoGet() final;
	void DoDelete() final {};
	void DoPost() final {};
	void DoPut() final {};
  private:
	void Process(const Poco::JSON::Object::Ptr &O, std::string &Answer);
};
}

#endif // OWGW_RESTAPI_TELEMETRYWEBSOCKET_H
