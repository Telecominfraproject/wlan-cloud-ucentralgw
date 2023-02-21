//
// Created by stephane bourque on 2022-05-18.
//

#pragma once

#include "RESTObjects/RESTAPI_GWobjects.h"

#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketReactor.h"

#include "framework/SubSystemServer.h"

#include "RADSEC_server.h"

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

		void OnAccountingSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void
		OnAuthenticationSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void OnCoASocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);

		void SendAccountingData(const std::string &serialNumber, const char *buffer,
								std::size_t size);
		void SendAuthenticationData(const std::string &serialNumber, const char *buffer,
									std::size_t size);
		void SendCoAData(const std::string &serialNumber, const char *buffer, std::size_t size);

		void SetConfig(const GWObjects::RadiusProxyPoolList &C);
		void DeleteConfig();
		void GetConfig(GWObjects::RadiusProxyPoolList &C);

		void StartRADSECServers();
		void StopRADSECServers();

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
		};

		inline bool Continue() const { return Running_ && Enabled_ && !Pools_.empty(); }

	  private:
		std::unique_ptr<Poco::Net::DatagramSocket> AccountingSocketV4_;
		std::unique_ptr<Poco::Net::DatagramSocket> AccountingSocketV6_;
		std::unique_ptr<Poco::Net::DatagramSocket> AuthenticationSocketV4_;
		std::unique_ptr<Poco::Net::DatagramSocket> AuthenticationSocketV6_;
		std::unique_ptr<Poco::Net::DatagramSocket> CoASocketV4_;
		std::unique_ptr<Poco::Net::DatagramSocket> CoASocketV6_;
		std::unique_ptr<Poco::Net::SocketReactor> RadiusReactor_;
		Poco::Thread RadiusReactorThread_;

		GWObjects::RadiusProxyPoolList PoolList_;
		std::string ConfigFilename_;

		std::map<Poco::Net::SocketAddress, std::unique_ptr<RADSEC_server>> RADSECservers_;

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
		Poco::Net::SocketAddress Route(radius_type rtype, const Poco::Net::SocketAddress &A,
									   const RADIUS::RadiusPacket &P, bool &UseRADSEC);
		void ParseServerList(const GWObjects::RadiusProxyServerConfig &Config,
							 std::vector<Destination> &V4, std::vector<Destination> &V6,
							 bool setAsDefault);
		static Poco::Net::SocketAddress
		ChooseAddress(std::vector<Destination> &Pool,
					  const Poco::Net::SocketAddress &OriginalAddress);
		Poco::Net::SocketAddress DefaultRoute([[maybe_unused]] radius_type rtype,
											  const Poco::Net::SocketAddress &RequestedAddress,
											  const RADIUS::RadiusPacket &P, bool &UseRADSEC);
	};

	inline auto RADIUS_proxy_server() { return RADIUS_proxy_server::instance(); }

} // namespace OpenWifi
