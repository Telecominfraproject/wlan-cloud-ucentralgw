//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include "framework/MicroService.h"

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"

namespace OpenWifi {
	class RTTYS_Client_WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		explicit RTTYS_Client_WebSocketRequestHandler(Poco::Logger &L);

		void handleRequest(Poco::Net::HTTPServerRequest &request,
						   Poco::Net::HTTPServerResponse &response) override;

	  private:
		Poco::Logger 				&Logger_;
	};

	class PageRequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		PageRequestHandler(Poco::Logger &L)
			: Logger_(L) {
		}

		void handleRequest(Poco::Net::HTTPServerRequest &request,
						   Poco::Net::HTTPServerResponse &response) override;

	  private:
		Poco::Logger & Logger_;
		inline Poco::Logger &Logger() { return Logger_; }
	};

	class RTTYS_Client_RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		explicit RTTYS_Client_RequestHandlerFactory(Poco::Logger &L);

		Poco::Net::HTTPRequestHandler *
		createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;

	  private:
		Poco::Logger 				&Logger_;
	};
}