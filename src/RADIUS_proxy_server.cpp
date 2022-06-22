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
			AccountingSocketV4_->sendTo(buffer,(int)size,Route(radius_type::acct, Dst,AcctPoolsV4_,AcctPoolsIndexV4_));
		else
			AccountingSocketV6_->sendTo(buffer,(int)size,Route(radius_type::acct, Dst,AcctPoolsV6_,AcctPoolsIndexV6_));
		Logger().information(fmt::format("{}: Sending Accounting Packet to {}", serialNumber, Destination));
		std::cout << "Sending Accounting data to " << Destination << std::endl;
	}

	void RADIUS_proxy_server::SendAuthenticationData(const std::string &serialNumber, const char *buffer, std::size_t size) {
		RADIUS::RadiusPacket	P((unsigned char *)buffer,size);
		auto Destination = P.ExtractProxyStateDestination();
		Poco::Net::SocketAddress	Dst(Destination);

		std::lock_guard	G(Mutex_);
		if(Dst.af()==Poco::Net::AddressFamily::IPv4)
			AuthenticationSocketV4_->sendTo(buffer,(int)size,Route(radius_type::auth, Dst,AuthPoolsV4_,AuthPoolsIndexV4_));
		else
			AuthenticationSocketV6_->sendTo(buffer,(int)size,Route(radius_type::auth, Dst,AuthPoolsV6_,AuthPoolsIndexV6_));
		Logger().information(fmt::format("{}: Sending Authentication Packet to {}", serialNumber, Destination));
		std::cout << "Sending Authentication data to " << Destination << std::endl;
	}

	void RADIUS_proxy_server::SendCoAData(const std::string &serialNumber, const char *buffer, std::size_t size) {
		RADIUS::RadiusPacket	P((unsigned char *)buffer,size);
		auto Destination = P.ExtractProxyStateDestination();
		Poco::Net::SocketAddress	Dst(Destination);

		std::lock_guard	G(Mutex_);
		if(Dst.af()==Poco::Net::AddressFamily::IPv4)
			CoASocketV4_->sendTo(buffer,(int)size,Route(radius_type::coa, Dst,CoAPoolsV4_,CoAPoolsIndexV4_));
		else
			CoASocketV6_->sendTo(buffer,(int)size,Route(radius_type::coa, Dst,CoAPoolsV6_,CoAPoolsIndexV6_));
		Logger().information(fmt::format("{}: Sending CoA Packet to {}", serialNumber, Destination));
		std::cout << "Sending CoA data to " << Destination << std::endl;
	}

	void RADIUS_proxy_server::ParseServerList(const GWObjects::RadiusProxyServerConfig & Config, PoolIndexMap_t &MapV4, PoolIndexMap_t &MapV6, PoolIndexVec_t &VecV4, PoolIndexVec_t &VecV6, bool setAsDefault) {

		std::vector<Destination>	DestsV4,DestsV6;
		uint64_t TotalV4=0, TotalV6=0;

		for(const auto &server:Config.servers) {
			auto S = Poco::Net::SocketAddress(fmt::format("{}:{}",server.name,server.port));
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
				DestsV4.push_back(D);
				MapV4[S] = VecV4.size();
			} else {
				TotalV6 += server.weight;
				DestsV6.push_back(D);
				MapV6[S] = VecV6.size();
			}
		}

		for(auto &i:DestsV4) {
			i.step = 1000 - ((1000*i.weight)/TotalV4);
		}

		for(auto &i:DestsV6) {
			i.step = 1000 - ((1000*i.weight)/TotalV6);
		}

		if(!DestsV4.empty())
			VecV4.push_back(DestsV4);
		if(!DestsV6.empty())
			VecV6.push_back((DestsV6));
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
						ParseServerList(pool.authConfig, AuthPoolsIndexV4_, AuthPoolsIndexV6_, AuthPoolsV4_, AuthPoolsV6_, pool.useByDefault);
						ParseServerList(pool.acctConfig, AcctPoolsIndexV4_, AcctPoolsIndexV6_, AcctPoolsV4_, AcctPoolsV6_, pool.useByDefault);
						ParseServerList(pool.coaConfig, CoAPoolsIndexV4_, CoAPoolsIndexV6_, CoAPoolsV4_, CoAPoolsV6_, pool.useByDefault);
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

	Poco::Net::SocketAddress RADIUS_proxy_server::Route([[maybe_unused]] radius_type rtype, const Poco::Net::SocketAddress &OriginalAddress, PoolIndexVec_t & P, PoolIndexMap_t &M) {
		std::lock_guard	G(Mutex_);

		if(PoolList_.pools.empty())
			return OriginalAddress;

		bool useDefault = false;
		if(OriginalAddress.host() == Poco::Net::SocketAddress("0.0.0.0:0").host()) {
			useDefault = true;
		}

		auto It = M.find(OriginalAddress);
		if(!useDefault && It==M.end())
			return OriginalAddress;

		if(useDefault || It==M.end()) {
			for(auto &svr:P) {
				if(svr[0].useAsDefault)
					return ChooseAddress(svr,OriginalAddress);
			}
			return ChooseAddress(P[0],OriginalAddress);
		} else {
			auto Pool = P[It->second];
			return ChooseAddress(Pool, OriginalAddress);
		}
	}

	Poco::Net::SocketAddress RADIUS_proxy_server::ChooseAddress(std::vector<Destination> &Pool, const Poco::Net::SocketAddress & OriginalAddress) {

		if(Pool.size()==1)
			return Pool[0].Addr;

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

			if (!found)
				return OriginalAddress;

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

			if (!found)
				return OriginalAddress;

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
		AuthPoolsIndexV4_.clear();
		AuthPoolsIndexV6_.clear();
		AcctPoolsIndexV4_.clear();
		AcctPoolsIndexV6_.clear();
		CoAPoolsIndexV4_.clear();
		CoAPoolsIndexV6_.clear();
		AuthPoolsV4_.clear();
		AuthPoolsV6_.clear();
		AcctPoolsV4_.clear();
		AcctPoolsV6_.clear();
		CoAPoolsV4_.clear();
		CoAPoolsV6_.clear();
		defaultPoolIndex=0;
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