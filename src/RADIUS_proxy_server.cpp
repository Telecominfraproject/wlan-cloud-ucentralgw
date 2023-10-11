//
// Created by stephane bourque on 2022-05-18.
//

#include "Poco/JSON/Parser.h"

#include "AP_WS_Server.h"
#include "RADIUS_helpers.h"
#include "RADIUS_proxy_server.h"

#include "RADIUSSessionTracker.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {

/*
 	const int SMALLEST_RADIUS_PACKET = 20 + 19 + 4;
	const int DEFAULT_RADIUS_AUTHENTICATION_PORT = 1812;
	const int DEFAULT_RADIUS_ACCOUNTING_PORT = 1813;
	const int DEFAULT_RADIUS_CoA_PORT = 3799;
*/

	int RADIUS_proxy_server::Start() {

		ConfigFilename_ = MicroServiceDataDirectory() + "/radius_pool_config.json";
		Poco::File Config(ConfigFilename_);

		Enabled_ = MicroServiceConfigGetBool("radius.proxy.enable", false);
		if (!Enabled_ && !Config.exists()) {
			StopRADIUSDestinations();
			return 0;
		}

		poco_notice(Logger(), "Starting...");

		Enabled_ = true;

		ParseConfig();
		StartRADIUSDestinations();
		RadiusReactorThread_.start(*RadiusReactor_);
		Utils::SetThreadName(RadiusReactorThread_, "rad:reactor");
		Running_ = true;

		return 0;
	}

	void RADIUS_proxy_server::Stop() {
		if (Enabled_ && Running_) {
			poco_information(Logger(), "Stopping...");

			StopRADIUSDestinations();
			RadiusReactor_->stop();
			RadiusReactorThread_.join();
			Running_ = false;
			poco_information(Logger(), "Stopped...");
		}
	}

