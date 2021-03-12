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

    int Start();
    void Stop();

    class CountedSocketReactor : public Poco::Net::SocketReactor {
    public:
        explicit CountedSocketReactor(uint64_t Id): Id_(Id),SocketCount_(0) {
        }

        ~CountedSocketReactor() {
            Poco::Net::SocketReactor::stop();
            Thread_.join();
        }

        void start() { Thread_.start( *this); }
        uint64_t Count() const { return SocketCount_; }
        void Get() { SocketCount_++; }
        void Release() { SocketCount_--; }
        uint64_t Id() const { return Id_;}

    private:
        Poco::Thread                Thread_;
        uint64_t                    SocketCount_;
        uint64_t                    Id_;
    };

    class CountedSocketReactorFactory {
    public:
        explicit CountedSocketReactorFactory(uint64_t NumReactors):
                NumReactors_(NumReactors)
        {
            for(auto i=0;i<NumReactors_;i++)
            {
                auto NewReactor = std::shared_ptr<CountedSocketReactor>(new CountedSocketReactor(i));
                Reactors_.push_back( NewReactor );
                NewReactor->start();
            }
        }

        std::shared_ptr<CountedSocketReactor> GetAReactor() {
            uint64_t Min;

            std::lock_guard<std::mutex> guard(Mutex_);

            auto Tmp = Reactors_.end();

            for( auto i = Reactors_.begin() ; i != Reactors_.end() ; i++ )
            {
                if(Tmp == Reactors_.end())
                {
                    Tmp = i;
                    Min = (*i)->Count();
                }
                else if((*i)->Count()<Min)
                {
                    Tmp = i;
                    Min = (*i)->Count();
                }
            }

            // std::cout << "Rector: " << (*Tmp)->Id() << " with " << (*Tmp)->Count() << " sockets." << std::endl;

            return (*Tmp);
        }

    private:
        std::mutex  Mutex_;
        uint64_t    NumReactors_;
        std::vector<std::shared_ptr<CountedSocketReactor>>  Reactors_;
    };

    class CountedReactor {
    public:
        explicit CountedReactor( std::shared_ptr<CountedSocketReactor> R )
        :   Reactor_(R){
            Reactor_->Get();
        };

        ~CountedReactor()
        {
            Reactor_->Release();
        }

        std::shared_ptr<CountedSocketReactor> Reactor() { return Reactor_; }

    private:
        std::shared_ptr<CountedSocketReactor> Reactor_;
    };


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

        std::shared_ptr<CountedSocketReactor> GetAReactor() {
            return Factory_->GetAReactor();
        }

    private:
        int Start() override;
        void Stop() override;

        static Service *instance_;
        std::vector<std::shared_ptr<Poco::Net::HTTPServer>>     HTTPServers_;
        std::shared_ptr<CountedSocketReactorFactory>            Factory_;
    };

    class WSRequestHandler : public HTTPRequestHandler
        /// Handle a WebSocket connection.
    {
    public:
        explicit WSRequestHandler(Poco::Logger &logger)
        : Logger_(logger)
        { };
        void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override;
    private:
        Poco::Logger                & Logger_;
    };

    class WSRequestHandlerFactory : public HTTPRequestHandlerFactory {
    public:
        explicit WSRequestHandlerFactory()
        {};
        HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override;
    private:

    };

    class WSConnection {
    public:
        WSConnection(Poco::Logger   & Logger,
                     HTTPServerRequest & Request,
                     HTTPServerResponse & Response ):
            Logger_(Logger),
            WS_(Request,Response),
            Conn_(nullptr),
            RPC_(0)
        {
            WS_.setReceiveTimeout(Poco::Timespan());
            WS_.setNoDelay(true);
            WS_.setKeepAlive(true);
            Reactor_ = std::shared_ptr<CountedReactor>(new CountedReactor(Service::instance()->GetAReactor()));
            Reactor_->Reactor()->addEventHandler(WS_,
                                     Poco::NObserver<WSConnection,Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
            Reactor_->Reactor()->addEventHandler(WS_,
                                     Poco::NObserver<WSConnection,Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
        }

        ~WSConnection();

        void ProcessLegacyMessage(Poco::DynamicStruct &ds,std::string &Response);
        void ProcessJSONRPCMessage(Poco::DynamicStruct &ds,std::string &Response);
        bool SendCommand(const std::string &Cmd);
        void OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf);
        void OnSocketShutdown(const AutoPtr<Poco::Net::ShutdownNotification>& pNf) { delete this; };
        bool LookForUpgrade(std::string &Response);

        Poco::Net::WebSocket    & WS() { return WS_;};

    private:
        std::mutex mutex_;
        std::shared_ptr<CountedReactor> Reactor_;
        Poco::Logger                    & Logger_;
        Poco::Net::WebSocket            WS_;
        std::string                     SerialNumber_;
        std::shared_ptr<uCentral::DeviceRegistry::ConnectionState>  Conn_;
        uint64_t                        RPC_;
    };

}; //namespace

#endif //UCENTRAL_UCENTRALWEBSOCKETSERVER_H
