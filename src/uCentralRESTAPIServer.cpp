//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralRESTAPIServer.h"

#include "Poco/URI.h"

#include "RESTAPI_oauth2Handler.h"
#include "RESTAPI_devicesHandler.h"
#include "RESTAPI_deviceHandler.h"
#include "RESTAPI_deviceCommandHandler.h"
#include "RESTAPI_UnknownRequestHandler.h"

namespace uCentral::RESTAPI {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("RESTAPIServer", "RESTAPIServer", "ucentral.restapi")
    {
    }

    int Service::Start() {
        logger_.information("Starting.");

        for(const auto & svr: ConfigurationServers()) {
            std::string l{"Starting: " +
                          svr.address() + ":" + std::to_string(svr.port()) +
                          " key:" + svr.key_file() +
                          " cert:" + svr.cert_file()};

            logger_.information(l);

            SecureServerSocket sock(svr.port(),
                                    64,
                                    new Context(Poco::Net::Context::TLS_SERVER_USE,
                                                svr.key_file(),
                                                svr.cert_file(),
                                                ""));

            auto Params = new HTTPServerParams;

            Params->setMaxThreads(16);
            Params->setMaxQueued(100);

            auto NewServer = std::make_shared<Poco::Net::HTTPServer>(new RequestHandlerFactory, sock, Params);

            NewServer->start();

            RESTServers_.push_back(NewServer);
        }

        return 0;
    }

    HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const HTTPServerRequest & Request) {

        auto    & Logger = uCentral::RESTAPI::Service::instance()->logger();

        Logger.information("Request from "
                            + Request.clientAddress().toString()
                            + ": "
                            + Request.getMethod()
                            + " "
                            + Request.getURI()
                            + " "
                            + Request.getVersion());

        Poco::URI uri(Request.getURI());
        auto *path = uri.getPath().c_str();

        RESTAPIHandler::BindingMap bindings;

        if (RESTAPIHandler::ParseBindings(path, "/api/v1/oauth2", bindings)) {
            return new RESTAPI_oauth2Handler(bindings, Logger);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/oauth2/{token}", bindings)) {
            return new RESTAPI_oauth2Handler(bindings, Logger);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/devices", bindings)) {
            return new RESTAPI_devicesHandler(bindings, Logger);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/device/{serialNumber}/{command}", bindings)) {
            return new RESTAPI_deviceCommandHandler(bindings, Logger);
        } else if (RESTAPIHandler::ParseBindings(path, "/api/v1/device/{serialNumber}", bindings)) {
            return new RESTAPI_deviceHandler(bindings, Logger);
        }

        return new RESTAPI_UnknownRequestHandler(bindings,Logger);
    }

    void Service::Stop() {
        SubSystemServer::logger().information("Stopping ");

        for( const auto & svr : RESTServers_ )
            svr->stop();
    }

};  // namespace