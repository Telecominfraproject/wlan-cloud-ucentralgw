//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/NObserver.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/FIFOBuffer.h"

namespace OpenWifi {
	class RTTYS_ClientConnection {
	  public:
		RTTYS_ClientConnection(std::unique_ptr<Poco::Net::WebSocket> WS, std::string &Id,
							  Poco::Net::SocketReactor &Reactor);
		~RTTYS_ClientConnection();
		void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);

		void SendData( const u_char *Buf, size_t len );
		void SendData( const std::string & S, bool login=false);

		void Close();

	  private:
		std::unique_ptr<Poco::Net::WebSocket> WS_;
		std::string 		Id_;
		std::string 		Sid_;
		Poco::Net::SocketReactor &SR_;
		std::atomic_bool 	Connected_=false;
		Poco::Logger & 		Logger_;
		u_char 				Buffer[16000]{0};

		inline Poco::Logger & Logger() { return Logger_; }
	};
}