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

	enum class connection_state {
		initialized, waiting_for_login, connected, aborting, shutting_down, done
	};

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

		[[nodiscard]] inline std::string ID() { return Id_; }
		[[nodiscard]] inline bool Valid() volatile const { return Valid_; }
		using MyMutexType = std::recursive_mutex;
		using MyGuard = std::lock_guard<MyMutexType>;

	  private:
		Poco::Net::SocketReactor 				&Reactor_;
		std::string 							Id_;
		std::unique_ptr<Poco::Net::WebSocket>	WS_;
		Poco::Logger 							&Logger_;
		std::string 							Sid_;
		volatile bool 							Valid_=false;
		u_char 									Buffer_[64000]{0};
		MyMutexType								Mutex_;
		// volatile  connection_state	state_ = connection_state::initialized;

		void EndConnection(bool SendNotification=true);

	};
}