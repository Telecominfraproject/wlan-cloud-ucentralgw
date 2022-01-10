//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_server.h"
#include "RTTYS_WebServer.h"
namespace OpenWifi {

	int RTTYS_server::Start() {

		Internal_ = MicroService::instance().ConfigGetBool("rtty.internal",false);
		if(Internal_) {
			int DSport = MicroService::instance().ConfigGetInt("rtty.port", 5912);
			int CSport = MicroService::instance().ConfigGetInt("rtty.viewport", 5913);
			RTTY_UIAssets_ =
				MicroService::instance().ConfigPath("rtty.assets", "$OWGW_ROOT/rtty_ui");

			auto CertFileName = MicroService::instance().ConfigPath("openwifi.restapi.host.0.cert");
			auto KeyFileName = MicroService::instance().ConfigPath("openwifi.restapi.host.0.key");
			auto RootCa = MicroService::instance().ConfigPath("openwifi.restapi.host.0.rootca");
			Poco::Crypto::X509Certificate Root(RootCa);

			auto DSContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE,
									   	KeyFileName, CertFileName, "",
										Poco::Net::Context::VERIFY_RELAXED);

			DSContext->addCertificateAuthority(Root);
			DSContext->disableStatelessSessionResumption();
			DSContext->enableSessionCache();
			DSContext->setSessionCacheSize(0);
			DSContext->setSessionTimeout(10);
			DSContext->enableExtendedCertificateVerification(true);
			SSL_CTX *SSLCtxD = DSContext->sslContext();
			SSL_CTX_dane_enable(SSLCtxD);

			Poco::Net::SecureServerSocket DeviceSocket(DSport, 64, DSContext);
			DeviceSocket.setNoDelay(true);

			DeviceAcceptor_ =
				std::make_unique<Poco::Net::SocketAcceptor<RTTY_Device_ConnectionHandler>>(
					DeviceSocket, DeviceReactor_);

			DeviceReactorThread_.start(DeviceReactor_);

			auto CSContext =
				new Poco::Net::Context(Poco::Net::Context::SERVER_USE, KeyFileName, CertFileName,
									   "", Poco::Net::Context::VERIFY_RELAXED);
			CSContext->addCertificateAuthority(Root);
			CSContext->disableStatelessSessionResumption();
			CSContext->enableSessionCache();
			CSContext->setSessionCacheSize(0);
			CSContext->setSessionTimeout(10);
			CSContext->enableExtendedCertificateVerification(true);
			SSL_CTX *SSLCtxC = DSContext->sslContext();
			SSL_CTX_dane_enable(SSLCtxC);

			Poco::Net::SecureServerSocket ClientSocket(CSport, 64, CSContext);
			ClientSocket.setNoDelay(true);

			auto HttpParams = new Poco::Net::HTTPServerParams;
			HttpParams->setMaxThreads(50);
			HttpParams->setMaxQueued(200);
			HttpParams->setKeepAlive(true);

			WebServer_ = std::make_unique<Poco::Net::HTTPServer>(
				new RTTY_Client_RequestHandlerFactory(ClientReactor_), ClientSocket, HttpParams);
			ClientReactorThread_.start(ClientReactor_);
			ClientReactorThread_.setName("RTTYWebServerClientThread");
			WebServer_->start();
		}

		return 0;
	}

	void RTTYS_server::Stop() {
		if(Internal_) {
			WebServer_->stopAll();
			DeviceReactor_.stop();
			DeviceReactorThread_.join();
			ClientReactor_.stop();
			ClientReactorThread_.join();
		}
	}

	bool RTTYS_server::Login(const std::string & Id) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}
		if(It->second.Device!= nullptr)
			It->second.Device->Login();
		return true;
	}

	bool RTTYS_server::Logout(const std::string & Id) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}
		if(It->second.Device!= nullptr)
			It->second.Device->Logout();
		return true;
	}

	bool RTTYS_server::Close(const std::string & Id) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}

		if(It->second.Device!= nullptr)
			delete It->second.Client;
		It->second.Client = nullptr;
		return true;
	}


}