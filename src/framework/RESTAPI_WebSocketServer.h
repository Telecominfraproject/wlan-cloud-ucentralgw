//
// Created by stephane bourque on 2022-10-26.
//

#pragma once

#include "framework/RESTAPI_Handler.h"
#include "Poco/Net/WebSocket.h"

#include "framework/UI_WebSocketClientServer.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {
	class RESTAPI_webSocketServer : public RESTAPIHandler {
	  public:
		inline RESTAPI_webSocketServer(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServerAccounting &Server, uint64_t TransactionId, bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{	Poco::Net::HTTPRequest::HTTP_GET,
													  	Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server, TransactionId, Internal,false) {}
		static auto PathName() { return std::list<std::string>{"/api/v1/ws"};}
		void DoGet() final {
				try
				{
					if(Request->find("Upgrade") != Request->end() && Poco::icompare((*Request)["Upgrade"], "websocket") == 0) {
						try
						{
							Poco::Net::WebSocket WS(*Request, *Response);
							auto Id = MicroServiceCreateUUID();
							UI_WebSocketClientServer()->NewClient(WS,Id,UserInfo_.userinfo.email, TransactionId_);
						}
						catch (...) {
							std::cout << "Cannot create websocket client..." << std::endl;
						}
					}
				} catch(...) {
					std::cout << "Cannot upgrade connection..." << std::endl;
				}
			};
		void DoDelete() final {};
		void DoPost() final {};
		void DoPut() final {};
	  private:
	};
}