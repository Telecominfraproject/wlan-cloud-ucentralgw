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

#include "uDeviceRegistry.h"

namespace uCentral::WebSocket {

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
            WS_(Request,Response),
            IncomingMessage_{0}
        {
            Conn_.Address = WS_.peerAddress().toString();
            Conn_.SerialNumber = "";
            Conn_.UUID = 0 ;
            Conn_.MessageCount = 0 ;
            Conn_.TX = 0 ;
            Conn_.RX = 0 ;
            WS_.setReceiveTimeout(Poco::Timespan());
            WS_.setNoDelay(true);
            WS_.setKeepAlive(true);
        }

        ~WSConnection();

        void ProcessMessage(std::string &Response);
        bool SendCommand(const std::string &Cmd);

        void OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf);
        void OnSocketShutdown(const AutoPtr<Poco::Net::ShutdownNotification>& pNf) { delete this; };


        Poco::Net::WebSocket    & WS() { return WS_;};

    private:
        std::mutex mutex_;
        Poco::Net::SocketReactor                & SocketReactor_;
        Poco::Logger                            & Logger_;
        Poco::Net::WebSocket                    WS_;
        uCentral::DeviceRegistry::ConnectionState Conn_;
        char                                    IncomingMessage_[32000];

    };

}; //namespace

#endif //UCENTRAL_UCENTRALWEBSOCKETSERVER_H
