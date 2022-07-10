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
		RTTYS_ClientConnection(Poco::Net::WebSocket *WS,
							   Poco::Logger &L,
							   std::string &Id);
		~RTTYS_ClientConnection();
		void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);

		void SendData( const u_char *Buf, size_t len );
		void SendData( const std::string & S, bool login=false);
		void CompleteLogin();

		[[nodiscard]] inline std::string ID() { return Id_; }
		[[nodiscard]] inline auto Valid() {
			std::lock_guard	G(Mutex_);
			return Valid_;
		}
		using MyMutexType = std::mutex;
		using Guard = std::lock_guard<MyMutexType>;

		void EndConnection(Guard &G);
		inline void EndConnection() {
			Guard G(Mutex_);
			EndConnection(G);
		}

	  private:
		Poco::Net::WebSocket		*WS_= nullptr;
		Poco::Logger & 				Logger_;
		std::string 				Id_;
		std::string 				Sid_;
		mutable bool 				Connected_=false;
		mutable bool 				Valid_=false;
		u_char 						Buffer_[16000]{0};
		MyMutexType					Mutex_;

		mutable std::atomic_bool 	aborting_connection_ = false;
		mutable std::atomic_bool	completing_connection_ = false;

		inline Poco::Logger & Logger() { return Logger_; }
	};
}