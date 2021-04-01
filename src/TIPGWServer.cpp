//
// Created by stephane bourque on 2021-02-12.
//
#include "TIPGWServer.h"

#include "Poco/Net/HTTPServer.h"

namespace uCentral::TIPGW {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("TIPGWServer", "TIPGWServer", "tip.gateway") {
    }

    int Start() {
        return uCentral::TIPGW::Service::instance()->Start();
    }

    void Stop() {
        uCentral::TIPGW::Service::instance()->Stop();
    }

    int Service::Start() {
        Logger_.information("Starting.");

        for (const auto &Svr: ConfigServersList_) {
			Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.address(), std::to_string(Svr.port()),
											 Svr.key_file(),Svr.cert_file()));

            auto Sock{Svr.CreateSecureSocket()};

            auto Params = new Poco::Net::HTTPServerParams;
            Params->setMaxThreads(16);
            Params->setMaxQueued(100);

            auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new RequestHandlerFactory, Sock, Params);
            NewServer->start();
            RESTServers_.push_back(std::move(NewServer));
        }

        return 0;
    }

    void Service::Stop() {
        Logger_.information("Stopping ");
        for(auto const & svr : RESTServers_)
            svr->stop();
    }

    void RequestHandler::handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {

    }

    Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &request) {

        Logger_.information(Poco::format("%s from %s: %s",request.getMethod(),
                                        request.clientAddress().toString(),
                                        request.getURI()));
        return nullptr;
    }


};  // namespace