/*	inline static bool isRadsec(const GWObjects::RadiusProxyPool &Cfg) {
		return Cfg.radsecPoolType=="orion" || Cfg.radsecPoolType=="globalreach" || Cfg.radsecPoolType=="radsec";
	}
 */

	void RADIUS_proxy_server::StartRADIUSDestinations() {
		std::lock_guard G(Mutex_);
		for (const auto &pool : PoolList_.pools) {
			if(pool.enabled) {
				for (const auto &entry : pool.authConfig.servers) {
DBGLINE					RADIUS_Destinations_[Poco::Net::SocketAddress(entry.ip, 0)] =
						std::make_unique<RADIUS_Destination>(*RadiusReactor_, pool);
DBGLINE
				}
			} else {
				poco_information(Logger(),fmt::format("Pool {} is not enabled.", pool.name));
			}
		}
	}

	void RADIUS_proxy_server::StopRADIUSDestinations() {
		std::lock_guard G(Mutex_);
		RADIUS_Destinations_.clear();
	}

	void RADIUS_proxy_server::RouteAndSendAccountingPacket(const std::string &Destination, const std::string &serialNumber, RADIUS::RadiusPacket &P, bool RecomputeAuthenticator, std::string & secret) {
		try{

			//	are we sending this to a pool?
			auto CallingStationID = P.ExtractCallingStationID();
			auto CalledStationID = P.ExtractCalledStationID();
			Poco::Net::SocketAddress Dst(Destination);

			std::lock_guard G(Mutex_);

			auto DestinationServer = RADIUS_Destinations_.find(Dst);
			if (DestinationServer != end(RADIUS_Destinations_)) {
				poco_trace(Logger(),fmt::format("{}: Sending Acct {} bytes to {}", serialNumber, P.Size(), Destination));
				if(DestinationServer->second->ServerType()!=GWObjects::RadiusEndpointType::generic) {
					if(RecomputeAuthenticator)
						P.RecomputeAuthenticator(secret);
					DestinationServer->second->SendData(serialNumber, (const unsigned char *)P.Buffer(),
														P.Size());
				} else {
					DestinationServer->second->SendRadiusDataAcctData(
						serialNumber, (const unsigned char *)P.Buffer(), P.Size());
				}
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			poco_warning(Logger(),
						 fmt::format("Bad RADIUS ACCT Packet from {}. Dropped.", serialNumber));
		}
	}

	void store_packet(const std::string &serialNumber, const char *buffer, std::size_t size) {
		static std::uint64_t pkt=0;

		std::string filename = MicroServiceDataDirectory() + "/radius." + serialNumber + "." + std::to_string(pkt++) + ".bin";

		std::ofstream ofs(filename,std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);
		ofs.write(buffer,size);
		ofs.close();
	}

	void RADIUS_proxy_server::SendAccountingData(const std::string &serialNumber,
												 const char *buffer, std::size_t size, std::string & secret) {

		if (!Continue())
			return;

		try {
			RADIUS::RadiusPacket P((unsigned char *)buffer, size);
			auto Destination = P.ExtractProxyStateDestination();
			RouteAndSendAccountingPacket(Destination, serialNumber, P, false, secret);
			RADIUSSessionTracker()->AddAccountingSession(Destination, serialNumber, P, secret);

		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			poco_warning(Logger(),
						 fmt::format("Bad RADIUS ACCT Packet from {}. Dropped.", serialNumber));
		}
	}

	bool RADIUS_proxy_server::SendData(Poco::Net::DatagramSocket &Sock, const unsigned char *buf,
									   std::size_t size, const Poco::Net::SocketAddress &S) {
		return Sock.sendTo(buf, size, S) == (int)size;
	}

	void RADIUS_proxy_server::SendAuthenticationData(const std::string &serialNumber,
													 const char *buffer, std::size_t size, std::string & secret) {

		if (!Continue())
			return;

		try {
			RADIUS::RadiusPacket P((unsigned char *)buffer, size);
			auto Destination = P.ExtractProxyStateDestination();
			auto CallingStationID = P.ExtractCallingStationID();
			auto CalledStationID = P.ExtractCalledStationID();
			Poco::Net::SocketAddress Dst(Destination);

			std::lock_guard G(Mutex_);

			auto DestinationServer = RADIUS_Destinations_.find(Dst);
			if (DestinationServer != end(RADIUS_Destinations_)) {
				poco_trace(Logger(),fmt::format("{}: Sending Auth {} bytes to {}", serialNumber, P.Size(), Destination));
				if(DestinationServer->second->ServerType()!=GWObjects::RadiusEndpointType::generic) {
					P.RecomputeAuthenticator(secret);
					DestinationServer->second->SendData(serialNumber, (const unsigned char *)buffer,
														size);
				} else {
					DestinationServer->second->SendRadiusDataAuthData(
						serialNumber, (const unsigned char *)buffer, size);
				}
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			poco_warning(Logger(),
						 fmt::format("Bad RADIUS AUTH Packet from {}. Dropped.", serialNumber));
		}
	}

	void RADIUS_proxy_server::SendCoAData(const std::string &serialNumber, const char *buffer,
										  std::size_t size, std::string & secret) {

		if (!Continue())
			return;

		try {
			RADIUS::RadiusPacket P((unsigned char *)buffer, size);
			auto Destination = P.ExtractProxyStateDestination();
			auto CallingStationID = P.ExtractCallingStationID();
			auto CalledStationID = P.ExtractCalledStationID();
			Poco::Net::SocketAddress Dst(Destination);

			std::lock_guard G(Mutex_);

			auto DestinationServer = RADIUS_Destinations_.find(Dst);
			if (DestinationServer != end(RADIUS_Destinations_)) {
				poco_trace(Logger(),fmt::format("{}: Sending CoA {} bytes to {}", serialNumber, P.Size(), Destination));
				if(DestinationServer->second->ServerType()!=GWObjects::RadiusEndpointType::generic) {
					P.RecomputeAuthenticator(secret);
					DestinationServer->second->SendData(serialNumber, (const unsigned char *)buffer,
														size);
				} else {
					DestinationServer->second->SendRadiusDataCoAData(
						serialNumber, (const unsigned char *)buffer, size);
				}
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			poco_warning(Logger(),
						 fmt::format("Bad RADIUS AUTH Packet from {}. Dropped.", serialNumber));
		}
	}

	void RADIUS_proxy_server::ParseServerList(const GWObjects::RadiusProxyServerConfig &Config,
											  std::vector<Destination> &V4,
											  std::vector<Destination> &V6, bool setAsDefault,
											  const std::string &poolProxyIp) {
		uint64_t TotalV4 = 0, TotalV6 = 0;

		for (const auto &server : Config.servers) {
			Poco::Net::IPAddress a;
			if (!Poco::Net::IPAddress::tryParse(server.ip, a)) {
				poco_error(Logger(), fmt::format("RADIUS-PARSE Config: server address {} is nto a "
												 "valid address in v4 or v6. Entry skipped.",
												 server.ip));
				continue;
			}
			auto S = Poco::Net::SocketAddress(fmt::format("{}:{}", server.ip, server.port));
			Destination D{.Addr = S,
						  .state = 0,
						  .step = 0,
						  .weight = server.weight,
						  .available = true,
						  .strategy = Config.strategy,
						  .monitor = Config.monitor,
						  .monitorMethod = Config.monitorMethod,
						  .methodParameters = Config.methodParameters,
						  .useAsDefault = setAsDefault,
						  .useRADSEC = server.radsec,
						  .realms = server.radsecRealms,
						  .secret = server.secret,
						  .poolProxyIp = poolProxyIp};

			if (setAsDefault && D.useRADSEC)
				DefaultIsRADSEC_ = true;

			if (S.family() == Poco::Net::IPAddress::IPv4) {
				TotalV4 += server.weight;
				V4.push_back(D);
			} else {
				TotalV6 += server.weight;
				V6.push_back(D);
			}
		}

		for (auto &i : V4) {
			if (TotalV4 == 0) {
				i.step = 1000;
			} else {
				i.step = 1000 - ((1000 * i.weight) / TotalV4);
			}
		}

		for (auto &i : V6) {
			if (TotalV6 == 0) {
				i.step = 1000;
			} else {
				i.step = 1000 - ((1000 * i.weight) / TotalV6);
			}
		}
	}

	void RADIUS_proxy_server::ParseConfig() {

		try {
			Poco::File F(ConfigFilename_);

			std::lock_guard G(Mutex_);

			if (F.exists()) {
				std::ifstream ifs(ConfigFilename_, std::ios_base::binary);
				Poco::JSON::Parser P;
				auto RawConfig = P.parse(ifs).extract<Poco::JSON::Object::Ptr>();
				GWObjects::RadiusProxyPoolList RPC;
				if (RPC.from_json(RawConfig)) {
					ResetConfig();
					PoolList_ = RPC;
					for (const auto &pool : RPC.pools) {
						RadiusPool NewPool;
						ParseServerList(pool.authConfig, NewPool.AuthV4, NewPool.AuthV6,
										pool.useByDefault, pool.poolProxyIp);
						ParseServerList(pool.acctConfig, NewPool.AcctV4, NewPool.AcctV6,
										pool.useByDefault, pool.poolProxyIp);
						ParseServerList(pool.coaConfig, NewPool.CoaV4, NewPool.CoaV6,
										pool.useByDefault, pool.poolProxyIp);
						Pools_.push_back(NewPool);
					}
				} else {
					poco_warning(Logger(),
								 fmt::format("Configuration file '{}' is bad.", ConfigFilename_));
				}
			} else {
				poco_warning(Logger(),
							 fmt::format("No configuration file '{}' exists.", ConfigFilename_));
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			poco_error(Logger(),
					   fmt::format("Error while parsing configuration file '{}'", ConfigFilename_));
		}
	}

/*
	static bool RealmMatch(const std::string &user_realm, const std::string &realm) {
		if (realm.find_first_of('*') == std::string::npos)
			return user_realm == realm;
		return realm.find(user_realm) != std::string::npos;
	}

	Poco::Net::SocketAddress
	RADIUS_proxy_server::DefaultRoute(radius_type rtype,
									  const Poco::Net::SocketAddress &RequestedAddress,
									  const RADIUS::RadiusPacket &P, bool &UseRADSEC,
									  std::string &Secret) {

		bool IsV4 = RequestedAddress.family() == Poco::Net::SocketAddress::IPv4;

		// find the realm...
		auto UserName = P.UserName();
		if (!UserName.empty()) {
			auto UserTokens = Poco::StringTokenizer(UserName, "@");
			auto UserRealm = ((UserTokens.count() > 1) ? UserTokens[1] : UserName);
			Poco::toLowerInPlace(UserRealm);

			for (const auto &pool : Pools_) {
				for (const auto &server : pool.AuthV4) {
					if (!server.realms.empty()) {
						for (const auto &realm : server.realms) {
							if (RealmMatch(UserRealm, realm)) {
								UseRADSEC = true;
								return server.Addr;
							}
						}
					}
				}
			}
		}

		if (DefaultIsRADSEC_) {
			UseRADSEC = true;
			return (IsV4 ? Pools_[DefaultPoolIndex_].AuthV4[0].Addr
						 : Pools_[DefaultPoolIndex_].AuthV6[0].Addr);
		}

		switch (rtype) {
		case radius_type::auth: {
			return ChooseAddress(IsV4 ? Pools_[DefaultPoolIndex_].AuthV4
									  : Pools_[DefaultPoolIndex_].AuthV6,
								 RequestedAddress, Secret);
		}
		case radius_type::coa: {
			return ChooseAddress(IsV4 ? Pools_[DefaultPoolIndex_].CoaV4
									  : Pools_[DefaultPoolIndex_].CoaV6,
								 RequestedAddress, Secret);
		}
		case radius_type::acct:
		default: {
			return ChooseAddress(IsV4 ? Pools_[DefaultPoolIndex_].AcctV4
									  : Pools_[DefaultPoolIndex_].AcctV6,
								 RequestedAddress, Secret);
		}
		}
	}

	Poco::Net::SocketAddress
	RADIUS_proxy_server::Route([[maybe_unused]] radius_type rtype,
							   const Poco::Net::SocketAddress &RequestedAddress,
							   const RADIUS::RadiusPacket &P, bool &UseRADSEC,
							   std::string &Secret) {

		if (Pools_.empty()) {
			UseRADSEC = false;
			return RequestedAddress;
		}

		bool IsV4 = RequestedAddress.family() == Poco::Net::SocketAddress::IPv4;
		bool useDefault;

		useDefault = IsV4 ? RequestedAddress.host() ==
								Poco::Net::IPAddress::wildcard(Poco::Net::IPAddress::IPv4)
						  : RequestedAddress.host() ==
								Poco::Net::IPAddress::wildcard(Poco::Net::IPAddress::IPv6);

		if (useDefault) {
			return DefaultRoute(rtype, RequestedAddress, P, UseRADSEC, Secret);
		}

		auto isAddressInPool = [&](const std::vector<Destination> &D, bool &UseRADSEC) -> bool {
			for (const auto &entry : D) {
				if (!entry.poolProxyIp.empty() &&
					entry.poolProxyIp == RequestedAddress.host().toString()) {
					UseRADSEC = entry.useRADSEC;
					return true;
				}
				if (entry.Addr.host() == RequestedAddress.host()) {
					UseRADSEC = entry.useRADSEC;
					return true;
				}
			}
			return false;
		};

		for (auto &pool : Pools_) {
			// try and match the pool's address to the destination
			switch (rtype) {
			case radius_type::coa: {
				if (isAddressInPool((IsV4 ? pool.CoaV4 : pool.CoaV6), UseRADSEC)) {
					return ChooseAddress(IsV4 ? pool.CoaV4 : pool.CoaV6, RequestedAddress, Secret);
				}
			} break;
			case radius_type::auth: {
				if (isAddressInPool((IsV4 ? pool.AuthV4 : pool.AuthV6), UseRADSEC)) {
					return ChooseAddress(IsV4 ? pool.AuthV4 : pool.AuthV6, RequestedAddress, Secret);
				}
			} break;
			case radius_type::acct: {
				if (isAddressInPool((IsV4 ? pool.AcctV4 : pool.AcctV6), UseRADSEC)) {
					return ChooseAddress(IsV4 ? pool.AcctV4 : pool.AcctV6, RequestedAddress, Secret);
				}
			} break;
			}
		}

		UseRADSEC = false;
		return RequestedAddress;
	}

	Poco::Net::SocketAddress
	RADIUS_proxy_server::ChooseAddress(std::vector<Destination> &Pool,
									   const Poco::Net::SocketAddress &OriginalAddress,
									   std::string &Secret) {

		if (Pool.size() == 1) {
			Secret = Pool[0].secret;
			auto A = Pool[0].Addr;
			return A;
		}

		if (Pool[0].strategy == "weighted") {
			bool found = false;
			uint64_t cur_state = std::numeric_limits<uint64_t>::max();
			std::size_t pos = 0, index = 0;
			for (auto &i : Pool) {
				if (!i.available) {
					i.state += i.step;
					continue;
				}
				if (i.state < cur_state) {
					index = pos;
					cur_state = i.state;
					found = true;
					Secret = i.secret ;
				}
				pos++;
			}

			if (!found) {
				return OriginalAddress;
			}
			Pool[index].state += Pool[index].step;
			return Pool[index].Addr;

		} else if (Pool[0].strategy == "round_robin") {
			bool found = false;
			uint64_t cur_state = std::numeric_limits<uint64_t>::max();
			std::size_t pos = 0, index = 0;
			for (auto &i : Pool) {
				if (!i.available) {
					i.state += 1;
					continue;
				}
				if (i.state < cur_state) {
					index = pos;
					cur_state = i.state;
					Secret = i.secret;
					found = true;
				}
				pos++;
			}

			if (!found) {
//				return OriginalAddress;
			}

			Pool[index].state += 1;
			return Pool[index].Addr;
		} else if (Pool[0].strategy == "random") {
			if (Pool.size() > 1) {
				auto index = std::rand() % Pool.size();
				Secret = Pool[index].secret;
				return Pool[index].Addr;
			} else {
				return OriginalAddress;
			}
		}
		return OriginalAddress;
	}
*/
	void RADIUS_proxy_server::SetConfig(const GWObjects::RadiusProxyPoolList &C) {
		std::lock_guard G(Mutex_);

		Poco::JSON::Object Disk;
		C.to_json(Disk);

		std::ofstream ofs(ConfigFilename_, std::ios_base::trunc | std::ios_base::binary);
		Disk.stringify(ofs);
		ofs.close();

		Stop();
		ResetConfig();
		PoolList_ = C;
		Start();
	}

	void RADIUS_proxy_server::ResetConfig() {
		PoolList_.pools.clear();
		Pools_.clear();
		DefaultPoolIndex_ = 0;
		DefaultIsRADSEC_ = false;
	}

	void RADIUS_proxy_server::DeleteConfig() {
		std::lock_guard G(Mutex_);

		try {
			Poco::File F(ConfigFilename_);
			if (F.exists())
				F.remove();
		} catch (...) {
		}
		Stop();
		ResetConfig();
	}

	void RADIUS_proxy_server::GetConfig(GWObjects::RadiusProxyPoolList &C) {
		std::lock_guard G(Mutex_);
		C = PoolList_;
	}

} // namespace OpenWifi