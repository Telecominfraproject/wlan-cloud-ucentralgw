//
// Created by stephane bourque on 2021-02-28.
//

#ifndef UCENTRAL_UCENTRALRESTAPISERVER_H
#define UCENTRAL_UCENTRALRESTAPISERVER_H

#include "SubSystemServer.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/Context.h"
#include "Poco/JSON/Parser.h"
#include "Poco/DynamicAny.h"

using Poco::Net::ServerSocket;
using Poco::Net::SecureServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::Context;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::JSON::Parser;

namespace uCentral::RESTAPI {
    class Service : public SubSystemServer {

    public:
        Service() noexcept;

        int Start() override;
        void Stop() override;

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

    private:
        static Service *instance_;
        std::vector<std::shared_ptr<Poco::Net::HTTPServer>>   RESTServers_;
    };

    class RequestHandlerFactory : public HTTPRequestHandlerFactory {
    public:
        HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override;
    };


}; //   namespace



#endif //UCENTRAL_UCENTRALRESTAPISERVER_H
