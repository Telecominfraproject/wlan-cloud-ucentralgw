//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_server.h"

#include "Poco/URI.h"

#include "RESTAPI_BlackList.h"
#include "RESTAPI_command.h"
#include "RESTAPI_commands.h"
#include "RESTAPI_default_configuration.h"
#include "RESTAPI_default_configurations.h"
#include "RESTAPI_device_commandHandler.h"
#include "RESTAPI_device_handler.h"
#include "RESTAPI_devices_handler.h"
#include "RESTAPI_file.h"
#include "RESTAPI_system_command.h"
#include "RESTAPI_ouis.h"
#include "RESTAPI_deviceDashboardHandler.h"

#include "Utils.h"
#include "RESTAPI_webSocketServer.h"
#include "RESTAPI_TelemetryWebSocket.h"

namespace OpenWifi {

    class RESTAPI_server *RESTAPI_server::instance_ = nullptr;

    int RESTAPI_server::Start() {
        Logger_.information("Starting.");
		Server_.InitLogging();
        for(const auto & Svr: ConfigServersList_) {
			Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.Address(), std::to_string(Svr.Port()),
											 Svr.KeyFile(),Svr.CertFile()));

            auto Sock{Svr.CreateSecureSocket(Logger_)};

			Svr.LogCert(Logger_);
			if(!Svr.RootCA().empty())
				Svr.LogCas(Logger_);

            auto Params = new Poco::Net::HTTPServerParams;
            Params->setMaxThreads(50);
            Params->setMaxQueued(200);
			Params->setKeepAlive(true);

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new RESTAPIServerRequestHandlerFactory(Server_), Pool_, Sock, Params);
            NewServer->start();
            RESTServers_.push_back(std::move(NewServer));
        }

        return 0;
    }

	void RESTAPI_server::Stop() {
		Logger_.information("Stopping ");
		for( const auto & svr : RESTServers_ )
			svr->stop();
	}

	Poco::Net::HTTPRequestHandler *RESTAPIServerRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {
        Poco::URI uri(Request.getURI());
        const auto & Path = uri.getPath();
        RESTAPIHandler::BindingMap Bindings;

		return RESTAPI_Router<
								RESTAPI_devices_handler,
							  	RESTAPI_device_handler,
								RESTAPI_device_commandHandler,
								RESTAPI_default_configurations,
								RESTAPI_default_configuration,
								RESTAPI_command,
								RESTAPI_commands,
							  	RESTAPI_ouis,
								RESTAPI_file,
								RESTAPI_system_command,
								RESTAPI_deviceDashboardHandler,
								RESTAPI_webSocketServer,
								RESTAPI_BlackList,
								RESTAPI_TelemetryWebSocket>(Path,Bindings,Logger_, Server_);
    }

}  // namespace