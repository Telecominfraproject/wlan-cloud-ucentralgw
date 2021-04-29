//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralRESTAPIServer.h"

#include "Poco/URI.h"

#include "RESTAPI_oauth2Handler.h"
#include "RESTAPI_devicesHandler.h"
#include "RESTAPI_deviceHandler.h"
#include "RESTAPI_deviceCommandHandler.h"
#include "RESTAPI_default_configuration.h"
#include "RESTAPI_default_configurations.h"
#include "RESTAPI_commands.h"
#include "RESTAPI_command.h"
#include "RESTAPI_file.h"
#include "RESTAPI_SystemCommand.h"
#include "RESTAPI_BlackList.h"

#include "RESTAPI_unknownRequestHandler.h"

namespace uCentral::RESTAPI {

    Service *Service::instance_ = nullptr;

    int Start() {
        return uCentral::RESTAPI::Service::instance()->Start();
    }

    void Stop() {
        uCentral::RESTAPI::Service::instance()->Stop();
    }

    Service::Service() noexcept:
            SubSystemServer("RESTAPIServer", "RESTAPIServer", "ucentral.restapi")
    {
    }

    int Service::Start() {
        Logger_.information("Starting.");

        for(const auto & Svr: ConfigServersList_) {
			Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.address(), std::to_string(Svr.port()),
											 Svr.key_file(),Svr.cert_file()));

            auto Sock{Svr.CreateSecureSocket()};

			Svr.log_cert(Logger_);
			if(!Svr.root_ca().empty())
				Svr.log_cas(Logger_);

            auto Params = new Poco::Net::HTTPServerParams;
            Params->setMaxThreads(16);
            Params->setMaxQueued(100);

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new RequestHandlerFactory, Pool_, Sock, Params);
            NewServer->start();
            RESTServers_.push_back(std::move(NewServer));
        }

        return 0;
    }

    Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest & Request) {

        Logger_.information("Request from "
                            + Request.clientAddress().toString()
                            + ": "
                            + Request.getMethod()
                            + " "
                            + Request.getURI()
                            + " "
                            + Request.getVersion()
                            );

        Poco::URI uri(Request.getURI());
        auto *path = uri.getPath().c_str();
        RESTAPIHandler::BindingMap bindings;

        if (RESTAPIHandler::ParseBindings(path, "/api/v1/oauth2", bindings)) {
            return new RESTAPI_oauth2Handler(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/oauth2/{token}", bindings)) {
            return new RESTAPI_oauth2Handler(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/devices", bindings)) {
            return new RESTAPI_devicesHandler(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/device/{serialNumber}/{command}", bindings)) {
            return new RESTAPI_deviceCommandHandler(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/device/{serialNumber}", bindings)) {
            return new RESTAPI_deviceHandler(bindings, Logger_);
        }  else if (RESTAPIHandler::ParseBindings(path, "/api/v1/default_configurations", bindings)) {
            return new RESTAPI_default_configurations(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/default_configuration/{name}", bindings)) {
            return new RESTAPI_default_configuration(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/command/{commandUUID}", bindings)) {
            return new RESTAPI_command(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/commands", bindings)) {
            return new RESTAPI_commands(bindings, Logger_);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/file/{uuid}", bindings)) {
            return new RESTAPI_file(bindings, Logger_);
		} else if (RESTAPIHandler::ParseBindings(path, "/api/v1/system", bindings)) {
			return new RESTAPI_SystemCommand(bindings, Logger_);
		} else if (RESTAPIHandler::ParseBindings(path, "/api/v1/blacklist", bindings)) {
			return new RESTAPI_BlackList(bindings, Logger_);
		}

        return new RESTAPI_UnknownRequestHandler(bindings,Logger_);
    }

    void Service::Stop() {
        Logger_.information("Stopping ");
        for( const auto & svr : RESTServers_ )
            svr->stop();
    }

}  // namespace