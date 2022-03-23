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

	int WebSocketServer::Start() {
		ReactorPool_.Start("DeviceReactorPool_");
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
			auto NewSocketAcceptor = std::make_unique<Poco::Net::ParallelSocketAcceptor<WSConnection, Poco::Net::SocketReactor>>(Sock, Reactor_,  Poco::Environment::processorCount()*2);
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

		ReactorThread_.start(Reactor_);

        return 0;
    }

    void WebSocketServer::Stop() {
        Logger().notice("Stopping reactors...");
		ReactorPool_.Stop();
		Reactor_.stop();
		ReactorThread_.join();
    }

}      //namespace
