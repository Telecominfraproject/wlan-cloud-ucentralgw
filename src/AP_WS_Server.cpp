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
#include "ConfigurationCache.h"
#include "TelemetryStream.h"

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
			Context->setSessionTimeout(60);
			Context->enableSessionCache(false);
			Context->enableExtendedCertificateVerification(false);
			Context->disableStatelessSessionResumption();
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

		Running_ = true;
		return 0;
	}

	void AP_WS_Server::Stop() {
		poco_information(Logger(),"Stopping...");
		Running_ = false;

		for(auto &server:WebServers_) {
			server->stopAll();
		}
		Reactor_pool_->Stop();
		Reactor_.stop();
		ReactorThread_.join();
		poco_information(Logger(),"Stopped...");
	}

}      //namespace