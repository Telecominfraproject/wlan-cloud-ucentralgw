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

        void start() {
            Thread_.start( *this);
            }
        uint64_t Count() {

            return SocketCount_; }
        void Get() {
            Poco::Mutex::ScopedLock lock(Mutex_);
            SocketCount_++; }
        void Release() {
            Poco::Mutex::ScopedLock lock(Mutex_);
            SocketCount_--; }
        uint64_t Id() const { return Id_;}

    private:
        Poco::Mutex      Mutex_;
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
                auto NewReactor = new CountedSocketReactor(i);
                Reactors_.push_back( NewReactor );
                NewReactor->start();
            }
        }

        CountedSocketReactor * GetAReactor() {
            uint64_t Min;

            Poco::Mutex::ScopedLock lock(Mutex_);

            auto Tmp = Reactors_.end();
            int TotalSockets = 0 ;

            for( auto i = Reactors_.begin() ; i != Reactors_.end() ; i++ )
            {
                TotalSockets += (*i)->Count();
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

            std::cout << "Currently " << TotalSockets << " sockets." << std::endl;

            return (*Tmp);
        }

    private:
        Poco::Mutex Mutex_;
        uint64_t    NumReactors_;
        std::vector<CountedSocketReactor *>  Reactors_;
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

        CountedSocketReactor * GetAReactor() {
            return Factory_->GetAReactor();
        }

    private:
        int Start() override;
        void Stop() override;

        static Service *instance_;
        std::vector<std::shared_ptr<Poco::Net::HTTPServer>>     HTTPServers_;
        std::shared_ptr<CountedSocketReactorFactory>            Factory_;
    };

    class CountedReactor {
    public:
        explicit CountedReactor()
        {
            Reactor_ = Service::instance()->GetAReactor();
            Reactor_->Get();
            // std::cout << "Got reactor: " << Reactor_->Id() << std::endl;
        };

        ~CountedReactor()
        {
            // std::cout << "Leaving reactor: " << Reactor_->Id() << std::endl;
            Reactor_->Release();
        }

        CountedSocketReactor * Reactor() { return Reactor_; }

    private:
        CountedSocketReactor * Reactor_;
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
        WSRequestHandlerFactory():
            Logger_(Service::instance()->Logger())
        {
        }
        HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override;
    private:
        Poco::Logger    & Logger_;
    };

    class WSConnection {
    public:
        WSConnection(Poco::Logger   & Logger,
                     HTTPServerRequest & Request,
                     HTTPServerResponse & Response );

        ~WSConnection();

        void ProcessJSONRPCEvent(Poco::DynamicStruct ds);
        void ProcessJSONRPCResult(Poco::DynamicStruct ds);
        bool SendCommand(const std::string &Cmd);
        void OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf);
        void OnSocketShutdown(const AutoPtr<Poco::Net::ShutdownNotification>& pNf);
        void OnSocketError(const AutoPtr<Poco::Net::ErrorNotification>& pNf);
        bool LookForUpgrade(std::string &Response);
        static Poco::DynamicStruct ExtractCompressedData(const std::string & CompressedData);
        void Register();
        void DeRegister();

    private:
        Poco::Mutex                     Mutex_;
        CountedReactor                  * Reactor_;
        Poco::Logger                    & Logger_;
        Poco::Net::WebSocket            * WS_;
        std::string                     SerialNumber_;
        std::shared_ptr<uCentral::DeviceRegistry::ConnectionState>  Conn_;
        std::map<uint64_t,std::string>  RPCs_;
        uint64_t                        RPC_;
        bool                            Registered_;
    };

}; //namespace

#endif //UCENTRAL_UCENTRALWEBSOCKETSERVER_H
