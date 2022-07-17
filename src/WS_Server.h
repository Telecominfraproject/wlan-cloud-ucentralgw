//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <mutex>
#include <thread>
#include <array>
#include <ctime>

#include "framework/MicroService.h"

#include "Poco/AutoPtr.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/ParallelSocketAcceptor.h"
#include "Poco/Net/SocketAcceptor.h"

#include "WS_Connection.h"
#include "WS_ReactorPool.h"

namespace OpenWifi {

	class APWebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		explicit APWebSocketRequestHandler(Poco::Logger &L, Poco::Net::SocketReactor &R)
			:
			  Logger_(L),
			  Reactor_(R) {

			  };

		inline void handleRequest(Poco::Net::HTTPServerRequest &request,
					  Poco::Net::HTTPServerResponse &response)  override {
			try {
				new WSConnection(request, response, Logger_, Reactor_);
			} catch (...) {
				Logger_.warning("Exception during WS creation");
			}
		}
	  private:
		Poco::Logger 				&Logger_;
		Poco::Net::SocketReactor	&Reactor_;
	};

	class APWebSocketRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		explicit APWebSocketRequestHandlerFactory(Poco::Logger &L, Poco::Net::SocketReactor &R)
			: Logger_(L),
			  Reactor_(R) {
		}

		Poco::Net::HTTPRequestHandler * createRequestHandler([[maybe_unused]] const Poco::Net::HTTPServerRequest &request) override {
			return new APWebSocketRequestHandler(Logger_,Reactor_);
		}

	  private:
		Poco::Logger 				&Logger_;
		Poco::Net::SocketReactor	&Reactor_;
	};

	class WebSocketServer : public SubSystemServer {
	  public:
		static auto instance() {
		    static auto instance_ = new WebSocketServer;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		bool IsCertOk() { return IssuerCert_!= nullptr; }
		bool ValidateCertificate(const std::string & ConnectionId, const Poco::Crypto::X509Certificate & Certificate);
		// Poco::Net::SocketReactor & GetNextReactor() { return ReactorPool_.NextReactor(); }

		inline bool IsSimSerialNumber(const std::string & SerialNumber) const {
			return IsSim(SerialNumber) && SerialNumber == SimulatorId_;
		}

		inline static bool IsSim(const std::string & SerialNumber) {
			return SerialNumber.substr(0,6) == "53494d";
		}

		inline bool IsSimEnabled() const {
			return SimulatorEnabled_;
		}

		inline bool UseProvisioning() const { return LookAtProvisioning_; }
		inline bool UseDefaults() const { return UseDefaultConfig_; }

	  private:
		std::unique_ptr<Poco::Crypto::X509Certificate>	IssuerCert_;
		// typedef std::unique_ptr<Poco::Net::ParallelSocketAcceptor<WSConnection, Poco::Net::SocketReactor>> ws_server_reactor_type_t;
//		typedef Poco::Net::SocketAcceptor<WSConnection> ws_server_reactor_type_t;
//		std::vector<std::unique_ptr<ws_server_reactor_type_t>>	Acceptors_;

		Poco::Net::SocketReactor				Reactor_;
		Poco::Thread							ReactorThread_;
		std::string 							SimulatorId_;
		bool 									LookAtProvisioning_ = false;
		bool 									UseDefaultConfig_ = true;
		bool 									SimulatorEnabled_=false;
		std::unique_ptr<Poco::Net::HTTPServer>	ConnectionServer_;

		WebSocketServer() noexcept:
		    SubSystemServer("WebSocketServer", "WS-SVR", "ucentral.websocket") {

		}
	};

	inline auto WebSocketServer() { return WebSocketServer::instance(); }

} //namespace
