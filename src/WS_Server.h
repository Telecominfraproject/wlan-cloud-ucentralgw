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

#include "WS_Connection.h"
#include "WS_ReactorPool.h"

namespace OpenWifi {

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
		Poco::Net::SocketReactor & GetNextReactor() { return ReactorPool_.NextReactor(); }

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
		std::vector<std::unique_ptr<Poco::Net::ParallelSocketAcceptor<WSConnection, Poco::Net::SocketReactor>>>	Acceptors_;
		Poco::Net::SocketReactor		Reactor_;
		Poco::Thread					ReactorThread_;
		ReactorPool						ReactorPool_;
		std::string 					SimulatorId_;
		bool 							LookAtProvisioning_ = false;
		bool 							UseDefaultConfig_ = true;
		bool 							SimulatorEnabled_=false;

		WebSocketServer() noexcept:
		    SubSystemServer("WebSocketServer", "WS-SVR", "ucentral.websocket") {

		}
	};

	inline auto WebSocketServer() { return WebSocketServer::instance(); }

} //namespace
