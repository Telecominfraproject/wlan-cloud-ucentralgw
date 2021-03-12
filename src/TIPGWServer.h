//
// Created by stephane bourque on 2021-02-12.
//

#ifndef UCENTRAL_TIPGWSERVER_H
#define UCENTRAL_TIPGWSERVER_H

#include "SubSystemServer.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPRequestHandler.h"

namespace uCentral::TIPGW {

    int Start();
    void Stop();

    class Service : public SubSystemServer {
    public:
        Service() noexcept;

        friend int Start();
        friend void Stop();

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

    private:
        int Start() override;
        void Stop() override;
        std::vector<std::shared_ptr<Poco::Net::HTTPServer>>     RESTServers_;
        static Service *instance_;
    };

    class RequestHandler : public Poco::Net::HTTPRequestHandler
        /// Handle a WebSocket connection.
    {
    public:
        explicit RequestHandler(Poco::Logger &logger)
                : Logger_(logger)
        { };
        void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) override;
    private:
        Poco::Logger                & Logger_;
    };

    class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        explicit RequestHandlerFactory()
        {};
        Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
    private:

    };

};  // Namespace

#endif //UCENTRAL_TIPGWSERVER_H
