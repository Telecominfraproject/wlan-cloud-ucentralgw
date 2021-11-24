//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include "framework/MicroService.h"

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"

namespace OpenWifi {
	class RTTY_Client_WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		explicit RTTY_Client_WebSocketRequestHandler(Poco::Net::SocketReactor &R);

		void handleRequest(Poco::Net::HTTPServerRequest &request,
						   Poco::Net::HTTPServerResponse &response) override;

	  private:
		Poco::Net::SocketReactor &R_;
	};

	class PageRequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		void handleRequest(Poco::Net::HTTPServerRequest &request,
						   Poco::Net::HTTPServerResponse &response) override;
	};

	class RTTY_Client_RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		explicit RTTY_Client_RequestHandlerFactory(Poco::Net::SocketReactor &R);

		Poco::Net::HTTPRequestHandler *
		createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

	  private:
		Poco::Net::SocketReactor &Reactor_;
	};
}