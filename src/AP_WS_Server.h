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

#include "AP_WS_Connection.h"
#include "WS_ReactorPool.h"

namespace OpenWifi {

	class AP_WS_RequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		explicit AP_WS_RequestHandler(Poco::Logger &L)
			: Logger_(L) {
		};

		void handleRequest(Poco::Net::HTTPServerRequest &request,
						   Poco::Net::HTTPServerResponse &response) override;
	  private:
		Poco::Logger 				&Logger_;
	};

	class AP_WS_RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		inline explicit AP_WS_RequestHandlerFactory(Poco::Logger &L)
		: Logger_(L) {
		}

		inline Poco::Net::HTTPRequestHandler *
		createRequestHandler(const Poco::Net::HTTPServerRequest &request) override {
			if (request.find("Upgrade") != request.end() &&
				Poco::icompare(request["Upgrade"], "websocket") == 0) {
				Poco::Thread::current()->setName("AP_CONN_INIT");
				return new AP_WS_RequestHandler(Logger_);
			} else {
				return nullptr;
			}
		}
	  private:
		Poco::Logger 				&Logger_;
	};

	class AP_WS_Server : public SubSystemServer {
	  public:
		static auto instance() {
			static auto instance_ = new AP_WS_Server;
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

		[[nodiscard]] inline Poco::Net::SocketReactor & NextReactor() { return Reactor_; }

	  private:
		std::unique_ptr<Poco::Crypto::X509Certificate>		IssuerCert_;
		std::list<std::unique_ptr<Poco::Net::HTTPServer>>	WebServers_;
		Poco::Net::SocketReactor							Reactor_;
		Poco::Thread										ReactorThread_;
		std::string 										SimulatorId_;
		Poco::ThreadPool									DeviceConnectionPool_{"dev-pool", 2, 32};
		bool 												LookAtProvisioning_ = false;
		bool 												UseDefaultConfig_ = true;
		bool 												SimulatorEnabled_=false;

		AP_WS_Server() noexcept:
			SubSystemServer("WebSocketServer", "WS-SVR", "ucentral.websocket") {
		}
	};

	inline auto AP_WS_Server() { return AP_WS_Server::instance(); }

} //namespace