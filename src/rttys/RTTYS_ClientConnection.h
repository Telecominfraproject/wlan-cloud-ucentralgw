//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include "Poco/Net/WebSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/NObserver.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/FIFOBuffer.h"
#include "Poco/Logger.h"

#include <mutex>
#include <shared_mutex>

namespace OpenWifi {

	class RTTYS_Device_ConnectionHandler;

	class RTTYS_ClientConnection {
	  public:
		RTTYS_ClientConnection(Poco::Net::HTTPServerRequest &request,
							   Poco::Net::HTTPServerResponse &response,
							   Poco::Net::SocketReactor & reactor,
							   const std::string &Id);
		~RTTYS_ClientConnection();
		void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);

		void SendData( const u_char *Buf, size_t len );
		void SendData( const std::string & S );
		void DeRegister();

		[[nodiscard]] inline std::string ID() { return Id_; }
		[[nodiscard]] inline bool Valid()  { return Valid_; }
		inline void SetDevice(std::shared_ptr<RTTYS_Device_ConnectionHandler> Device) {
			std::lock_guard		G(Mutex_);
			Device_ = std::move(Device);
		}

		void EndConnection();

	  private:
		Poco::Net::SocketReactor 				&Reactor_;
		std::string 							Id_;
		std::unique_ptr<Poco::Net::WebSocket>	WS_;
		Poco::Logger 							&Logger_;
		std::string 							Sid_;
		std::recursive_mutex								Mutex_;
		volatile bool										Valid_=false;
		volatile bool 										Registered_=false;
		std::shared_ptr<RTTYS_Device_ConnectionHandler>		Device_;
	};
}