//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_server.h"
#include "RTTYS_WebServer.h"
namespace OpenWifi {

	int RTTYS_server::Start() {
		int DSport = MicroService::instance().ConfigGetInt("srtty.device.port", 5912);
		int CSport = MicroService::instance().ConfigGetInt("srtty.client.port",5913);
		RTTY_UIAssets_ = MicroService::instance().ConfigPath("srtty.assets", "$OWGW_ROOT/rtty_ui");
		RTTY_UIuri_ = MicroService::instance().ConfigPath("srtty.ui");

		UI_ = RTTY_UIuri_ + ":" + std::to_string(CSport);

		auto CertFileName = MicroService::instance().ConfigPath("ucentral.websocket.host.0.cert");
		auto KeyFileName = MicroService::instance().ConfigPath("ucentral.websocket.host.0.key");
		auto CALocation = MicroService::instance().ConfigPath("ucentral.websocket.host.0.cas");

		auto DSContext = new Poco::Net::Context( Poco::Net::Context::SERVER_USE,
												KeyFileName, CertFileName, CALocation, Poco::Net::Context::VERIFY_RELAXED);
		Poco::Net::SecureServerSocket	DeviceSocket(DSport,64,DSContext);
		auto CSContext = new Poco::Net::Context( Poco::Net::Context::SERVER_USE,
												 KeyFileName, CertFileName, CALocation, Poco::Net::Context::VERIFY_RELAXED);
		Poco::Net::SecureServerSocket	ClientSocket(CSport,64,CSContext);

		DeviceAcceptor_ = std::make_unique<Poco::Net::SocketAcceptor<RTTY_Device_ConnectionHandler>>(DeviceSocket, DeviceReactor_);

		DeviceReactorThread_.start(DeviceReactor_);
		ClientReactorThread_.start(ClientReactor_);

		WebServer_ = std::make_unique<Poco::Net::HTTPServer>(new RTTY_Client_RequestHandlerFactory(ClientReactor_), ClientSocket, new Poco::Net::HTTPServerParams);
		WebServer_->start();

		return 0;
	}

	void RTTYS_server::Stop() {
		WebServer_->stopAll();
		DeviceReactor_.stop();
		ClientReactor_.stop();
		DeviceReactorThread_.join();
		ClientReactorThread_.join();
	}
}