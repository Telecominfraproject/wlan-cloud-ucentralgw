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
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/StreamSocket.h"

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

#include "Poco/Net/ParallelSocketReactor.h"

#include "uDeviceRegistry.h"

#include <mutex>
#include <thread>

namespace uCentral::WebSocket {

    int Start();
    void Stop();

    class CountedSocketReactor : public Poco::Net::SocketReactor {
    public:
        explicit CountedSocketReactor(uint64_t Id): Id_(Id),SocketCount_(0) {
            setTimeout(Poco::Timespan(0,10000));
        }

        ~CountedSocketReactor() override {
            Poco::Net::SocketReactor::stop();
        }

        uint64_t Count() const {
            return SocketCount_; }
        void Get() {
            std::lock_guard<std::mutex> guard(Mutex_);
            SocketCount_++; }
        void Release() {
            std::lock_guard<std::mutex> guard(Mutex_);
            SocketCount_--; }
        uint64_t Id() const { return Id_;}

    private:
        std::mutex       Mutex_;
        uint64_t         SocketCount_;
        uint64_t         Id_;
    };

    class CountedSocketReactorFactory {
    public:
        explicit CountedSocketReactorFactory(Poco::Logger & Logger):
            Logger_(Logger),
            NumReactors_(0)
        {
        }

        void Init(unsigned int NumReactors) {
            NumReactors_ = NumReactors;
            for(auto i=0;i<NumReactors_;i++)
            {
                auto NewReactor = new CountedSocketReactor(i);
                auto NewThread  = new Poco::Thread;

                ReactorThreads_.emplace_back( std::pair(NewReactor, NewThread));
                NewThread->start(*NewReactor);
                NewThread->setName( "Reactor:" + std::to_string(i));
            }
        }

        void Close() {
            Logger_.information("Closing Reactor factory...");
            for(auto &[Reactor,Thread]:ReactorThreads_)
            {
                Reactor->stop();
                Thread->join();
            }
        }

        ~CountedSocketReactorFactory() {
            for(auto &[Reactor,Thread]:ReactorThreads_)
            {
                delete Reactor;
                delete Thread;
            }
        }

        CountedSocketReactor * GetAReactor() {
            uint64_t Min;

            std::lock_guard<std::mutex> guard(Mutex_);

            auto Tmp = ReactorThreads_.end();
            int TotalSockets = 0 ;

            for( auto i = ReactorThreads_.begin() ; i != ReactorThreads_.end() ; i++ )
            {
                TotalSockets += i->first->Count();
                if(Tmp == ReactorThreads_.end()) {
                    Tmp = i;
                    Min = i->first->Count();
                }
                else if(i->first->Count()<Min) {
                    Tmp = i;
                    Min = i->first->Count();
                }
            }

            // std::cout << "Currently " << TotalSockets << " sockets." << std::endl;

            Tmp->first->Get();

            return Tmp->first;
        }

    private:
        std::mutex      Mutex_;
        Poco::Logger    & Logger_;
        uint64_t        NumReactors_;
        std::vector<std::pair<CountedSocketReactor *, Poco::Thread *>>  ReactorThreads_;
    };

    class CountedReactor {
    public:
        explicit CountedReactor();
        ~CountedReactor();
        CountedSocketReactor * Reactor() { return Reactor_; }
    private:
        CountedSocketReactor * Reactor_;
    };

    class WSConnection {
        const int BufSize = 12000;
    public:
        WSConnection(StreamSocket& socket, SocketReactor& reactor);
        ~WSConnection();

        void ProcessJSONRPCEvent(Poco::DynamicStruct ds);
        void ProcessJSONRPCResult(Poco::DynamicStruct ds);
        void ProcessIncomingFrame();
        bool SendCommand(const std::string &Cmd);
        void OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf);
        void OnSocketShutdown(const AutoPtr<Poco::Net::ShutdownNotification>& pNf);
        void OnSocketError(const AutoPtr<Poco::Net::ErrorNotification>& pNf);
        bool LookForUpgrade(std::string &Response);
        static Poco::DynamicStruct ExtractCompressedData(const std::string & CompressedData);
        void Register();
        void DeRegister();
        void StartHandshake();

    private:
        std::mutex                          Mutex_;
        std::shared_ptr<CountedReactor>     Reactor_;
        Poco::Logger                    &   Logger_;
        StreamSocket                        Socket_;
        SocketReactor &                     ParentAcceptorReactor_;
        std::shared_ptr<Poco::Net::WebSocket> WS_;
        std::string                         SerialNumber_;
        uCentral::DeviceRegistry::ConnectionState * Conn_;
        std::map<uint64_t,std::string>      RPCs_;
        uint64_t                            RPC_;
        bool                                Registered_;
        std::shared_ptr<char>               IncomingMessage_;
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
            return Factory_.GetAReactor();
        }

    private:
        static Service *instance_;

        int Start() override;
        void Stop() override;

        std::vector<std::tuple<
                std::shared_ptr<Poco::Thread> ,
                std::shared_ptr<Poco::Net::SocketReactor>,
                std::shared_ptr<Poco::Net::SocketAcceptor<WSConnection>>
        >>      Servers_;
        CountedSocketReactorFactory                             Factory_;
    };



}; //namespace

#endif //UCENTRAL_UCENTRALWEBSOCKETSERVER_H
