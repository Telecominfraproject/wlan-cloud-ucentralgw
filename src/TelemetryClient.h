//
// Created by stephane bourque on 2022-02-03.
//

#pragma once

#include <string>
#include <mutex>

#include "Poco/AutoPtr.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Logger.h"

namespace OpenWifi {
	class TelemetryClient {
		static constexpr int BufSize = 64000;
	  public:
		TelemetryClient(
			std::string UUID,
			uint64_t SerialNumber,
			std::unique_ptr<Poco::Net::WebSocket> WSock,
			Poco::Net::SocketReactor& Reactor,
			Poco::Logger &Logger);
		~TelemetryClient();

		void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
		void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);
		bool Send(const std::string &Payload);
		void ProcessIncomingFrame();
		inline Poco::Logger & Logger() { return Logger_; }

	  private:
		std::recursive_mutex        			Mutex_;
		std::string 							UUID_;
		uint64_t 								SerialNumber_;
		Poco::Net::SocketReactor				&Reactor_;
		Poco::Logger               				&Logger_;
		Poco::Net::StreamSocket     			Socket_;
		std::string 							CId_;
		std::unique_ptr<Poco::Net::WebSocket>	WS_;
		bool 									Registered_=false;
		void SendTelemetryShutdown();
		void CompleteStartup();
		void DeRegister();
	};
}