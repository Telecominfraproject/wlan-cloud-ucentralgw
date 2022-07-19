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
#include <openssl/ssl.h>

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

	static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
	{
		char    buf[256];
		X509   *err_cert;
		int     err, depth;
		SSL    *ssl;

		err_cert = X509_STORE_CTX_get_current_cert(ctx);
		err = X509_STORE_CTX_get_error(ctx);
		depth = X509_STORE_CTX_get_error_depth(ctx);

		std::cout << __LINE__ << "  " << preverify_ok << std::endl;
		if (!preverify_ok) {
			printf("verify error:num=%d:%s:depth=%d:%s\n", err,
				   X509_verify_cert_error_string(err), depth, buf);
		}

		std::cout << __LINE__ << std::endl;
//		if (!preverify_ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT))
//		{
			std::cout << __LINE__ << std::endl;
			X509_NAME_oneline(X509_get_issuer_name(X509_STORE_CTX_get0_current_issuer(ctx)), buf, 256);
			printf("issuer= %s\n", buf);
			X509_NAME_oneline(X509_get_subject_name(X509_STORE_CTX_get0_current_issuer(ctx)), buf, 256);
			printf("subject= %s\n", buf);
//		}

		std::cout << __LINE__ << std::endl;
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
			ctx->enableExtendedCertificateVerification(false);
			auto SSL_CTX = ctx->sslContext();
			SSL_CTX_set_verify(SSL_CTX,SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE,verify_callback);
			ConnectionServer_ = std::make_unique<Poco::Net::HTTPServer>(new APWebSocketRequestHandlerFactory(ctx,Logger(),Reactor_), Sock, Params);
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
