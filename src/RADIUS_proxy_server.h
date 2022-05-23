//
// Created by stephane bourque on 2022-05-18.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "RESTObjects/RESTAPI_GWobjects.h"

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

		void SetConfig(const GWObjects::RadiusProxyPoolList &C);
		void DeleteConfig();
		void GetConfig(GWObjects::RadiusProxyPoolList &C);

		struct Destination {
			Poco::Net::SocketAddress 	Addr;
			uint64_t 					state = 0;
			uint64_t 					step = 0;
			uint64_t 					weight=0;
			bool 						available = true;
			std::string 				strategy;
			bool 						monitor=false;
			std::string 				monitorMethod;
			std::vector<std::string>	methodParameters;
		};

	  private:
		std::unique_ptr<Poco::Net::DatagramSocket>	AccountingSocketV4_;
		std::unique_ptr<Poco::Net::DatagramSocket>	AccountingSocketV6_;
		std::unique_ptr<Poco::Net::DatagramSocket>	AuthenticationSocketV4_;
		std::unique_ptr<Poco::Net::DatagramSocket>	AuthenticationSocketV6_;
		Poco::Net::SocketReactor		AccountingReactor_;
		Poco::Net::SocketReactor		AuthenticationReactor_;
		Poco::Thread					AuthenticationReactorThread_;
		Poco::Thread					AccountingReactorThread_;
		GWObjects::RadiusProxyPoolList	PoolList_;
		std::string 					ConfigFilename_;

		typedef std::map<Poco::Net::SocketAddress,uint> PoolIndexMap_t;
		PoolIndexMap_t	AuthPoolsIndexV4_;
		PoolIndexMap_t 	AcctPoolsIndexV4_;
		PoolIndexMap_t	AuthPoolsIndexV6_;
		PoolIndexMap_t 	AcctPoolsIndexV6_;

		typedef std::vector<std::vector<Destination>> PoolIndexVec_t;
		PoolIndexVec_t	AuthPoolsV4_;
		PoolIndexVec_t	AuthPoolsV6_;
		PoolIndexVec_t	AcctPoolsV4_;
		PoolIndexVec_t	AcctPoolsV6_;

		RADIUS_proxy_server() noexcept:
		   SubSystemServer("RADIUS-PROXY", "RADIUS-PROXY", "radius.proxy")
		{
		}

		void ParseConfig();
		void ResetConfig();
		Poco::Net::SocketAddress Route(const Poco::Net::SocketAddress &A, PoolIndexVec_t & P, PoolIndexMap_t &M);
		void ParseServerList(const GWObjects::RadiusProxyServerConfig & Config,  PoolIndexMap_t &MapV4, PoolIndexMap_t &MapV6, PoolIndexVec_t &VecV4, PoolIndexVec_t &VecV6);
	};

	inline auto RADIUS_proxy_server() { return RADIUS_proxy_server::instance(); }

}

