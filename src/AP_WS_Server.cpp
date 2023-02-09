//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPServerRequest.h"

#include "AP_WS_Server.h"
#include "AP_WS_Connection.h"
#include "ConfigurationCache.h"
#include "TelemetryStream.h"

#include "framework/MicroServiceFuncs.h"
#include "framework/utils.h"
#include "UI_GW_WebSocketNotifications.h"
#include "fmt/format.h"

namespace OpenWifi {

	void AP_WS_RequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
											 Poco::Net::HTTPServerResponse &response)  {
		try {
			AP_WS_Server()->AddConnection(id_,std::make_shared<AP_WS_Connection>(request,response,id_, Logger_, AP_WS_Server()->NextReactor()));
		} catch (...) {
			poco_warning(Logger_,"Exception during WS creation");
		}
	};

	bool AP_WS_Server::ValidateCertificate(const std::string & ConnectionId, const Poco::Crypto::X509Certificate & Certificate) {
		if(IsCertOk()) {
			if(!Certificate.issuedBy(*IssuerCert_)) {
				poco_warning(Logger(),fmt::format("CERTIFICATE({}): issuer mismatch. Local='{}' Incoming='{}'", ConnectionId, IssuerCert_->issuerName(), Certificate.issuerName()));
				return false;
			}
			return true;
		}
		return false;
	}

	int AP_WS_Server::Start() {

		AllowSerialNumberMismatch_ = MicroServiceConfigGetBool("openwifi.certificates.allowmismatch",true);
		MismatchDepth_ = MicroServiceConfigGetInt("openwifi.certificates.mismatchdepth",2);

		Reactor_pool_ = std::make_unique<AP_WS_ReactorThreadPool>();
		Reactor_pool_->Start();

		for(const auto & Svr : ConfigServersList_ ) {

			poco_notice(Logger(),fmt::format("Starting: {}:{} Keyfile:{} CertFile: {}", Svr.Address(),
										Svr.Port(), Svr.KeyFile(), Svr.CertFile()));

			Svr.LogCert(Logger());
			if (!Svr.RootCA().empty())
				Svr.LogCas(Logger());

			if (!IsCertOk()) {
				IssuerCert_ = std::make_unique<Poco::Crypto::X509Certificate>(Svr.IssuerCertFile());
				poco_information(Logger(),
					fmt::format("Certificate Issuer Name:{}", IssuerCert_->issuerName()));
			}

			Poco::Net::Context::Params P;

			P.verificationMode = Poco::Net::Context::VERIFY_ONCE;
			P.verificationDepth = 9;
			P.loadDefaultCAs = Svr.RootCA().empty();
			P.cipherList = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
			P.dhUse2048Bits = true;
			P.caLocation = Svr.Cas();

			auto Context = Poco::AutoPtr<Poco::Net::Context>(new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P));

			Poco::Crypto::X509Certificate Cert(Svr.CertFile());
			Poco::Crypto::X509Certificate Root(Svr.RootCA());

			Context->useCertificate(Cert);
			Context->addChainCertificate(Root);

			Context->addCertificateAuthority(Root);
			Poco::Crypto::X509Certificate Issuing(Svr.IssuerCertFile());
			Context->addChainCertificate(Issuing);
			Context->addCertificateAuthority(Issuing);

			Poco::Crypto::RSAKey Key("", Svr.KeyFile(), Svr.KeyFilePassword());
			Context->usePrivateKey(Key);

			Context->setSessionCacheSize(0);
			Context->setSessionTimeout(120);
			Context->flushSessionCache();
			Context->enableSessionCache(true);
			Context->enableExtendedCertificateVerification(false);
			// Context->disableStatelessSessionResumption();
			Context->disableProtocols(Poco::Net::Context::PROTO_TLSV1 | Poco::Net::Context::PROTO_TLSV1_1);

			auto WebServerHttpParams = new Poco::Net::HTTPServerParams;
			WebServerHttpParams->setMaxThreads(50);
			WebServerHttpParams->setMaxQueued(200);
			WebServerHttpParams->setKeepAlive(true);
			WebServerHttpParams->setName("ws:ap_dispatch");

			if (Svr.Address() == "*") {
				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, Svr.Port());
				auto NewWebServer = std::make_unique<Poco::Net::HTTPServer>(
					new AP_WS_RequestHandlerFactory(Logger()), DeviceConnectionPool_, Poco::Net::SecureServerSocket(SockAddr, Svr.Backlog(), Context), WebServerHttpParams);
				WebServers_.push_back(std::move(NewWebServer));
			} else {
				Poco::Net::IPAddress Addr(Svr.Address());
				Poco::Net::SocketAddress SockAddr(Addr, Svr.Port());
				auto NewWebServer = std::make_unique<Poco::Net::HTTPServer>(
					new AP_WS_RequestHandlerFactory(Logger()), DeviceConnectionPool_, Poco::Net::SecureServerSocket(SockAddr, Svr.Backlog(), Context), WebServerHttpParams);
				WebServers_.push_back(std::move(NewWebServer));
			}
		}

		for(auto &server:WebServers_) {
			server->start();
		}

		ReactorThread_.start(Reactor_);

		auto ProvString = MicroServiceConfigGetString("autoprovisioning.process","default");
		if(ProvString!="default") {
			auto Tokens = Poco::StringTokenizer(ProvString, ",");
			for (const auto &i : Tokens) {
				if (i == "prov")
					LookAtProvisioning_ = true;
				else
					UseDefaultConfig_ = true;
			}
		} else {
			UseDefaultConfig_ = true;
		}

		SimulatorId_ = MicroServiceConfigGetString("simulatorid","");
		SimulatorEnabled_ = !SimulatorId_.empty();
		Utils::SetThreadName(ReactorThread_,"dev:react:head");

		GarbageCollectorCallback_ = std::make_unique<Poco::TimerCallback<AP_WS_Server>>(*this,&AP_WS_Server::onGarbageCollecting);
		Timer_.setStartInterval(10 * 1000);
		Timer_.setPeriodicInterval(5 * 1000); // every minute
		Timer_.start(*GarbageCollectorCallback_, MicroServiceTimerPool());

		Running_ = true;
		return 0;
	}

	void AP_WS_Server::onGarbageCollecting([[maybe_unused]] Poco::Timer &timer) {
		std::lock_guard			Lock(WSServerMutex_);
		if(!Garbage_.empty()) {
			Garbage_.clear();
		}

		static uint64_t last_log = Utils::Now();

		NumberOfConnectedDevices_ = 0;
		NumberOfConnectingDevices_ = 0;
		AverageDeviceConnectionTime_ = 0;
		uint64_t	total_connected_time=0;

		auto now = Utils::Now();
		for (const auto & connection:SerialNumbers_) {
			if(connection.second.second == nullptr) {
				continue;
			}
			if (connection.second.second->State_.Connected) {
				NumberOfConnectedDevices_++;
				total_connected_time += (now - connection.second.second->State_.started);
			} else {
				NumberOfConnectingDevices_++;
			}
		}

		AverageDeviceConnectionTime_ = (NumberOfConnectedDevices_!=0) ? total_connected_time/NumberOfConnectedDevices_ : 0;
		if((now-last_log)>120) {
			last_log = now;
			poco_information(Logger(),
							 fmt::format("Active AP connections: {} Connecting: {} Average connection time: {} seconds",
										 NumberOfConnectedDevices_, NumberOfConnectingDevices_, AverageDeviceConnectionTime_));
		}

		GWWebSocketNotifications::NumberOfConnection_t	Notification;
		Notification.content.numberOfConnectingDevices = NumberOfConnectingDevices_;
		Notification.content.numberOfDevices = NumberOfConnectedDevices_;
		Notification.content.averageConnectedTime = AverageDeviceConnectionTime_;
		GWWebSocketNotifications::NumberOfConnections(Notification);
	}

	void AP_WS_Server::Stop() {
		poco_information(Logger(),"Stopping...");
		Running_ = false;

		Timer_.stop();

		for(auto &server:WebServers_) {
			server->stopAll();
		}
		Reactor_pool_->Stop();
		Reactor_.stop();
		ReactorThread_.join();
		poco_information(Logger(),"Stopped...");
	}

	bool AP_WS_Server::GetStatistics(uint64_t SerialNumber, std::string &Statistics) const {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == SerialNumbers_.end() || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->GetLastStats(Statistics);
		return true;
	}

	bool AP_WS_Server::GetState(uint64_t SerialNumber, GWObjects::ConnectionState & State) const {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == SerialNumbers_.end() || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->GetState(State);
		return true;
	}

	bool AP_WS_Server::GetHealthcheck(uint64_t SerialNumber, GWObjects::HealthCheck & CheckData) const {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == SerialNumbers_.end() || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->GetLastHealthCheck(CheckData);
		return true;
	}

	void AP_WS_Server::SetSessionDetails(uint64_t connection_id, uint64_t SerialNumber) {
		std::lock_guard			Lock(WSServerMutex_);

		auto Conn = Sessions_.find(connection_id);
		if(Conn == end(Sessions_))
			return;

		auto CurrentSerialNumber = SerialNumbers_.find(SerialNumber);
		if(	(CurrentSerialNumber==SerialNumbers_.end())	||
			(CurrentSerialNumber->second.first<connection_id)) {
			SerialNumbers_[SerialNumber] = std::make_pair(connection_id, Conn->second);
			return;
		}
	}

	bool AP_WS_Server::EndSession(uint64_t session_id, uint64_t serial_number) {
		std::lock_guard G(WSServerMutex_);

		auto Session = Sessions_.find(session_id);
		if(Session==end(Sessions_))
			return false;

		Garbage_.push_back(Session->second);

		auto Device = SerialNumbers_.find(serial_number);
		if (Device == end(SerialNumbers_)) {
			Sessions_.erase(Session);
			return false;
		}

		if(Device->second.first==session_id) {
			Sessions_.erase(Session);
			SerialNumbers_.erase(Device);
			return true;
		}

		Sessions_.erase(Session);
		return false;
	}

	bool AP_WS_Server::Connected(uint64_t SerialNumber, GWObjects::DeviceRestrictions & Restrictions) const {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == end(SerialNumbers_) || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->GetRestrictions(Restrictions);
		return  DevicePtr->State_.Connected;
	}

	bool AP_WS_Server::Connected(uint64_t SerialNumber) const {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == end(SerialNumbers_) || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}
		return  DevicePtr->State_.Connected;
	}

	bool AP_WS_Server::SendFrame(uint64_t SerialNumber, const std::string & Payload) const {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == SerialNumbers_.end() || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}
		try {
			return DevicePtr->Send(Payload);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendFrame: Could not send data to device '{}'", Utils::IntToSerialNumber(SerialNumber)));
		}
		return false;
	}

	void AP_WS_Server::StopWebSocketTelemetry(uint64_t RPCID, uint64_t SerialNumber) {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);

			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == end(SerialNumbers_) || Device->second.second == nullptr) {
				return;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->StopWebSocketTelemetry(RPCID);
	}

	void AP_WS_Server::SetWebSocketTelemetryReporting(uint64_t RPCID, uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime, const std::vector<std::string> & TelemetryTypes) {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == end(SerialNumbers_) || Device->second.second == nullptr) {
				return;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->SetWebSocketTelemetryReporting(RPCID, Interval, Lifetime, TelemetryTypes);
	}

	void AP_WS_Server::SetKafkaTelemetryReporting(uint64_t RPCID, uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime, const std::vector<std::string> & TelemetryTypes) {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == end(SerialNumbers_) || Device->second.second == nullptr) {
				return;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->SetKafkaTelemetryReporting(RPCID, Interval, Lifetime, TelemetryTypes);
	}

	void AP_WS_Server::StopKafkaTelemetry(uint64_t RPCID, uint64_t SerialNumber) {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == end(SerialNumbers_) || Device->second.second == nullptr) {
				return;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->StopKafkaTelemetry(RPCID);
	}

	void AP_WS_Server::GetTelemetryParameters(uint64_t SerialNumber , bool & TelemetryRunning,
												uint64_t & TelemetryInterval,
												uint64_t & TelemetryWebSocketTimer,
												uint64_t & TelemetryKafkaTimer,
												uint64_t & TelemetryWebSocketCount,
												uint64_t & TelemetryKafkaCount,
												uint64_t & TelemetryWebSocketPackets,
												uint64_t & TelemetryKafkaPackets) {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if (Device == end(SerialNumbers_) || Device->second.second == nullptr) {
				return;
			}
			DevicePtr = Device->second.second;
		}
		DevicePtr->GetTelemetryParameters(
			TelemetryRunning, TelemetryInterval, TelemetryWebSocketTimer, TelemetryKafkaTimer,
			TelemetryWebSocketCount, TelemetryKafkaCount, TelemetryWebSocketPackets,
			TelemetryKafkaPackets);
	}

	bool AP_WS_Server::SendRadiusAccountingData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
			if (Device == SerialNumbers_.end() || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}

		try {
			return DevicePtr->SendRadiusAccountingData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	bool AP_WS_Server::SendRadiusAuthenticationData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
			if (Device == SerialNumbers_.end() || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}

		try {
			return DevicePtr->SendRadiusAuthenticationData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	bool AP_WS_Server::SendRadiusCoAData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_ptr<AP_WS_Connection>	DevicePtr;
		{
			std::lock_guard Lock(WSServerMutex_);
			auto Device = SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
			if (Device == SerialNumbers_.end() || Device->second.second == nullptr) {
				return false;
			}
			DevicePtr = Device->second.second;
		}

		try {
			return DevicePtr->SendRadiusCoAData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusCoAData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

}      //namespace