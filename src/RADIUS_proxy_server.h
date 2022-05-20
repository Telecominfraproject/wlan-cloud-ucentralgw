//
// Created by stephane bourque on 2022-05-18.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketReactor.h"

namespace OpenWifi {
	class RADIUS_proxy_server : public SubSystemServer {
	  public:
		inline static auto instance() {
			static auto instance_= new RADIUS_proxy_server;
			return instance_;
		}

		int Start() final;
		void Stop() final;

		void OnAccountingSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void OnAuthenticationSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void SendAccountingData(const std::string &serialNumber, const std::string &Destination,const char *buffer, std::size_t size);
		void SendAuthenticationData(const std::string &serialNumber, const std::string &Destination,const char *buffer, std::size_t size);

	  private:
		std::unique_ptr<Poco::Net::DatagramSocket>	AccountingSocket_;
		std::unique_ptr<Poco::Net::DatagramSocket>	AuthenticationSocket_;
		Poco::Net::SocketReactor	AccountingReactor_;
		Poco::Net::SocketReactor	AuthenticationReactor_;
		Poco::Thread				AuthenticationReactorThread_;
		Poco::Thread				AccountingReactorThread_;

		RADIUS_proxy_server() noexcept:
		   SubSystemServer("RADIUS-PROXY", "RADIUS-PROXY", "radius.proxy")
		{
		}
	};

	inline auto RADIUS_proxy_server() { return RADIUS_proxy_server::instance(); }

}

