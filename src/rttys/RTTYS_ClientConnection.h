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
							   std::string &Id);
		~RTTYS_ClientConnection();
		void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);

		void SendData( const u_char *Buf, size_t len );
		void SendData( const std::string & S, bool login=false);
		void CompleteStartup();

		[[nodiscard]] inline std::string ID() { return Id_; }
		[[nodiscard]] inline auto Valid() {	return Valid_;	}
		using MyMutexType = std::mutex;
		using Guard = std::lock_guard<MyMutexType>;

		void EndConnection(bool external, Guard &G);
		inline void EndConnection(bool external) {
			Guard G(Mutex_);
			EndConnection(external, G);
		}

	  private:
		Poco::Net::WebSocket		*WS_= nullptr;
		std::string 				Id_;
		Poco::Logger 				&Logger_;
		std::string 				Sid_;
		mutable bool 				Connected_=false;
		mutable bool 				Valid_=false;
		u_char 						Buffer_[16000]{0};
		MyMutexType					Mutex_;

		mutable std::atomic_bool 	aborting_connection_ = false;
		mutable std::atomic_bool	completing_connection_ = false;
	};
}