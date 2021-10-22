//
// Created by stephane bourque on 2021-08-12.
//

#ifndef UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H
#define UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H

#include "framework/MicroService.h"

namespace OpenWifi {
	class RESTAPI_webSocketServer : public RESTAPIHandler {
	  public:
		RESTAPI_webSocketServer(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer &Server, bool Internal)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{	Poco::Net::HTTPRequest::HTTP_GET,
												  	Poco::Net::HTTPRequest::HTTP_OPTIONS},
													Server, Internal,false) {}
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/ws"};}
		void DoGet() final;
		void DoDelete() final {};
		void DoPost() final {};
		void DoPut() final {};
	  private:
		void Process(const Poco::JSON::Object::Ptr &O, std::string &Answer);
	};
}

#endif // UCENTRALGW_RESTAPI_WEBSOCKETSERVER_H
