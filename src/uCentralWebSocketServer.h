//
// Created by stephane bourque on 2021-02-28.
//

#ifndef UCENTRAL_UCENTRALWEBSOCKETSERVER_H
#define UCENTRAL_UCENTRALWEBSOCKETSERVER_H

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
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Observer.h"
#include "Poco/NObserver.h"

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

namespace uCentral::WebSocket {

    struct ConnectionState {
        uint64_t messages;
        std::string SerialNumber;
        std::string Address;
        uint64_t CfgUUID;
        uint64_t TX, RX;
    };

    class Service : public SubSystemServer {
    public:
        Service() noexcept;

        int start() override;
        void stop() override;

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

    private:
        static Service *instance_;
        std::vector<std::shared_ptr<Poco::Net::HTTPServer>>   HTTPServers_;
        Poco::Net::SocketReactor    SocketReactor_;
        Poco::Thread                SocketReactorThread_;
    };

    class WSRequestHandler : public HTTPRequestHandler
        /// Handle a WebSocket connection.
    {
    public:
        explicit WSRequestHandler(Poco::Logger &logger,
                                         Poco::Net::SocketReactor   &Reactor)
        : Logger_(logger),
        Reactor_(Reactor)
        { };
        void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override;
        void process_message(char *IncomingMessage, std::string &Response, ConnectionState &Connection);
    private:
        Poco::Logger                & Logger_;
        Poco::Net::SocketReactor    & Reactor_;
    };

    class WSRequestHandlerFactory : public HTTPRequestHandlerFactory {
    public:
        explicit WSRequestHandlerFactory(Poco::Net::SocketReactor & SocketReactor) :
        SocketReactor_(SocketReactor)
        {};
        HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override;
    private:
        Poco::Net::SocketReactor    & SocketReactor_;
    };

    class WSConnection {
    public:
        WSConnection(Poco::Net::SocketReactor &SocketReactor,
                     Poco::Logger   & Logger,
                     HTTPServerRequest & Request,
                     HTTPServerResponse & Response ):
            SocketReactor_(SocketReactor),
            Logger_(Logger),
            Request_(Request),
            Response_(Response),
            WS_(Request,Response)

        {
            Conn.Address = WS_.peerAddress().toString();
            WS_.setReceiveTimeout(Poco::Timespan());
            WS_.setNoDelay(true);
            WS_.setKeepAlive(true);
        }

        ~WSConnection() {
            SocketReactor_.removeEventHandler(WS_,Poco::NObserver<WSConnection,Poco::Net::ReadableNotification>(*this,&WSConnection::onSocketReadable));
            SocketReactor_.removeEventHandler(WS_,Poco::NObserver<WSConnection,Poco::Net::ShutdownNotification>(*this,&WSConnection::onSocketShutdown));
            WS_.shutdown();
        }

        void process_message(char *IncomingMessage, std::string &Response, ConnectionState &Connection);
        void onSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf);
        void onSocketShutdown(const AutoPtr<Poco::Net::ShutdownNotification>& pNf) { delete this; };

        Poco::Net::WebSocket    & WS() { return WS_;};

    private:
        Poco::Net::SocketReactor    & SocketReactor_;
        Poco::Logger                & Logger_;
        HTTPServerRequest           & Request_;
        HTTPServerResponse          & Response_;
        Poco::Net::WebSocket        WS_;
        ConnectionState             Conn;
    };

}; //namespace

#endif //UCENTRAL_UCENTRALWEBSOCKETSERVER_H
