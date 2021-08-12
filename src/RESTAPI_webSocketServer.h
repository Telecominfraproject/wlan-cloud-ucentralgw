//
// Created by stephane bourque on 2021-08-12.
//

#ifndef UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H
#define UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H

class RESTAPI_webSocketServer {};

#include "RESTAPI_handler.h"

namespace uCentral {
	class RESTAPI_webSocketServer : public RESTAPIHandler {
	  public:
		RESTAPI_webSocketServer(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
												  Poco::Net::HTTPRequest::HTTP_OPTIONS},
												  Internal) {}
		void handleRequest(Poco::Net::HTTPServerRequest &Request,
						 Poco::Net::HTTPServerResponse &Response) override final;
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/ws"};}
		void DoGet(Poco::Net::HTTPServerRequest &Request,
				   Poco::Net::HTTPServerResponse &Response);
	  private:
		void Process(const Poco::JSON::Object::Ptr &O, std::string &Answer);
	};
}

#endif // UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H
