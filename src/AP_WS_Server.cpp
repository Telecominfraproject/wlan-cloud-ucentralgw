//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Net/Context.h"
#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/Net/HTTPServerRequest.h"

#include "AP_WS_Connection.h"
#include "AP_WS_Server.h"
#include "ConfigurationCache.h"
#include "TelemetryStream.h"

#include "UI_GW_WebSocketNotifications.h"
#include "fmt/format.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/utils.h"
#include <framework/KafkaManager.h>

namespace OpenWifi {

	void AP_WS_RequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
											 Poco::Net::HTTPServerResponse &response) {
		try {
			AP_WS_Server()->AddConnection(
				id_, std::make_shared<AP_WS_Connection>(request, response, id_, Logger_,
														AP_WS_Server()->NextReactor()));
		} catch (...) {
			poco_warning(Logger_, "Exception during WS creation");
		}
	};

	bool AP_WS_Server::ValidateCertificate(const std::string &ConnectionId,
										   const Poco::Crypto::X509Certificate &Certificate) {
		if (IsCertOk()) {
			if (!Certificate.issuedBy(*IssuerCert_)) {
				poco_warning(
					Logger(),
					fmt::format("CERTIFICATE({}): issuer mismatch. Local='{}' Incoming='{}'",
								ConnectionId, IssuerCert_->issuerName(), Certificate.issuerName()));
				return false;
			}
			return true;
		}
		return false;
	}

	int AP_WS_Server::Start() {

		AllowSerialNumberMismatch_ =
			MicroServiceConfigGetBool("openwifi.certificates.allowmismatch", true);
		MismatchDepth_ = MicroServiceConfigGetInt("openwifi.certificates.mismatchdepth", 2);

		SessionTimeOut_ = MicroServiceConfigGetInt("openwifi.session.timeout", 10*60);

		Reactor_pool_ = std::make_unique<AP_WS_ReactorThreadPool>(Logger());
		Reactor_pool_->Start();

		for (const auto &Svr : ConfigServersList_) {

			poco_notice(Logger(),
						fmt::format("Starting: {}:{} Keyfile:{} CertFile: {}", Svr.Address(),
									Svr.Port(), Svr.KeyFile(), Svr.CertFile()));

			Svr.LogCert(Logger());
			if (!Svr.RootCA().empty())
				Svr.LogCas(Logger());

			if (!IsCertOk()) {
				IssuerCert_ = std::make_unique<Poco::Crypto::X509Certificate>(Svr.IssuerCertFile());
				poco_information(
					Logger(), fmt::format("Certificate Issuer Name:{}", IssuerCert_->issuerName()));
			}

			Poco::Net::Context::Params P;

			P.verificationMode = Poco::Net::Context::VERIFY_ONCE;
			P.verificationDepth = 9;
			P.loadDefaultCAs = Svr.RootCA().empty();
			P.cipherList = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
			P.dhUse2048Bits = true;
			P.caLocation = Svr.Cas();

			auto Context = Poco::AutoPtr<Poco::Net::Context>(
				new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P));

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
			Context->disableProtocols(Poco::Net::Context::PROTO_TLSV1 |
									  Poco::Net::Context::PROTO_TLSV1_1);

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
					new AP_WS_RequestHandlerFactory(Logger()), DeviceConnectionPool_,
					Poco::Net::SecureServerSocket(SockAddr, Svr.Backlog(), Context),
					WebServerHttpParams);
				WebServers_.push_back(std::move(NewWebServer));
			} else {
				Poco::Net::IPAddress Addr(Svr.Address());
				Poco::Net::SocketAddress SockAddr(Addr, Svr.Port());
				auto NewWebServer = std::make_unique<Poco::Net::HTTPServer>(
					new AP_WS_RequestHandlerFactory(Logger()), DeviceConnectionPool_,
					Poco::Net::SecureServerSocket(SockAddr, Svr.Backlog(), Context),
					WebServerHttpParams);
				WebServers_.push_back(std::move(NewWebServer));
			}
		}

		for (auto &server : WebServers_) {
			server->start();
		}

		ReactorThread_.start(Reactor_);

		auto ProvString = MicroServiceConfigGetString("autoprovisioning.process", "default");
		if (ProvString != "default") {
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

		SimulatorId_ = MicroServiceConfigGetString("simulatorid", "");
		SimulatorEnabled_ = !SimulatorId_.empty();
		Utils::SetThreadName(ReactorThread_, "dev:react:head");

/*
		GarbageCollectorCallback_ = std::make_unique<Poco::TimerCallback<AP_WS_Server>>(
			*this, &AP_WS_Server::onGarbageCollecting);
		Timer_.setStartInterval(10 * 1000);
		Timer_.setPeriodicInterval(10 * 1000); // every minute
		Timer_.start(*GarbageCollectorCallback_, MicroServiceTimerPool());
*/
		Running_ = true;
		GarbageCollector_.setName("ws:garbage");
		GarbageCollector_.start(*this);
		return 0;
	}

	void AP_WS_Server::run() {
		uint64_t last_log = Utils::Now(),
				 last_zombie_run = 0,
				 last_garbage_run = 0;

		while(Running_) {

			if(!Poco::Thread::trySleep(30000)) {
				break;
			}

			Logger().information(fmt::format("Garbage collecting starting run."	));
			{
				std::lock_guard SessionLock(GarbageMutex_);
				if (!GarbageSessions_.empty()) {
					Logger().information(fmt::format("Garbage collecting removing {} stale connections.", GarbageSessions_.size()));
					GarbageSessions_.clear();
					Logger().information(fmt::format("Garbage collecting removed stale connections."));
				}
			}

			uint64_t total_connected_time = 0, now = Utils::Now();

			if(now-last_zombie_run > 60) {
				poco_information(Logger(), fmt::format("Garbage collecting zombies... (step 1)"));
				std::vector<std::uint64_t> SessionsToRemove;
				NumberOfConnectedDevices_ = 0;
				NumberOfConnectingDevices_ = 0;
				AverageDeviceConnectionTime_ = 0;
				last_zombie_run = now;
				for(int hashIndex=0;hashIndex<256;hashIndex++) {
					std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
					auto hint = SerialNumbers_[hashIndex].begin();
					while (hint != end(SerialNumbers_[hashIndex])) {
						if (hint->second.second == nullptr) {
							hint = SerialNumbers_[hashIndex].erase(hint);
						} else if ((now - hint->second.second->State_.LastContact) >
								   SessionTimeOut_) {
							poco_information(
								Logger(),
								fmt::format(
									"{}: Session seems idle. Controller disconnecting device.",
									hint->second.second->SerialNumber_));
							std::cout << __LINE__ << ": AP_WS_Server" << std::endl;
							std::lock_guard ConnectionLock(hint->second.second->ConnectionMutex_);
							std::cout << __LINE__ << ": AP_WS_Server" << std::endl;
							hint->second.second->EndConnection();
							hint = SerialNumbers_[hashIndex].erase(hint);
						} else if (hint->second.second->State_.Connected) {
							NumberOfConnectedDevices_++;
							total_connected_time += (now - hint->second.second->State_.started);
							hint++;
						} else {
							NumberOfConnectingDevices_++;
							hint++;
						}
					}
				}

				poco_information(Logger(), fmt::format("Garbage collecting zombies... (step 2)"));
				LeftOverSessions_ = 0;
				for(int i=0;i<256;i++) {
					std::lock_guard Lock(SessionMutex_[i]);
					auto hint = Sessions_[i].begin();
					while (hint != end(Sessions_[i])) {
						if ((now - hint->second->LastContact_) > SessionTimeOut_) {
							poco_information(
								Logger(),
								fmt::format(
									"{}: Session seems idle. Controller disconnecting device.",
									hint->second->SerialNumber_));
							hint = Sessions_[i].erase(hint);
						} else {
							LeftOverSessions_++;
							hint++;
						}
					}
				}

				AverageDeviceConnectionTime_ =
					NumberOfConnectedDevices_ > 0 ? total_connected_time / NumberOfConnectedDevices_
												  : 0;
				poco_information(Logger(), fmt::format("Garbage collecting zombies done..."));

			} else {
				NumberOfConnectedDevices_=0;
				for(int i=0;i<256;i++) {
					std::lock_guard Lock(SerialNumbersMutex_[i]);
					NumberOfConnectedDevices_ += SerialNumbers_[i].size();
				}
				if(last_garbage_run>0)
					AverageDeviceConnectionTime_ += (now - last_garbage_run);
			}

			if ((now - last_log) > 60) {
				last_log = now;
				poco_information(Logger(),
								 fmt::format("Active AP connections: {} Connecting: {} Average connection time: {} seconds. Left Over Sessions: {}",
											 NumberOfConnectedDevices_, NumberOfConnectingDevices_,
											 AverageDeviceConnectionTime_, LeftOverSessions_));
			}

			GWWebSocketNotifications::NumberOfConnection_t Notification;
			Notification.content.numberOfConnectingDevices = NumberOfConnectingDevices_;
			Notification.content.numberOfDevices = NumberOfConnectedDevices_;
			Notification.content.averageConnectedTime = AverageDeviceConnectionTime_;
			GetTotalDataStatistics(Notification.content.tx,Notification.content.rx);
			GWWebSocketNotifications::NumberOfConnections(Notification);

			Poco::JSON::Object	KafkaNotification;
			Notification.to_json(KafkaNotification);

			Poco::JSON::Object FullEvent;
			FullEvent.set("type", "load-update");
			FullEvent.set("timestamp", now);
			FullEvent.set("payload", KafkaNotification);

			KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, "system", FullEvent);
			Logger().information(fmt::format("Garbage collection finished run."	));
			last_garbage_run = now;
		}
		Logger().information(fmt::format("Garbage collector done for the day."	));
	}

	void AP_WS_Server::Stop() {
		poco_information(Logger(), "Stopping...");
		Running_ = false;

		GarbageCollector_.wakeUp();
		GarbageCollector_.join();

		for (auto &server : WebServers_) {
			server->stopAll();
		}

		Reactor_pool_->Stop();
		Reactor_.stop();
		ReactorThread_.join();
		poco_information(Logger(), "Stopped...");
	}

	bool AP_WS_Server::GetHealthDevices(std::uint64_t lowLimit, std::uint64_t  highLimit, std::vector<std::string> & SerialNumbers) {
		SerialNumbers.clear();
		for(int i=0;i<256;i++) {
			std::lock_guard Lock(SessionMutex_[i]);
			for (const auto &connection : Sessions_[i]) {
				if (connection.second->RawLastHealthcheck_.Sanity >= lowLimit &&
					connection.second->RawLastHealthcheck_.Sanity <= highLimit) {
					SerialNumbers.push_back(connection.second->SerialNumber_);
				}
			}
		}
		return true;
	}

	bool AP_WS_Server::GetStatistics(uint64_t SerialNumber, std::string &Statistics) const {

		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == SerialNumbers_[hashIndex].end() || Device->second.second == nullptr) {
			return false;
		}
		Device->second.second->GetLastStats(Statistics);

		return true;
	}

	bool AP_WS_Server::GetState(uint64_t SerialNumber, GWObjects::ConnectionState &State) const {
		std::shared_ptr<AP_WS_Connection> Connection;
		{
			auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
			std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
			auto DeviceHint = SerialNumbers_[hashIndex].find(SerialNumber);
			if (DeviceHint == SerialNumbers_[hashIndex].end() ||
				DeviceHint->second.second == nullptr) {
				return false;
			}
			Connection = DeviceHint->second.second;
		}
		std::cout << __LINE__ << " " << Connection->State_.sessionId << ":" << Connection->SerialNumber_ << std::endl;
		Connection->GetState(State);
		std::cout << __LINE__ << " " << Connection->State_.sessionId << ":" << Connection->SerialNumber_ << std::endl;
		return true;
	}

	bool AP_WS_Server::GetHealthcheck(uint64_t SerialNumber,
									  GWObjects::HealthCheck &CheckData) const {

		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == SerialNumbers_[hashIndex].end() || Device->second.second == nullptr) {
			return false;
		}
		Device->second.second->GetLastHealthCheck(CheckData);
		return true;

	}

	void AP_WS_Server::SetSessionDetails(uint64_t connection_id, uint64_t SerialNumber) {
		std::shared_ptr<AP_WS_Connection> Connection;

		std::cout << __LINE__ << ": Attempting to set connection details" << std::endl;

		std::lock_guard SessionLock(SessionMutex_[connection_id % 256]);
		auto ConnHint = Sessions_[connection_id % 256].find(connection_id);
		if (ConnHint == end(Sessions_[connection_id % 256])) {
			std::cout << __LINE__ << ": Failed (1) to set connection details" << std::endl;
			return;
		}
		Connection = ConnHint->second;

		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto CurrentSerialNumber = SerialNumbers_[hashIndex].find(SerialNumber);
		if ((CurrentSerialNumber == SerialNumbers_[hashIndex].end()) ||
			(CurrentSerialNumber->second.first < connection_id)) {
			SerialNumbers_[hashIndex][SerialNumber] = std::make_pair(connection_id, Connection);
			Sessions_[connection_id % 256].erase(ConnHint);
			std::cout << __LINE__ << ": Set connection details" << std::endl;
			return;
		}
		std::cout << __LINE__ << ": Failed to set connection details" << std::endl;
	}

	bool AP_WS_Server::EndSession(uint64_t session_id, uint64_t SerialNumber) {

		{
			std::lock_guard SessionLock(SessionMutex_[session_id % 256]);
			auto Session = Sessions_[session_id % 256].find(session_id);
			if (Session != end(Sessions_[session_id % 256])) {
				Sessions_[session_id % 256].erase(Session);
			}
		}

		{
			auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
			std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
			auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
			if (Device == SerialNumbers_[hashIndex].end()
				|| Device->second.second == nullptr
				|| Device->second.second->State_.sessionId != session_id) {
				return false;
			}
			SerialNumbers_[hashIndex].erase(Device);
		}
		return true;
	}


	bool AP_WS_Server::Connected(uint64_t SerialNumber,
								 GWObjects::DeviceRestrictions &Restrictions) const {
		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return false;
		}
		Device->second.second->GetRestrictions(Restrictions);
		return Device->second.second->State_.Connected;
	}

	bool AP_WS_Server::Connected(uint64_t SerialNumber) const {
		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return false;
		}
		return Device->second.second->State_.Connected;
	}

	bool AP_WS_Server::SendFrame(uint64_t SerialNumber, const std::string &Payload) const {
		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return false;
		}

		try {
			return Device->second.second->Send(Payload);
		} catch (...) {
			poco_debug(Logger(), fmt::format(": SendFrame: Could not send data to device '{}'",
											 Utils::IntToSerialNumber(SerialNumber)));
		}
		return false;
	}

	void AP_WS_Server::StopWebSocketTelemetry(uint64_t RPCID, uint64_t SerialNumber) {
		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return;
		}
		Device->second.second->StopWebSocketTelemetry(RPCID);
	}

	void
	AP_WS_Server::SetWebSocketTelemetryReporting(uint64_t RPCID, uint64_t SerialNumber,
												 uint64_t Interval, uint64_t Lifetime,
												 const std::vector<std::string> &TelemetryTypes) {
		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return;
		}
		Device->second.second->SetWebSocketTelemetryReporting(RPCID, Interval, Lifetime, TelemetryTypes);
	}

	void AP_WS_Server::SetKafkaTelemetryReporting(uint64_t RPCID, uint64_t SerialNumber,
												  uint64_t Interval, uint64_t Lifetime,
												  const std::vector<std::string> &TelemetryTypes) {
		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return;
		}
		Device->second.second->SetKafkaTelemetryReporting(RPCID, Interval, Lifetime, TelemetryTypes);
	}

	void AP_WS_Server::StopKafkaTelemetry(uint64_t RPCID, uint64_t SerialNumber) {
		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return;
		}
		Device->second.second->StopKafkaTelemetry(RPCID);
	}

	void AP_WS_Server::GetTelemetryParameters(
		uint64_t SerialNumber, bool &TelemetryRunning, uint64_t &TelemetryInterval,
		uint64_t &TelemetryWebSocketTimer, uint64_t &TelemetryKafkaTimer,
		uint64_t &TelemetryWebSocketCount, uint64_t &TelemetryKafkaCount,
		uint64_t &TelemetryWebSocketPackets, uint64_t &TelemetryKafkaPackets) {

		auto hashIndex = Utils::CalculateMacAddressHash(SerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(SerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return;
		}

		Device->second.second->GetTelemetryParameters(TelemetryRunning, TelemetryInterval,
										  TelemetryWebSocketTimer, TelemetryKafkaTimer,
										  TelemetryWebSocketCount, TelemetryKafkaCount,
										  TelemetryWebSocketPackets, TelemetryKafkaPackets);
	}

	bool AP_WS_Server::SendRadiusAccountingData(const std::string &SerialNumber,
												const unsigned char *buffer, std::size_t size) {

		auto IntSerialNumber = Utils::SerialNumberToInt(SerialNumber);
		auto hashIndex = Utils::CalculateMacAddressHash(IntSerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(IntSerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return false;
		}

		try {
			return Device->second.second->SendRadiusAccountingData(buffer, size);
		} catch (...) {
			poco_debug(
				Logger(),
				fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'",
							SerialNumber));
		}
		return false;
	}

	bool AP_WS_Server::SendRadiusAuthenticationData(const std::string &SerialNumber,
													const unsigned char *buffer, std::size_t size) {
		auto IntSerialNumber = Utils::SerialNumberToInt(SerialNumber);
		auto hashIndex = Utils::CalculateMacAddressHash(IntSerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(IntSerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return false;
		}

		try {
			return Device->second.second->SendRadiusAuthenticationData(buffer, size);
		} catch (...) {
			poco_debug(
				Logger(),
				fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'",
							SerialNumber));
		}
		return false;
	}

	bool AP_WS_Server::SendRadiusCoAData(const std::string &SerialNumber,
										 const unsigned char *buffer, std::size_t size) {
		auto IntSerialNumber = Utils::SerialNumberToInt(SerialNumber);
		auto hashIndex = Utils::CalculateMacAddressHash(IntSerialNumber);
		std::lock_guard Lock(SerialNumbersMutex_[hashIndex]);
		auto Device = SerialNumbers_[hashIndex].find(IntSerialNumber);
		if (Device == end(SerialNumbers_[hashIndex]) || Device->second.second == nullptr) {
			return false;
		}

		try {
			return Device->second.second->SendRadiusCoAData(buffer, size);
		} catch (...) {
			poco_debug(Logger(),
					   fmt::format(": SendRadiusCoAData: Could not send data to device '{}'",
								   SerialNumber));
		}
		return false;
	}

} // namespace OpenWifi