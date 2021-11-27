//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_server.h"
#include "RTTYS_WebServer.h"
namespace OpenWifi {

	int RTTYS_server::Start() {

		Internal_ = MicroService::instance().ConfigGetBool("rtty.internal",false);
		if(Internal_) {
			std::cout << __LINE__ << std::endl;
			int DSport = MicroService::instance().ConfigGetInt("rtty.port", 5912);
			std::cout << __LINE__ << std::endl;
			int CSport = MicroService::instance().ConfigGetInt("rtty.viewport", 5913);
			std::cout << __LINE__ << std::endl;
			RTTY_UIAssets_ =
				MicroService::instance().ConfigPath("rtty.assets", "$OWGW_ROOT/rtty_ui");
			std::cout << __LINE__ << std::endl;

//			RTTY_UIuri_ = "https://" + MicroService::instance().ConfigGetString("rtty.server");
//			UI_ = RTTY_UIuri_ + ":" + std::to_string(CSport);

std::cout << __LINE__ << std::endl;
			auto CertFileName = MicroService::instance().ConfigPath("openwifi.restapi.host.0.cert");
			auto KeyFileName = MicroService::instance().ConfigPath("openwifi.restapi.host.0.key");
			std::cout << __LINE__ << std::endl;

			auto DSContext =
				new Poco::Net::Context(Poco::Net::Context::SERVER_USE, KeyFileName, CertFileName,
									   "", Poco::Net::Context::VERIFY_RELAXED);
			std::cout << __LINE__ << std::endl;
			Poco::Net::SecureServerSocket DeviceSocket(DSport, 64, DSContext);
			DeviceAcceptor_ =
				std::make_unique<Poco::Net::SocketAcceptor<RTTY_Device_ConnectionHandler>>(
					DeviceSocket, DeviceReactor_);
			std::cout << __LINE__ << std::endl;
			DeviceReactorThread_.start(DeviceReactor_);
			std::cout << __LINE__ << std::endl;

			auto CSContext =
				new Poco::Net::Context(Poco::Net::Context::SERVER_USE, KeyFileName, CertFileName,
									   "", Poco::Net::Context::VERIFY_RELAXED);
			std::cout << __LINE__ << std::endl;
			Poco::Net::SecureServerSocket ClientSocket(CSport, 64, CSContext);
			std::cout << __LINE__ << std::endl;

			auto HttpParams = new Poco::Net::HTTPServerParams;
			HttpParams->setMaxThreads(50);
			HttpParams->setMaxQueued(200);
			HttpParams->setKeepAlive(true);

			std::cout << __LINE__ << std::endl;
			WebServer_ = std::make_unique<Poco::Net::HTTPServer>(
				new RTTY_Client_RequestHandlerFactory(ClientReactor_), ClientSocket, HttpParams);
			std::cout << __LINE__ << std::endl;
			ClientReactorThread_.start(ClientReactor_);
			std::cout << __LINE__ << std::endl;
			WebServer_->start();
			std::cout << __LINE__ << std::endl;
		}

		return 0;
	}

	void RTTYS_server::Stop() {
		if(Internal_) {
			WebServer_->stopAll();
			DeviceReactor_.stop();
			ClientReactor_.stop();
			DeviceReactorThread_.join();
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