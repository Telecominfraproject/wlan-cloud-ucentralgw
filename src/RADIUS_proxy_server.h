//
// Created by stephane bourque on 2022-05-18.
//

#pragma once

#include "RESTObjects/RESTAPI_GWobjects.h"

#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketReactor.h"

#include "framework/SubSystemServer.h"

#include "RADIUS_Destination.h"

namespace OpenWifi {

	enum class radius_type { auth, acct, coa };

	class RADIUS_proxy_server : public SubSystemServer {
	  public:
		inline static auto instance() {
			static auto instance_ = new RADIUS_proxy_server;
			return instance_;
		}

		int Start() final;
		void Stop() final;
		inline bool Enabled() const { return Enabled_; }

		void SendAccountingData(const std::string &Destination,const std::string &serialNumber, const char *buffer,
								std::size_t size, std::string & secret);
		void SendAuthenticationData(const std::string &serialNumber, const char *buffer,
									std::size_t size, std::string & secret);
		void SendCoAData(const std::string &serialNumber, const char *buffer, std::size_t size, std::string & secret);

		void RouteAndSendAccountingPacket(const std::string &Destination, const std::string &serialNumber, RADIUS::RadiusPacket &P, bool reComputeAuthenticator, std::string & secret);

		void SetConfig(const GWObjects::RadiusProxyPoolList &C);
		void DeleteConfig();
		void GetConfig(GWObjects::RadiusProxyPoolList &C);

		void StartRADIUSDestinations();
		void StopRADIUSDestinations();

		struct Destination {
			Poco::Net::SocketAddress Addr;
			uint64_t state = 0;
			uint64_t step = 0;
			uint64_t weight = 0;
			bool available = true;
			std::string strategy;
			bool monitor = false;
			std::string monitorMethod;
			std::vector<std::string> methodParameters;
			bool useAsDefault = false;
			bool useRADSEC = false;
			std::vector<std::string> realms;
			std::string secret;
			std::string poolProxyIp;
		};

		inline bool Continue() const { return Running_ && Enabled_ && !Pools_.empty(); }

	  private:
		Poco::Net::SocketReactor 	RadiusReactor_;
		Poco::Thread 				RadiusReactorThread_;

		GWObjects::RadiusProxyPoolList PoolList_;
		std::string ConfigFilename_;

		std::map<std::uint32_t, std::unique_ptr<RADIUS_Destination>> RADIUS_Destinations_;

		struct RadiusPool {
			std::vector<Destination> AuthV4;
			std::vector<Destination> AuthV6;
			std::vector<Destination> AcctV4;
			std::vector<Destination> AcctV6;
			std::vector<Destination> CoaV4;
			std::vector<Destination> CoaV6;
		};

		std::vector<RadiusPool> Pools_;
		uint DefaultPoolIndex_ = 0;
		bool Enabled_ = false;
		bool DefaultIsRADSEC_ = false;
		std::atomic_bool Running_ = false;

		RADIUS_proxy_server() noexcept
			: SubSystemServer("RADIUS-PROXY", "RADIUS-PROXY", "radius.proxy") {}

		static bool SendData(Poco::Net::DatagramSocket &Sock, const unsigned char *buf,
							 std::size_t size, const Poco::Net::SocketAddress &S);

		void ParseConfig();
		void ResetConfig();
//		Poco::Net::SocketAddress Route(radius_type rtype, const Poco::Net::SocketAddress &A,
//									   const RADIUS::RadiusPacket &P, bool &UseRADSEC, std::string &secret);

		void ParseServerList(const GWObjects::RadiusProxyServerConfig &Config,
							 std::vector<Destination> &V4,
							 std::vector<Destination> &V6, bool setAsDefault,
							 const std::string &poolProxyIp);
/*		static Poco::Net::SocketAddress
		ChooseAddress(std::vector<Destination> &Pool,
					  const Poco::Net::SocketAddress &OriginalAddress, std::string &Secret);
		Poco::Net::SocketAddress DefaultRoute([[maybe_unused]] radius_type rtype,
											  const Poco::Net::SocketAddress &RequestedAddress,
											  const RADIUS::RadiusPacket &P, bool &UseRADSEC,
											  std::string &Secret);
*/	};

	inline auto RADIUS_proxy_server() { return RADIUS_proxy_server::instance(); }

} // namespace OpenWifi
