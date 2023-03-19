//
// Created by stephane bourque on 2022-05-18.
//

#include "Poco/JSON/Parser.h"

#include "AP_WS_Server.h"
#include "RADIUS_helpers.h"
#include "RADIUS_proxy_server.h"

#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {

	const int SMALLEST_RADIUS_PACKET = 20 + 19 + 4;
	const int DEFAULT_RADIUS_AUTHENTICATION_PORT = 1812;
	const int DEFAULT_RADIUS_ACCOUNTING_PORT = 1813;
	const int DEFAULT_RADIUS_CoA_PORT = 3799;

	int RADIUS_proxy_server::Start() {

		ConfigFilename_ = MicroServiceDataDirectory() + "/radius_pool_config.json";
		Poco::File Config(ConfigFilename_);

		Enabled_ = MicroServiceConfigGetBool("radius.proxy.enable", false);
		if (!Enabled_ && !Config.exists()) {
			StopRADSECServers();
			return 0;
		}

		poco_notice(Logger(), "Starting...");

		Enabled_ = true;

		Poco::Net::SocketAddress AuthSockAddrV4(
			Poco::Net::AddressFamily::IPv4,
			MicroServiceConfigGetInt("radius.proxy.authentication.port",
									 DEFAULT_RADIUS_AUTHENTICATION_PORT));
		AuthenticationSocketV4_ =
			std::make_unique<Poco::Net::DatagramSocket>(AuthSockAddrV4, true, true);
		Poco::Net::SocketAddress AuthSockAddrV6(
			Poco::Net::AddressFamily::IPv6,
			MicroServiceConfigGetInt("radius.proxy.authentication.port",
									 DEFAULT_RADIUS_AUTHENTICATION_PORT));
		AuthenticationSocketV6_ =
			std::make_unique<Poco::Net::DatagramSocket>(AuthSockAddrV6, true, true);

		Poco::Net::SocketAddress AcctSockAddrV4(
			Poco::Net::AddressFamily::IPv4,
			MicroServiceConfigGetInt("radius.proxy.accounting.port",
									 DEFAULT_RADIUS_ACCOUNTING_PORT));
		AccountingSocketV4_ =
			std::make_unique<Poco::Net::DatagramSocket>(AcctSockAddrV4, true, true);
		Poco::Net::SocketAddress AcctSockAddrV6(
			Poco::Net::AddressFamily::IPv6,
			MicroServiceConfigGetInt("radius.proxy.accounting.port",
									 DEFAULT_RADIUS_ACCOUNTING_PORT));
		AccountingSocketV6_ =
			std::make_unique<Poco::Net::DatagramSocket>(AcctSockAddrV6, true, true);

		Poco::Net::SocketAddress CoASockAddrV4(
			Poco::Net::AddressFamily::IPv4,
			MicroServiceConfigGetInt("radius.proxy.coa.port", DEFAULT_RADIUS_CoA_PORT));
		CoASocketV4_ = std::make_unique<Poco::Net::DatagramSocket>(CoASockAddrV4, true, true);
		Poco::Net::SocketAddress CoASockAddrV6(
			Poco::Net::AddressFamily::IPv6,
			MicroServiceConfigGetInt("radius.proxy.coa.port", DEFAULT_RADIUS_CoA_PORT));
		CoASocketV6_ = std::make_unique<Poco::Net::DatagramSocket>(CoASockAddrV6, true, true);

		RadiusReactor_.reset();
		RadiusReactor_ = std::make_unique<Poco::Net::SocketReactor>();
		RadiusReactor_->addEventHandler(
			*AuthenticationSocketV4_,
			Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
				*this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));
		RadiusReactor_->addEventHandler(
			*AuthenticationSocketV6_,
			Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
				*this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));

		RadiusReactor_->addEventHandler(
			*AccountingSocketV4_,
			Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
				*this, &RADIUS_proxy_server::OnAccountingSocketReadable));
		RadiusReactor_->addEventHandler(
			*AccountingSocketV6_,
			Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
				*this, &RADIUS_proxy_server::OnAccountingSocketReadable));

		RadiusReactor_->addEventHandler(
			*CoASocketV4_, Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
							   *this, &RADIUS_proxy_server::OnCoASocketReadable));
		RadiusReactor_->addEventHandler(
			*CoASocketV6_, Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
							   *this, &RADIUS_proxy_server::OnCoASocketReadable));

		ParseConfig();
		StartRADSECServers();
		RadiusReactorThread_.start(*RadiusReactor_);
		Utils::SetThreadName(RadiusReactorThread_, "rad:reactor");
		Running_ = true;

		return 0;
	}

	void RADIUS_proxy_server::Stop() {
		if (Enabled_ && Running_) {
			poco_information(Logger(), "Stopping...");
			RadiusReactor_->removeEventHandler(
				*AuthenticationSocketV4_,
				Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
					*this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));
			RadiusReactor_->removeEventHandler(
				*AuthenticationSocketV6_,
				Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
					*this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));

			RadiusReactor_->removeEventHandler(
				*AccountingSocketV4_,
				Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
					*this, &RADIUS_proxy_server::OnAccountingSocketReadable));
			RadiusReactor_->removeEventHandler(
				*AccountingSocketV6_,
				Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
					*this, &RADIUS_proxy_server::OnAccountingSocketReadable));

			RadiusReactor_->removeEventHandler(
				*CoASocketV4_,
				Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
					*this, &RADIUS_proxy_server::OnCoASocketReadable));
			RadiusReactor_->removeEventHandler(
				*CoASocketV6_,
				Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
					*this, &RADIUS_proxy_server::OnCoASocketReadable));

			AuthenticationSocketV4_->close();
			AuthenticationSocketV6_->close();
			AccountingSocketV4_->close();
			AccountingSocketV6_->close();
			CoASocketV4_->close();
			CoASocketV6_->close();

			AuthenticationSocketV4_.reset();
			AuthenticationSocketV6_.reset();
			AccountingSocketV4_.reset();
			AccountingSocketV6_.reset();
			CoASocketV4_.reset();
			CoASocketV6_.reset();

			StopRADSECServers();
			RadiusReactor_->stop();
			RadiusReactorThread_.join();
			Running_ = false;
			poco_information(Logger(), "Stopped...");
		}
	}

	void RADIUS_proxy_server::StartRADSECServers() {
		std::lock_guard G(Mutex_);
		for (const auto &pool : PoolList_.pools) {
			for (const auto &entry : pool.authConfig.servers) {
				if (entry.radsec) {
					RADSECservers_[Poco::Net::SocketAddress(entry.ip, 0)] =
						std::make_unique<RADSEC_server>(*RadiusReactor_, entry);
				}
			}
		}
	}

	void RADIUS_proxy_server::StopRADSECServers() {
		std::lock_guard G(Mutex_);
		RADSECservers_.clear();
	}

	void RADIUS_proxy_server::OnAccountingSocketReadable(
		const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		Poco::Net::SocketAddress Sender;
		RADIUS::RadiusPacket P;

		auto ReceiveSize = pNf->socket().impl()->receiveBytes(P.Buffer(), P.BufferLen());
		if (ReceiveSize < SMALLEST_RADIUS_PACKET) {
			poco_warning(Logger(), "Accounting: bad packet received.");
			return;
		}
		P.Evaluate(ReceiveSize);
		auto SerialNumber = P.ExtractSerialNumberFromProxyState();
		if (SerialNumber.empty()) {
			poco_warning(Logger(), "Accounting: missing serial number.");
			return;
		}
		auto CallingStationID = P.ExtractCallingStationID();
		auto CalledStationID = P.ExtractCalledStationID();

		poco_debug(
			Logger(),
			fmt::format(
				"Accounting Packet received for {}, CalledStationID: {}, CallingStationID:{}",
				SerialNumber, CalledStationID, CallingStationID));
		AP_WS_Server()->SendRadiusAccountingData(SerialNumber, P.Buffer(), P.Size());
	}

	void RADIUS_proxy_server::OnAuthenticationSocketReadable(
		const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		Poco::Net::SocketAddress Sender;
		RADIUS::RadiusPacket P;

		auto ReceiveSize = pNf->socket().impl()->receiveBytes(P.Buffer(), P.BufferLen());
		if (ReceiveSize < SMALLEST_RADIUS_PACKET) {
			poco_warning(Logger(), "Authentication: bad packet received.");
			return;
		}
		P.Evaluate(ReceiveSize);
		auto SerialNumber = P.ExtractSerialNumberFromProxyState();
		if (SerialNumber.empty()) {
			poco_warning(Logger(), "Authentication: missing serial number.");
			return;
		}
		auto CallingStationID = P.ExtractCallingStationID();
		auto CalledStationID = P.ExtractCalledStationID();

		poco_debug(
			Logger(),
			fmt::format(
				"Authentication Packet received for {}, CalledStationID: {}, CallingStationID:{}",
				SerialNumber, CalledStationID, CallingStationID));
		AP_WS_Server()->SendRadiusAuthenticationData(SerialNumber, P.Buffer(), P.Size());
	}

	void RADIUS_proxy_server::OnCoASocketReadable(
		const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		Poco::Net::SocketAddress Sender;
		RADIUS::RadiusPacket P;

		auto ReceiveSize = pNf.get()->socket().impl()->receiveBytes(P.Buffer(), P.BufferLen());
		if (ReceiveSize < SMALLEST_RADIUS_PACKET) {
			poco_warning(Logger(), "CoA/DM: bad packet received.");
			return;
		}
		P.Evaluate(ReceiveSize);
		auto SerialNumber = P.ExtractSerialNumberTIP();
		if (SerialNumber.empty()) {
			poco_warning(Logger(), "CoA/DM: missing serial number.");
			return;
		}
		auto CallingStationID = P.ExtractCallingStationID();
		auto CalledStationID = P.ExtractCalledStationID();

		poco_debug(
			Logger(),
			fmt::format("CoA Packet received for {}, CalledStationID: {}, CallingStationID:{}",
						SerialNumber, CalledStationID, CallingStationID));
		AP_WS_Server()->SendRadiusCoAData(SerialNumber, P.Buffer(), P.Size());
	}

	void RADIUS_proxy_server::SendAccountingData(const std::string &serialNumber,
												 const char *buffer, std::size_t size) {

		if (!Continue())
			return;

		try {
			RADIUS::RadiusPacket P((unsigned char *)buffer, size);
			auto Destination = P.ExtractProxyStateDestination();
			auto CallingStationID = P.ExtractCallingStationID();
			auto CalledStationID = P.ExtractCalledStationID();
			Poco::Net::SocketAddress Dst(Destination);

			std::cout << "Sending accounting packet to proxy..." << std::endl;
			P.Log(std::cout);

			std::lock_guard G(Mutex_);
			bool UseRADSEC = false;
			auto FinalDestination = Route(radius_type::acct, Dst, P, UseRADSEC);
			if (UseRADSEC) {
				Poco::Net::SocketAddress RSP(FinalDestination.host(), 0);
				auto DestinationServer = RADSECservers_.find(RSP);
				if (DestinationServer != end(RADSECservers_)) {
					DestinationServer->second->SendData(serialNumber, (const unsigned char *)buffer,
														size);
				}
			} else {
				if ((Dst.family() == Poco::Net::SocketAddress::IPv4 &&
					 AccountingSocketV4_ == nullptr) ||
					(Dst.family() == Poco::Net::SocketAddress::IPv6 &&
					 AccountingSocketV6_ == nullptr)) {
					poco_debug(
						Logger(),
						fmt::format(
							"ACCT: Trying to use RADIUS GW PROXY but not configured. Device={}",
							serialNumber));
					return;
				}
				auto AllSent =
					SendData(Dst.family() == Poco::Net::SocketAddress::IPv4 ? *AccountingSocketV4_
																			: *AccountingSocketV6_,
							 (const unsigned char *)buffer, size, FinalDestination);
				if (!AllSent)
					poco_error(Logger(),
							   fmt::format("{}: Could not send Accounting packet packet to {}.",
										   serialNumber, Destination));
				else
					poco_debug(Logger(), fmt::format("{}: Sending Accounting Packet to {}, "
													 "CalledStationID: {}, CallingStationID:{}",
													 serialNumber, FinalDestination.toString(),
													 CalledStationID, CallingStationID));
			}
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
													 const char *buffer, std::size_t size) {

		if (!Continue())
			return;

		try {
			RADIUS::RadiusPacket P((unsigned char *)buffer, size);
			auto Destination = P.ExtractProxyStateDestination();
			auto CallingStationID = P.ExtractCallingStationID();
			auto CalledStationID = P.ExtractCalledStationID();
			Poco::Net::SocketAddress Dst(Destination);

			std::lock_guard G(Mutex_);
			bool UseRADSEC = false;
			auto FinalDestination = Route(radius_type::auth, Dst, P, UseRADSEC);
			if (UseRADSEC) {
				Poco::Net::SocketAddress RSP(FinalDestination.host(), 0);
				auto DestinationServer = RADSECservers_.find(RSP);
				if (DestinationServer != end(RADSECservers_)) {
					DestinationServer->second->SendData(serialNumber, (const unsigned char *)buffer,
														size);
				}
			} else {
				if ((Dst.family() == Poco::Net::SocketAddress::IPv4 &&
					 AuthenticationSocketV4_ == nullptr) ||
					(Dst.family() == Poco::Net::SocketAddress::IPv6 &&
					 AuthenticationSocketV6_ == nullptr)) {
					poco_debug(
						Logger(),
						fmt::format(
							"AUTH: Trying to use RADIUS GW PROXY but not configured. Device={}",
							serialNumber));
					return;
				}
				auto AllSent = SendData(Dst.family() == Poco::Net::SocketAddress::IPv4
											? *AuthenticationSocketV4_
											: *AuthenticationSocketV6_,
										(const unsigned char *)buffer, size, FinalDestination);
				if (!AllSent)
					poco_error(Logger(),
							   fmt::format("{}: Could not send Authentication packet packet to {}.",
										   serialNumber, Destination));
				else
					poco_debug(Logger(), fmt::format("{}: Sending Authentication Packet to {}, "
													 "CalledStationID: {}, CallingStationID:{}",
													 serialNumber, FinalDestination.toString(),
													 CalledStationID, CallingStationID));
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			poco_warning(Logger(),
						 fmt::format("Bad RADIUS AUTH Packet from {}. Dropped.", serialNumber));
		}
	}

	void RADIUS_proxy_server::SendCoAData(const std::string &serialNumber, const char *buffer,
										  std::size_t size) {

		if (!Continue())
			return;

		try {
			RADIUS::RadiusPacket P((unsigned char *)buffer, size);
			auto Destination = P.ExtractProxyStateDestination();

			if (Destination.empty()) {
				Destination = "0.0.0.0:0";
			}

			Poco::Net::SocketAddress Dst(Destination);
			std::lock_guard G(Mutex_);
			bool UseRADSEC = false;
			auto FinalDestination = Route(radius_type::coa, Dst, P, UseRADSEC);
			if (UseRADSEC) {
				Poco::Net::SocketAddress RSP(FinalDestination.host(), 0);
				auto DestinationServer = RADSECservers_.find(RSP);
				if (DestinationServer != end(RADSECservers_)) {
					DestinationServer->second->SendData(serialNumber, (const unsigned char *)buffer,
														size);
				}
			} else {
				if ((Dst.family() == Poco::Net::SocketAddress::IPv4 && CoASocketV4_ == nullptr) ||
					(Dst.family() == Poco::Net::SocketAddress::IPv6 && CoASocketV6_ == nullptr)) {
					poco_debug(
						Logger(),
						fmt::format(
							"CoA: Trying to use RADIUS GW PROXY but not configured. Device={}",
							serialNumber));
					return;
				}
				auto AllSent = SendData(
					Dst.family() == Poco::Net::SocketAddress::IPv4 ? *CoASocketV4_ : *CoASocketV6_,
					(const unsigned char *)buffer, size, FinalDestination);
				if (!AllSent)
					poco_error(Logger(), fmt::format("{}: Could not send CoA packet packet to {}.",
													 serialNumber, Destination));
				else
					poco_debug(Logger(), fmt::format("{}: Sending CoA Packet to {}", serialNumber,
													 FinalDestination.toString()));
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			poco_warning(Logger(),
						 fmt::format("Bad RADIUS CoA/DM Packet from {}. Dropped.", serialNumber));
		}
	}

	void RADIUS_proxy_server::ParseServerList(const GWObjects::RadiusProxyServerConfig &Config,
											  std::vector<Destination> &V4,
											  std::vector<Destination> &V6, bool setAsDefault) {
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
						  .realms = server.radsecRealms};

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
										pool.useByDefault);
						ParseServerList(pool.acctConfig, NewPool.AcctV4, NewPool.AcctV6,
										pool.useByDefault);
						ParseServerList(pool.coaConfig, NewPool.CoaV4, NewPool.CoaV6,
										pool.useByDefault);
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

	static bool RealmMatch(const std::string &user_realm, const std::string &realm) {
		if (realm.find_first_of('*') == std::string::npos)
			return user_realm == realm;
		return realm.find(user_realm) != std::string::npos;
	}

	Poco::Net::SocketAddress
	RADIUS_proxy_server::DefaultRoute(radius_type rtype,
									  const Poco::Net::SocketAddress &RequestedAddress,
									  const RADIUS::RadiusPacket &P, bool &UseRADSEC) {
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
								std::cout << "Realm match..." << std::endl;
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
								 RequestedAddress);
		}
		case radius_type::acct:
		default: {
			return ChooseAddress(IsV4 ? Pools_[DefaultPoolIndex_].AcctV4
									  : Pools_[DefaultPoolIndex_].AcctV6,
								 RequestedAddress);
		}
		case radius_type::coa: {
			return ChooseAddress(IsV4 ? Pools_[DefaultPoolIndex_].CoaV4
									  : Pools_[DefaultPoolIndex_].CoaV6,
								 RequestedAddress);
		}
		}
	}

	Poco::Net::SocketAddress
	RADIUS_proxy_server::Route([[maybe_unused]] radius_type rtype,
							   const Poco::Net::SocketAddress &RequestedAddress,
							   const RADIUS::RadiusPacket &P, bool &UseRADSEC) {
		std::lock_guard G(Mutex_);

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
			return DefaultRoute(rtype, RequestedAddress, P, UseRADSEC);
		}

		auto isAddressInPool = [&](const std::vector<Destination> &D, bool &UseRADSEC) -> bool {
			for (const auto &entry : D)
				if (entry.Addr.host() == RequestedAddress.host()) {
					UseRADSEC = entry.useRADSEC;
					return true;
				}
			return false;
		};

		for (auto &i : Pools_) {
			switch (rtype) {
			case radius_type::coa: {
				if (isAddressInPool((IsV4 ? i.CoaV4 : i.CoaV6), UseRADSEC)) {
					return ChooseAddress(IsV4 ? i.CoaV4 : i.CoaV6, RequestedAddress);
				}
			} break;
			case radius_type::auth: {
				if (isAddressInPool((IsV4 ? i.AuthV4 : i.AuthV6), UseRADSEC)) {
					return ChooseAddress(IsV4 ? i.AuthV4 : i.AuthV6, RequestedAddress);
				}
			} break;
			case radius_type::acct: {
				if (isAddressInPool((IsV4 ? i.AcctV4 : i.AcctV6), UseRADSEC)) {
					return ChooseAddress(IsV4 ? i.AcctV4 : i.AcctV6, RequestedAddress);
				}
			} break;
			}
		}

		UseRADSEC = false;
		return RequestedAddress;
	}

	Poco::Net::SocketAddress
	RADIUS_proxy_server::ChooseAddress(std::vector<Destination> &Pool,
									   const Poco::Net::SocketAddress &OriginalAddress) {

		if (Pool.size() == 1) {
			return Pool[0].Addr;
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
					found = true;
				}
				pos++;
			}

			if (!found) {
				return OriginalAddress;
			}

			Pool[index].state += 1;
			return Pool[index].Addr;
		} else if (Pool[0].strategy == "random") {
			if (Pool.size() > 1) {
				return Pool[std::rand() % Pool.size()].Addr;
			} else {
				return OriginalAddress;
			}
		}
		return OriginalAddress;
	}

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