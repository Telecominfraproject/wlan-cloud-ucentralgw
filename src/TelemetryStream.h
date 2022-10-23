//
// Created by stephane bourque on 2021-09-07.
//

#pragma once

#include <iostream>

#include "framework/MicroService.h"

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/ParallelSocketAcceptor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerRequestImpl.h"
#include "Poco/Timespan.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPServer.h"

#include "AP_WS_ReactorPool.h"
#include "TelemetryClient.h"

namespace OpenWifi {

	class TelemetryNotification : public Poco::Notification {
	  public:
		explicit TelemetryNotification(std::uint64_t SerialNUmber, const std::string &Payload) :
		 	SerialNumber_(SerialNUmber),
			Payload_(Payload) {
		}

		std::uint64_t 	SerialNumber_=0;
		std::string 	Payload_;
	};

	class TelemetryStream : public SubSystemServer, Poco::Runnable {
	  public:

		struct QueueUpdate {
			uint64_t 	SerialNumber;
			std::string Payload;
		};

		static auto instance() {
		    static auto instance_ = new TelemetryStream;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void run() final;

		bool IsValidEndPoint(uint64_t SerialNumber, const std::string & UUID);
		bool CreateEndpoint(uint64_t SerialNumber, std::string &EndPoint, const std::string &UUID);

		inline void NotifyEndPoint(uint64_t SerialNumber, const std::string &PayLoad) {
			MsgQueue_.enqueueNotification(new TelemetryNotification(SerialNumber,PayLoad));
		}

		bool NewClient(const std::string &UUID, uint64_t SerialNumber,  std::unique_ptr<Poco::Net::WebSocket> Client);
		void DeRegisterClient(const std::string &UUID);

		Poco::Net::SocketReactor & NextReactor() { return Reactor_; }

	  private:
		volatile std::atomic_bool 						Running_=false;
		std::map<uint64_t, std::set<std::string>>		SerialNumbers_;		//	serialNumber -> uuid
		Poco::Net::SocketReactor						Reactor_;
		Poco::Thread									ReactorThr_;
		Poco::Thread									NotificationMgr_;
		Poco::NotificationQueue							MsgQueue_;

		std::map<std::string, std::unique_ptr<TelemetryClient> >		Clients_;			// 	uuid -> client

		TelemetryStream() noexcept:
			SubSystemServer("TelemetryServer", "TELEMETRY-SVR", "openwifi.telemetry") {
		}
	};

	inline auto TelemetryStream() { return TelemetryStream::instance(); }

	}
