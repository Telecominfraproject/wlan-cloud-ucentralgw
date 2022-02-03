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

#include "TelemetryClient.h"
#include "WS_ReactorPool.h"

namespace OpenWifi {

	class TelemetryStream : public SubSystemServer {
	  public:

		struct QueueUpdate {
			std::string SerialNumber;
			std::string Payload;
		};

		static auto instance() {
		    static auto instance_ = new TelemetryStream;
			return instance_;
		}

		int Start() override;
		void Stop() override;

		bool IsValidEndPoint(const std::string &SerialNumber, const std::string & UUID);
		bool CreateEndpoint(const std::string &SerialNumber, std::string &EndPoint, std::string &UUID);
		void UpdateEndPoint(const std::string &SerialNumber, const std::string &PayLoad);
		bool RegisterClient(const std::string &UUID, TelemetryClient *Client);
		void DeRegisterClient(const std::string &UUID);
		Poco::Net::SocketReactor & NextReactor() { return ReactorPool_.NextReactor(); }

		void onMessage(bool& b);

	  private:
		std::atomic_bool 								Running_=false;
		std::map<std::string, TelemetryClient *>		Clients_;			// 	uuid -> client
		std::map<std::string, std::set<std::string>>	SerialNumbers_;		//	serialNumber -> uuid
		ReactorPool										ReactorPool_;
		std::unique_ptr<FIFO<QueueUpdate>>				Messages_=std::make_unique<FIFO<QueueUpdate>>(100);

		TelemetryStream() noexcept:
			SubSystemServer("TelemetryServer", "TELEMETRY-SVR", "openwifi.telemetry") {
		}
	};

	inline auto TelemetryStream() { return TelemetryStream::instance(); }

	}
