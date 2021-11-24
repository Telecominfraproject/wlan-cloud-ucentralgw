//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/NObserver.h"
#include "Poco/Net/SocketNotification.h"

namespace OpenWifi {
	class RTTY_ClientConnection {
	  public:
		RTTY_ClientConnection(Poco::Net::WebSocket &WS, std::string &Id,
							  Poco::Net::SocketReactor &Reactor);
		~RTTY_ClientConnection();
		void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);

	  private:
		Poco::Net::WebSocket WS_;
		std::string Id_;
		Poco::Net::SocketReactor &SR_;
	};
}