//
// Created by stephane bourque on 2022-05-18.
//

#include "RADIUS_proxy_server.h"
#include "DeviceRegistry.h"
#include "RADIUS_helpers.h"

namespace OpenWifi {

	const int SMALLEST_RADIUS_PACKET = 20+19+4;
	const int DEFAULT_RADIUS_AUTHENTICATION_PORT = 1812;
	const int DEFAULT_RADIUS_ACCOUNTING_PORT = 1813;
	const int DEFAULT_RADIUS_CoA_PORT = 3799;

	int RADIUS_proxy_server::Start() {

		ConfigFilename_ = MicroService::instance().DataDir()+"/radius_pool_config.json";

		Poco::Net::SocketAddress	AuthSockAddrV4(Poco::Net::AddressFamily::IPv4,
									   MicroService::instance().ConfigGetInt("radius.proxy.authentication.port",DEFAULT_RADIUS_AUTHENTICATION_PORT));
		AuthenticationSocketV4_ = std::make_unique<Poco::Net::DatagramSocket>(AuthSockAddrV4,true);
		Poco::Net::SocketAddress	AuthSockAddrV6(Poco::Net::AddressFamily::IPv6,
											  MicroService::instance().ConfigGetInt("radius.proxy.authentication.port",DEFAULT_RADIUS_AUTHENTICATION_PORT));
		AuthenticationSocketV6_ = std::make_unique<Poco::Net::DatagramSocket>(AuthSockAddrV6,true);

		Poco::Net::SocketAddress	AcctSockAddrV4(Poco::Net::AddressFamily::IPv4,
									   MicroService::instance().ConfigGetInt("radius.proxy.accounting.port",DEFAULT_RADIUS_ACCOUNTING_PORT));
		AccountingSocketV4_ = std::make_unique<Poco::Net::DatagramSocket>(AcctSockAddrV4,true);
		Poco::Net::SocketAddress	AcctSockAddrV6(Poco::Net::AddressFamily::IPv6,
											  MicroService::instance().ConfigGetInt("radius.proxy.accounting.port",DEFAULT_RADIUS_ACCOUNTING_PORT));
		AccountingSocketV6_ = std::make_unique<Poco::Net::DatagramSocket>(AcctSockAddrV6,true);

		Poco::Net::SocketAddress	CoASockAddrV4(Poco::Net::AddressFamily::IPv4,
												MicroService::instance().ConfigGetInt("radius.proxy.coa.port",DEFAULT_RADIUS_CoA_PORT));
		CoASocketV4_ = std::make_unique<Poco::Net::DatagramSocket>(CoASockAddrV4,true);
		Poco::Net::SocketAddress	CoASockAddrV6(Poco::Net::AddressFamily::IPv6,
												MicroService::instance().ConfigGetInt("radius.proxy.coa.port",DEFAULT_RADIUS_CoA_PORT));
		CoASocketV6_ = std::make_unique<Poco::Net::DatagramSocket>(CoASockAddrV6,true);

		AuthenticationReactor_.addEventHandler(*AuthenticationSocketV4_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
														   *this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));
		AuthenticationReactor_.addEventHandler(*AuthenticationSocketV6_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																			 *this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));

		AccountingReactor_.addEventHandler(*AccountingSocketV4_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																		  *this, &RADIUS_proxy_server::OnAccountingSocketReadable));
		AccountingReactor_.addEventHandler(*AccountingSocketV6_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																   *this, &RADIUS_proxy_server::OnAccountingSocketReadable));


		CoAReactor_.addEventHandler(*CoASocketV4_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																			 *this, &RADIUS_proxy_server::OnCoASocketReadable));
		CoAReactor_.addEventHandler(*CoASocketV6_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																	 *this, &RADIUS_proxy_server::OnCoASocketReadable));

		AuthenticationReactorThread_.start(AuthenticationReactor_);
		AccountingReactorThread_.start(AccountingReactor_);
		CoAReactorThread_.start(CoAReactor_);
		return 0;
	}

	void RADIUS_proxy_server::Stop() {
		AuthenticationReactor_.removeEventHandler(*AuthenticationSocketV4_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																		  *this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));
		AuthenticationReactor_.removeEventHandler(*AuthenticationSocketV6_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																				*this, &RADIUS_proxy_server::OnAuthenticationSocketReadable));

		AccountingReactor_.removeEventHandler(*AccountingSocketV4_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																  *this, &RADIUS_proxy_server::OnAccountingSocketReadable));
		AccountingReactor_.removeEventHandler(*AccountingSocketV6_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																	  *this, &RADIUS_proxy_server::OnAccountingSocketReadable));

		CoAReactor_.removeEventHandler(*CoASocketV4_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																		*this, &RADIUS_proxy_server::OnAccountingSocketReadable));
		CoAReactor_.removeEventHandler(*CoASocketV6_,Poco::NObserver<RADIUS_proxy_server, Poco::Net::ReadableNotification>(
																		*this, &RADIUS_proxy_server::OnAccountingSocketReadable));

		AuthenticationReactor_.stop();
		AuthenticationReactorThread_.join();

		AccountingReactor_.stop();
		AccountingReactorThread_.join();

		CoAReactor_.stop();
		CoAReactorThread_.join();
	}

	void RADIUS_proxy_server::OnAccountingSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		Poco::Net::SocketAddress	Sender;
		RADIUS::RadiusPacket		P;

		std::cout << "Accounting bytes received" << std::endl;

		auto ReceiveSize = pNf.get()->socket().impl()->receiveBytes(P.Buffer(),P.BufferLen());
		if(ReceiveSize<SMALLEST_RADIUS_PACKET) {
			std::cout << "Runt packet" << std::endl;
			return;
		}
		P.Evaluate(ReceiveSize);
		auto SerialNumber = P.ExtractSerialNumberFromProxyState();
		if(SerialNumber.empty()) {
			std::cout << "Invalid or missing serial number" << std::endl;
			return;
		}

		Logger().information(fmt::format("Accounting Packet received for {}",SerialNumber));
		std::cout << "Received an Accounting packet for :" << SerialNumber << std::endl;
		DeviceRegistry()->SendRadiusAccountingData(SerialNumber,P.Buffer(),P.Size());
	}

	void RADIUS_proxy_server::OnAuthenticationSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		Poco::Net::SocketAddress	Sender;
		RADIUS::RadiusPacket		P;

		std::cout << "Authentication bytes received" << std::endl;

		auto ReceiveSize = pNf.get()->socket().impl()->receiveBytes(P.Buffer(),P.BufferLen());
		if(ReceiveSize<SMALLEST_RADIUS_PACKET) {
			std::cout << "Runt packet" << std::endl;
			return;
		}
		P.Evaluate(ReceiveSize);
		auto SerialNumber = P.ExtractSerialNumberFromProxyState();
		if(SerialNumber.empty()) {
			std::cout << "Invalid or missing serial number" << std::endl;
			return;
		}
		Logger().information(fmt::format("Authentication Packet received for {}",SerialNumber));
		std::cout << "Received an Authentication packet for :" << SerialNumber << std::endl;
		DeviceRegistry()->SendRadiusAuthenticationData(SerialNumber,P.Buffer(),P.Size());
	}

	void RADIUS_proxy_server::OnCoASocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		Poco::Net::SocketAddress	Sender;
		RADIUS::RadiusPacket		P;

		std::cout << "CoA bytes received" << std::endl;

		auto ReceiveSize = pNf.get()->socket().impl()->receiveBytes(P.Buffer(),P.BufferLen());
		if(ReceiveSize<SMALLEST_RADIUS_PACKET) {
			std::cout << "Runt packet" << std::endl;
			return;
		}
		P.Evaluate(ReceiveSize);
		auto SerialNumber = P.ExtractSerialNumberTIP();
		if(SerialNumber.empty()) {
			std::cout << "Invalid or missing serial number" << std::endl;
			return;
		}
		Logger().information(fmt::format("CoA Packet received for {}",SerialNumber));
		std::cout << "Received an CoA packet for :" << SerialNumber << std::endl;
		DeviceRegistry()->SendRadiusCoAData(SerialNumber,P.Buffer(),P.Size());
	}

	void RADIUS_proxy_server::SendAccountingData(const std::string &serialNumber, const char *buffer, std::size_t size) {
		RADIUS::RadiusPacket	P((unsigned char *)buffer,size);
		auto Destination = P.ExtractProxyStateDestination();
		Poco::Net::SocketAddress	Dst(Destination);

		std::lock_guard	G(Mutex_);
		if(Dst.af()==Poco::Net::AddressFamily::IPv4)
			AccountingSocketV4_->sendTo(buffer,(int)size,Route(radius_type::acct, Dst));
		else
			AccountingSocketV6_->sendTo(buffer,(int)size,Route(radius_type::acct, Dst));
		Logger().information(fmt::format("{}: Sending Accounting Packet to {}", serialNumber, Destination));
		std::cout << "Sending Accounting data to " << Destination << std::endl;
	}

	void RADIUS_proxy_server::SendAuthenticationData(const std::string &serialNumber, const char *buffer, std::size_t size) {
		RADIUS::RadiusPacket	P((unsigned char *)buffer,size);
		auto Destination = P.ExtractProxyStateDestination();
		Poco::Net::SocketAddress	Dst(Destination);

		std::lock_guard	G(Mutex_);
		if(Dst.af()==Poco::Net::AddressFamily::IPv4)
			AuthenticationSocketV4_->sendTo(buffer,(int)size,Route(radius_type::auth, Dst));
		else
			AuthenticationSocketV6_->sendTo(buffer,(int)size,Route(radius_type::auth, Dst));
		Logger().information(fmt::format("{}: Sending Authentication Packet to {}", serialNumber, Destination));
		std::cout << "Sending Authentication data to " << Destination << std::endl;
	}

	void RADIUS_proxy_server::SendCoAData(const std::string &serialNumber, const char *buffer, std::size_t size) {
		RADIUS::RadiusPacket	P((unsigned char *)buffer,size);
		auto Destination = P.ExtractProxyStateDestination();
		Poco::Net::SocketAddress	Dst(Destination);

		std::lock_guard	G(Mutex_);
		if(Dst.af()==Poco::Net::AddressFamily::IPv4)
			CoASocketV4_->sendTo(buffer,(int)size,Route(radius_type::coa, Dst));
		else
			CoASocketV6_->sendTo(buffer,(int)size,Route(radius_type::coa, Dst));
		Logger().information(fmt::format("{}: Sending CoA Packet to {}", serialNumber, Destination));
		std::cout << "Sending CoA data to " << Destination << std::endl;
	}

	void RADIUS_proxy_server::ParseServerList(const GWObjects::RadiusProxyServerConfig & Config, std::vector<Destination> &V4, std::vector<Destination> &V6, bool setAsDefault) {
		uint64_t TotalV4=0, TotalV6=0;

		for(const auto &server:Config.servers) {
			Poco::Net::IPAddress a;
			if(!Poco::Net::IPAddress::tryParse(server.ip,a)) {
				Logger().error(fmt::format("RADIUS-PARSE Config: server address {} is nto a valid address in v4 or v6. Entry skipped.",server.ip));
				continue;
			}
			auto S = Poco::Net::SocketAddress(fmt::format("{}:{}",server.ip,server.port));
			Destination	D{
				.Addr = S,
				.state = 0,
				.step = 0,
				.weight = server.weight,
				.available = true,
				.strategy = Config.strategy,
				.monitor = Config. monitor,
				.monitorMethod = Config.monitorMethod,
				.methodParameters = Config.methodParameters,
				.useAsDefault = setAsDefault
			};

			if(S.af()==Poco::Net::AddressFamily::IPv4) {
				TotalV4 += server.weight;
				V4.push_back(D);
			} else {
				TotalV6 += server.weight;
				V6.push_back(D);
			}
		}

		for(auto &i:V4) {
			if(TotalV4==0) {
				i.step = 1000;
			} else {
				i.step = 1000 - ((1000 * i.weight) / TotalV4);
			}
		}

		for(auto &i:V6) {
			if(TotalV6==0) {
				i.step = 1000;
			} else {
				i.step = 1000 - ((1000 * i.weight) / TotalV6);
			}
		}
	}

	void RADIUS_proxy_server::ParseConfig() {

		try {
			Poco::File	F(ConfigFilename_);

			std::lock_guard	G(Mutex_);

			if(F.exists()) {
				std::ifstream ifs(ConfigFilename_,std::ios_base::binary);
				Poco::JSON::Parser	P;
				auto RawConfig = P.parse(ifs).extract<Poco::JSON::Object::Ptr>();
				GWObjects::RadiusProxyPoolList	RPC;
				if(RPC.from_json(RawConfig)) {
					ResetConfig();
					PoolList_ = RPC;
					for(const auto &pool:RPC.pools) {
						RadiusPool	NewPool;
						ParseServerList(pool.authConfig, NewPool.AuthV4, NewPool.AuthV6, pool.useByDefault);
						ParseServerList(pool.acctConfig, NewPool.AcctV4, NewPool.AcctV6, pool.useByDefault);
						ParseServerList(pool.coaConfig, NewPool.CoaV4, NewPool.CoaV6, pool.useByDefault);
						Pools_.push_back(NewPool);
					}
				} else {
					Logger().warning(fmt::format("Configuration file '{}' is bad.",ConfigFilename_));
				}
			} else {
				Logger().warning(fmt::format("No configuration file '{}' exists.",ConfigFilename_));
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			Logger().error(fmt::format("Error while parsing configuration file '{}'",ConfigFilename_));
		}
	}

	Poco::Net::SocketAddress RADIUS_proxy_server::DefaultRoute([[maybe_unused]] radius_type rtype, const Poco::Net::SocketAddress &RequestedAddress) {
		bool IsV4 = RequestedAddress.af()==Poco::Net::IPAddress::IPv4;
		switch(rtype) {
		case radius_type::coa:
			return ChooseAddress(IsV4 ? Pools_[defaultPoolIndex_].CoaV4 : Pools_[defaultPoolIndex_].CoaV6, RequestedAddress);
		case radius_type::auth:
			return ChooseAddress(IsV4 ? Pools_[defaultPoolIndex_].AuthV4 : Pools_[defaultPoolIndex_].AuthV6, RequestedAddress);
		case radius_type::acct:
			return ChooseAddress(IsV4 ? Pools_[defaultPoolIndex_].AcctV4 : Pools_[defaultPoolIndex_].AcctV6, RequestedAddress);
		}
	}

	Poco::Net::SocketAddress RADIUS_proxy_server::Route([[maybe_unused]] radius_type rtype, const Poco::Net::SocketAddress &RequestedAddress) {
		std::lock_guard	G(Mutex_);

		std::cout << __LINE__ << std::endl;
		if(Pools_.empty())
			return RequestedAddress;

		bool IsV4 = RequestedAddress.af()==Poco::Net::IPAddress::IPv4;
		bool useDefault = false;
		useDefault = IsV4 ? RequestedAddress.host() == Poco::Net::IPAddress::wildcard(Poco::Net::IPAddress::IPv4) : RequestedAddress.host() == Poco::Net::IPAddress::wildcard(Poco::Net::IPAddress::IPv6) ;

		if(useDefault) {
			return DefaultRoute(rtype, RequestedAddress);
		}

		auto isAddressInPool = [&](const std::vector<Destination> & D) -> bool {
			for(const auto &entry:D)
				if(entry.Addr.host()==RequestedAddress.host())
					return true;
			return false;
		};

		for(auto &i:Pools_) {
			switch(rtype) {
			case radius_type::coa:
				if(isAddressInPool((IsV4 ? i.CoaV4 : i.CoaV6)))
					return ChooseAddress(IsV4 ? i.CoaV4 : i.CoaV6, RequestedAddress);
			case radius_type::auth:
				if(isAddressInPool((IsV4 ? i.AuthV4 : i.AuthV6)))
					return ChooseAddress(IsV4 ? i.AuthV4 : i.AuthV6, RequestedAddress);
			case radius_type::acct:
				if(isAddressInPool((IsV4 ? i.AcctV4 : i.AcctV6)))
					return ChooseAddress(IsV4 ? i.AcctV4 : i.AcctV6, RequestedAddress);
			}
		}
		return DefaultRoute(rtype, RequestedAddress);
	}

	Poco::Net::SocketAddress RADIUS_proxy_server::ChooseAddress(std::vector<Destination> &Pool, const Poco::Net::SocketAddress & OriginalAddress) {

		std::cout << __LINE__ << std::endl;
		if(Pool.size()==1) {
			std::cout << __LINE__ << std::endl;
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
				std::cout << __LINE__ << std::endl;
				return OriginalAddress;
			}

			Pool[index].state += Pool[index].step;
			std::cout << __LINE__ << std::endl;
			return Pool[index].Addr;
		} else if (Pool[0].strategy == "round_robin") {
			bool found = false;
			std::cout << __LINE__ << std::endl;
			uint64_t cur_state = std::numeric_limits<uint64_t>::max();
			std::cout << __LINE__ << std::endl;
			std::size_t pos = 0, index = 0;
			std::cout << __LINE__ << std::endl;
			for (auto &i : Pool) {
				if (!i.available) {
					std::cout << __LINE__ << std::endl;
					i.state += 1;
					continue;
				}
				if (i.state < cur_state) {
					index = pos;
					cur_state = i.state;
					found = true;
					std::cout << __LINE__ << std::endl;
				}
				pos++;
				std::cout << __LINE__ << std::endl;
			}

			if (!found) {
				std::cout << __LINE__ << std::endl;
				return OriginalAddress;
			}

			Pool[index].state += 1;
			std::cout << __LINE__ << std::endl;
			return Pool[index].Addr;
		} else if (Pool[0].strategy == "random") {
			if (Pool.size() > 1) {
				std::cout << __LINE__ << std::endl;
				return Pool[std::rand() % Pool.size()].Addr;
			} else {
				std::cout << __LINE__ << std::endl;
				return OriginalAddress;
			}
		}
		std::cout << __LINE__ << std::endl;
		return OriginalAddress;
	}

	void RADIUS_proxy_server::SetConfig(const GWObjects::RadiusProxyPoolList &C) {
		std::lock_guard	G(Mutex_);
		PoolList_ = C;

		Poco::JSON::Object	Disk;
		C.to_json(Disk);

		std::ofstream ofs(ConfigFilename_, std::ios_base::trunc | std::ios_base::binary );
		Disk.stringify(ofs);
		ofs.close();

		ParseConfig();
	}

	void RADIUS_proxy_server::ResetConfig() {
		PoolList_.pools.clear();
		Pools_.clear();
		defaultPoolIndex_=0;
	}

	void RADIUS_proxy_server::DeleteConfig() {
		std::lock_guard	G(Mutex_);

		try {
			Poco::File F(ConfigFilename_);
			if (F.exists())
				F.remove();
		} catch (...) {

		}
		ResetConfig();
	}

	void RADIUS_proxy_server::GetConfig(GWObjects::RadiusProxyPoolList &C) {
		std::lock_guard	G(Mutex_);
		C = PoolList_;
	}

}