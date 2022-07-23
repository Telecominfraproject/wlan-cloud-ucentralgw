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
#include <openssl/ssl.h>

namespace OpenWifi {

	void AP_WS_RequestHandler::handleRequest(Poco::Net::HTTPServerRequest &request,
											 Poco::Net::HTTPServerResponse &response)  {
		try {
			new AP_WS_Connection(request,response);
		} catch (...) {
			Logger_.warning("Exception during WS creation");
		}
	};

	bool AP_WS_Server::ValidateCertificate(const std::string & ConnectionId, const Poco::Crypto::X509Certificate & Certificate) {
		if(IsCertOk()) {
			Logger().debug(fmt::format("CERTIFICATE({}): issuer='{}' cn='{}'", ConnectionId, Certificate.issuerName(),Certificate.commonName()));
			if(!Certificate.issuedBy(*IssuerCert_)) {
				Logger().debug(fmt::format("CERTIFICATE({}): issuer mismatch. Local='{}' Incoming='{}'", ConnectionId, IssuerCert_->issuerName(), Certificate.issuerName()));
				return false;
			}
			return true;
		}
		return false;
	}

	int AP_WS_Server::Start() {

		for(const auto & Svr : ConfigServersList_ ) {
			Logger().notice(fmt::format("Starting: {}:{} Keyfile:{} CertFile: {}", Svr.Address(),
										Svr.Port(), Svr.KeyFile(), Svr.CertFile()));

			Svr.LogCert(Logger());
			if (!Svr.RootCA().empty())
				Svr.LogCas(Logger());

			auto Sock{Svr.CreateSecureSocket(Logger())};

			if (!IsCertOk()) {
				IssuerCert_ = std::make_unique<Poco::Crypto::X509Certificate>(Svr.IssuerCertFile());
				Logger().information(
					fmt::format("Certificate Issuer Name:{}", IssuerCert_->issuerName()));
			}

			auto WebServerHttpParams = new Poco::Net::HTTPServerParams;
			WebServerHttpParams->setMaxThreads(50);
			WebServerHttpParams->setMaxQueued(200);
			WebServerHttpParams->setKeepAlive(true);

			auto NewWebServer = std::make_unique<Poco::Net::HTTPServer>(
				new AP_WS_RequestHandlerFactory(Logger()), Sock, WebServerHttpParams);
			WebServers_.push_back(std::move(NewWebServer));
		}

		for(auto &server:WebServers_)
			server->start();

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

		Utils::SetThreadName(ReactorThread_,"device-reactor");

		return 0;
	}

	void AP_WS_Server::Stop() {
		Logger().notice("Stopping reactors...");
		// ReactorPool_.Stop();
		Reactor_.stop();
		ReactorThread_.join();
	}

}      //namespace