//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/JSON/Array.h"

#include "ConfigurationCache.h"
#include "TelemetryStream.h"
#include "WS_Server.h"

namespace OpenWifi {

	bool WebSocketServer::ValidateCertificate(const std::string & ConnectionId, const Poco::Crypto::X509Certificate & Certificate) {
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


	typedef int (*SSL_verify_cb)(int preverify_ok, X509_STORE_CTX *x509_ctx);
	static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx) {
		std::cout << "verify callback" << std::endl;
		return 1;
	}

	int WebSocketServer::Start() {

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

		for(const auto & Svr: ConfigServersList_) {
			Logger().information(fmt::format("Starting: {}:{} Keyfile:{} CertFile: {}", Svr.Address(), Svr.Port(),
											 Svr.KeyFile(),Svr.CertFile()));
			Svr.LogCert(Logger());
			if (!Svr.RootCA().empty())
				Svr.LogCas(Logger());

			Poco::Net::HTTPServerParams::Ptr Params = new Poco::Net::HTTPServerParams;
			Params->setMaxThreads(50);
			Params->setMaxQueued(200);
			Params->setKeepAlive(true);

			std::unique_ptr<Poco::Net::HTTPServer>  NewServer;
			auto Sock{Svr.CreateSecureSocket(Logger())};

			if(!IsCertOk()) {
				IssuerCert_ = std::make_unique<Poco::Crypto::X509Certificate>(Svr.IssuerCertFile());
				Logger().information( fmt::format("Certificate Issuer Name:{}",IssuerCert_->issuerName()));
			}



			auto ctx = Sock.context();
			auto sslCtx = ctx->sslContext();
			SSL_CTX_dane_set_flags(sslCtx,0);
			// SSL_CTX_set_cert_verify_callback
			SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,
							   verify_callback);
			ConnectionServer_ = std::make_unique<Poco::Net::HTTPServer>(new APWebSocketRequestHandlerFactory(Logger(),Reactor_), Sock, Params);
			ConnectionServer_->start();
		}

		ReactorThread_.start(Reactor_);
		Utils::SetThreadName(ReactorThread_,"device-reactor");
		return 0;
	}

/*	int WebSocketServer::Start() {
		// ReactorPool_.Start("DeviceReactorPool_");
        for(const auto & Svr : ConfigServersList_ ) {
            Logger().notice( fmt::format("Starting: {}:{} Keyfile:{} CertFile: {}",
										Svr.Address(),
										Svr.Port(),
										Svr.KeyFile(),Svr.CertFile()));

			Svr.LogCert(Logger());
			if(!Svr.RootCA().empty())
				Svr.LogCas(Logger());

            auto Sock{Svr.CreateSecureSocket(Logger())};

			if(!IsCertOk()) {
				IssuerCert_ = std::make_unique<Poco::Crypto::X509Certificate>(Svr.IssuerCertFile());
				Logger().information( fmt::format("Certificate Issuer Name:{}",IssuerCert_->issuerName()));
			}
			auto NewSocketAcceptor = std::make_unique<ws_server_reactor_type_t>(Sock, Reactor_); // ,   2  );
            Acceptors_.push_back(std::move(NewSocketAcceptor));
        }

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

		ReactorThread_.setStackSize(3000000);
		ReactorThread_.start(Reactor_);
		Utils::SetThreadName(ReactorThread_,"device-reactor");

        return 0;
    }
*/

    void WebSocketServer::Stop() {
        Logger().notice("Stopping reactors...");
		ConnectionServer_->stopAll();
		// ReactorPool_.Stop();
		Reactor_.stop();
		ReactorThread_.join();
    }

}      //namespace
