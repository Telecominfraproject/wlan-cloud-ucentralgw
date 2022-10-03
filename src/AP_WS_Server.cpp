//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/JSON/Array.h"
#include "Poco/Net/Context.h"

#include "AP_WS_Server.h"
#include "AP_WS_Connection.h"
#include "ConfigurationCache.h"
#include "TelemetryStream.h"
#include "framework/WebSocketClientNotifications.h"

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

		AllowSerialNumberMismatch_ = MicroService::instance().ConfigGetBool("openwifi.certificates.allowmismatch",true);
		MismatchDepth_ = MicroService::instance().ConfigGetInt("openwifi.certificates.mismatchdepth",2);

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

			if(!Svr.KeyFilePassword().empty()) {
				auto PassphraseHandler = Poco::SharedPtr<MyPrivateKeyPassphraseHandler>( new MyPrivateKeyPassphraseHandler(Svr.KeyFilePassword(),Logger()));
				Poco::Net::SSLManager::instance().initializeServer(PassphraseHandler, nullptr,Context);
			}

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

		auto ProvString = MicroService::instance().ConfigGetString("autoprovisioning.process","default");
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

		SimulatorId_ = MicroService::instance().ConfigGetString("simulatorid","");
		SimulatorEnabled_ = !SimulatorId_.empty();
		Utils::SetThreadName(ReactorThread_,"dev:react:head");

		GarbageCollectorCallback_ = std::make_unique<Poco::TimerCallback<AP_WS_Server>>(*this,&AP_WS_Server::onGarbageCollecting);
		Timer_.setStartInterval(10 * 1000);
		Timer_.setPeriodicInterval(5 * 1000); // every minute
		Timer_.start(*GarbageCollectorCallback_, MicroService::instance().TimerPool());

		Running_ = true;
		return 0;
	}

	void AP_WS_Server::onGarbageCollecting([[maybe_unused]] Poco::Timer &timer) {
		std::unique_lock	Lock(LocalMutex_);
		std::cout << "Removing " << Garbage_.size() << " old connections." << std::endl;
		Garbage_.clear();
		static std::uint64_t last_log = OpenWifi::Now();

		NumberOfConnectedDevices_ = 0;
		NumberOfConnectingDevices_ = 0;
		AverageDeviceConnectionTime_ = 0;
		std::uint64_t	total_connected_time=0;

		auto now = OpenWifi::Now();
		for (auto connection=SerialNumbers_.begin(); connection!=SerialNumbers_.end();) {

			if(connection->second.second== nullptr) {
				connection++;
				continue;
			}

			if (connection->second.second->State_.Connected) {
				NumberOfConnectedDevices_++;
				total_connected_time += (now - connection->second.second->State_.started);
				connection++;
			} else {
				NumberOfConnectingDevices_++;
				connection++;
			}
		}

		AverageDeviceConnectionTime_ = (NumberOfConnectedDevices_!=0) ? total_connected_time/NumberOfConnectedDevices_ : 0;
		if((now-last_log)>120) {
			last_log = now;
			poco_information(Logger(),
							 fmt::format("Active AP connections: {} Connecting: {} Average connection time: {} seconds",
										 NumberOfConnectedDevices_, NumberOfConnectingDevices_, AverageDeviceConnectionTime_));
		}
		WebSocketClientNotificationNumberOfConnections(NumberOfConnectedDevices_,
													   AverageDeviceConnectionTime_,
													   NumberOfConnectingDevices_);
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

	bool AP_WS_Server::GetStatistics(std::uint64_t SerialNumber, std::string &Statistics) const {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device == SerialNumbers_.end() || Device->second.second==nullptr)
			return false;
		Statistics = Device->second.second->LastStats_;
		return true;
	}

	bool AP_WS_Server::GetState(uint64_t SerialNumber, GWObjects::ConnectionState & State) const {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device == SerialNumbers_.end() || Device->second.second==nullptr)
			return false;
		State = Device->second.second->State_;
		return true;
	}

	bool AP_WS_Server::GetHealthcheck(uint64_t SerialNumber, GWObjects::HealthCheck & CheckData) const {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device == SerialNumbers_.end() || Device->second.second==nullptr)
			return false;

		CheckData = Device->second.second->LastHealthcheck_;
		return true;
	}

	void AP_WS_Server::SetSessionDetails(std::uint64_t connection_id, uint64_t SerialNumber) {
		std::shared_lock	Guard(LocalMutex_);

		auto Conn = Sessions_.find(connection_id);
		if(Conn == end(Sessions_))
			return;

		auto CurrentSerialNumber = SerialNumbers_.find(SerialNumber);
		if(	(CurrentSerialNumber==SerialNumbers_.end())	||
			(CurrentSerialNumber->second.first<connection_id)) {
			SerialNumbers_[SerialNumber] = std::make_pair(connection_id, Conn->second.first);
			return;
		}
	}

	bool AP_WS_Server::EndSession(std::uint64_t session_id, std::uint64_t serial_number) {
		std::unique_lock G(LocalMutex_);

		auto Session = Sessions_.find(session_id);
		if(Session==end(Sessions_))
			return false;

		Garbage_.push_back(Session->second.first);

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

	bool AP_WS_Server::Connected(uint64_t SerialNumber) const {
		std::shared_lock Guard(LocalMutex_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second== nullptr)
			return false;

		return  Device->second.second->State_.Connected;
	}

	bool AP_WS_Server::SendFrame(uint64_t SerialNumber, const std::string & Payload) const {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==SerialNumbers_.end() || Device->second.second== nullptr)
			return false;

		try {
			// std::cout << "Device connection pointer: " << (std::uint64_t) Device->second.second << std::endl;
			return Device->second.second->Send(Payload);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendFrame: Could not send data to device '{}'", Utils::IntToSerialNumber(SerialNumber)));
		}
		return false;
	}

	void AP_WS_Server::StopWebSocketTelemetry(std::uint64_t RPCID, uint64_t SerialNumber) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second==nullptr)
			return;
		Device->second.second->StopWebSocketTelemetry(RPCID);
	}

	void AP_WS_Server::SetWebSocketTelemetryReporting(std::uint64_t RPCID, uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second==nullptr)
			return;
		Device->second.second->SetWebSocketTelemetryReporting(RPCID, Interval, Lifetime);
	}

	void AP_WS_Server::SetKafkaTelemetryReporting(std::uint64_t RPCID, uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second== nullptr)
			return;
		Device->second.second->SetKafkaTelemetryReporting(RPCID, Interval, Lifetime);
	}

	void AP_WS_Server::StopKafkaTelemetry(std::uint64_t RPCID, uint64_t SerialNumber) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second== nullptr)
			return;
		Device->second.second->StopKafkaTelemetry(RPCID);
	}

	void AP_WS_Server::GetTelemetryParameters(uint64_t SerialNumber , bool & TelemetryRunning,
												uint64_t & TelemetryInterval,
												uint64_t & TelemetryWebSocketTimer,
												uint64_t & TelemetryKafkaTimer,
												uint64_t & TelemetryWebSocketCount,
												uint64_t & TelemetryKafkaCount,
												uint64_t & TelemetryWebSocketPackets,
												uint64_t & TelemetryKafkaPackets) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_)|| Device->second.second== nullptr)
			return;
		Device->second.second->GetTelemetryParameters(TelemetryRunning,
													  TelemetryInterval,
													  TelemetryWebSocketTimer,
													  TelemetryKafkaTimer,
													  TelemetryWebSocketCount,
													  TelemetryKafkaCount,
													  TelemetryWebSocketPackets,
													  TelemetryKafkaPackets);
	}

	bool AP_WS_Server::SendRadiusAccountingData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end() || Device->second.second== nullptr)
			return false;

		try {
			return Device->second.second->SendRadiusAccountingData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	bool AP_WS_Server::SendRadiusAuthenticationData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end() || Device->second.second== nullptr)
			return false;

		try {
			return Device->second.second->SendRadiusAuthenticationData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	bool AP_WS_Server::SendRadiusCoAData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end() || Device->second.second== nullptr)
			return false;

		try {
			return Device->second.second->SendRadiusCoAData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusCoAData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

}      //namespace