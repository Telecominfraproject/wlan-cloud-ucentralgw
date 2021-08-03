//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UCENTRALWEBSOCKETSERVER_H
#define UCENTRAL_UCENTRALWEBSOCKETSERVER_H

#include <mutex>
#include <thread>
#include <array>
#include <ctime>

#include "DeviceRegistry.h"
#include "RESTAPI_GWobjects.h"
#include "StateProcessor.h"
#include "SubSystemServer.h"

#include "Poco/AutoPtr.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/JSON/Parser.h"
#include "Poco/DynamicAny.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/NObserver.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SecureStreamSocketImpl.h"

namespace uCentral {

    struct ReactorPoolEntry {
    	uint64_t 					NumSockets_=0;
		uint64_t 					Id_=0;
    	Poco::Thread				Thread_;
    	Poco::Net::SocketReactor	Reactor_;
    };

    class ReactorPool {
    public:
    	explicit ReactorPool(Poco::Logger & Logger):
            Logger_(Logger)
        {
        }

        void Init(unsigned int NumReactors) {
            NumReactors_ = NumReactors;
            std::cout << __LINE__ << std::endl;
			ReactorPool_.reserve(NumReactors_);
			std::cout << __LINE__ << std::endl;
            for(auto i=0;i<NumReactors_;i++)
            {
            	std::cout << __LINE__ << std::endl;
				auto E = std::make_unique<ReactorPoolEntry>();
				std::cout << __LINE__ << std::endl;
				std::cout << __LINE__ << std::endl;
				E->Id_=i;
				std::cout << __LINE__ << std::endl;
				E->Thread_.setName( "WebSocketReactor:" + std::to_string(i));
				std::cout << __LINE__ << std::endl;
				E->Thread_.start(E->Reactor_);
				std::cout << __LINE__ << std::endl;
                ReactorPool_.emplace_back( std::move(E) );
                std::cout << __LINE__ << std::endl;
            }
        }

        void Close() {
            Logger_.information("Closing Reactor factory...");
            for(auto &i:ReactorPool_)
            {
                i->Reactor_.stop();
                i->Thread_.join();
            }
			ReactorPool_.clear();
        }

        ~ReactorPool() {
			Close();
        }

        ReactorPoolEntry* GetAReactor() {
            std::lock_guard G(Mutex_);

            uint64_t Min;

            ReactorPoolEntry *Tmp = nullptr;
            for( auto &i : ReactorPool_)
            {
                TotalSockets_ += i->NumSockets_;
                if((Tmp == nullptr) || (i->NumSockets_<Min) ) {
                    Tmp = i.get();
                    Min = i->NumSockets_;
                }
            }
			Tmp->NumSockets_++;
            std::cout << "Reactor: " << Tmp->Id_ << "   Count: " << Tmp->NumSockets_ << "  TotalSockets: " << TotalSockets_ << std::endl;
            return Tmp;
        }

		void Release(ReactorPoolEntry *RE) {
    		std::lock_guard G(Mutex_);
			ReactorPool_[RE->Id_]->NumSockets_--;
		}

    private:
        std::mutex     									Mutex_;
        Poco::Logger    								& Logger_;
        uint64_t        								NumReactors_=0;
        uint64_t 										TotalSockets_ = 0 ;
		std::vector<std::unique_ptr<ReactorPoolEntry>>  ReactorPool_;
    };

	class WSConnection {
        static constexpr int BufSize = 64000;
    public:
        WSConnection(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor);
        ~WSConnection();

        void ProcessJSONRPCEvent(Poco::JSON::Object::Ptr	Doc);
        void ProcessJSONRPCResult(Poco::JSON::Object::Ptr	Doc);
        void ProcessIncomingFrame();
		bool Send(const std::string &Payload);
        void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
        void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
        void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);
        bool LookForUpgrade(uint64_t UUID);
		static bool ExtractCompressedData(const std::string & CompressedData, std::string & UnCompressedData);
        void Register();
        void DeRegister();
		void LogException(const Poco::Exception &E);
		[[nodiscard]] GWObjects::CertificateValidation CertificateValidation() const { return CertValidation_; };
    private:
		SubMutex                          	Mutex_;
		ReactorPoolEntry                    *Reactor_= nullptr;
        Poco::Logger                    &   Logger_;
        Poco::Net::StreamSocket       		Socket_;
        std::unique_ptr<Poco::Net::WebSocket> WS_;
        std::string                         SerialNumber_;
		std::string 						Compatible_;
		GWObjects::ConnectionState 	* Conn_ = nullptr;
        bool                                Registered_ = false ;
		std::string 						CId_;
		std::string							CN_;
		GWObjects::CertificateValidation	CertValidation_ = GWObjects::CertificateValidation::NO_CERTIFICATE;
		uint64_t 							Errors_=0;
		std::unique_ptr<uCentral::StateProcessor>	StatsProcessor_;
    };

    struct WebSocketServerEntry {
        std::unique_ptr<Poco::Net::SocketReactor>                   SocketReactor;
        std::unique_ptr<Poco::Net::SocketAcceptor<WSConnection>>    SocketAcceptor;
        std::unique_ptr<Poco::Thread>                               SocketReactorThread;
    };

    class WebSocketServer : public SubSystemServer {
    public:
        static WebSocketServer *instance() {
            if (instance_ == nullptr) {
                instance_ = new WebSocketServer;
            }
            return instance_;
        }

        ReactorPoolEntry* GetAReactor() {
            return Reactors_.GetAReactor();
        }

        void ReleaseAReactor(ReactorPoolEntry * E) {
        	return Reactors_.Release(E);
        }


		int Start() override;
		void Stop() override;
		bool IsCertOk() { return IssuerCert_!= nullptr; }
		const Poco::Crypto::X509Certificate & Certificate() const { return *IssuerCert_; }
		bool ValidateCertificate(const std::string & ConnectionId, const Poco::Crypto::X509Certificate & Certificate);

    private:
        static WebSocketServer *instance_;
		std::unique_ptr<Poco::Crypto::X509Certificate>	IssuerCert_;
		std::vector<WebSocketServerEntry>      			Servers_;
		ReactorPool				            			Reactors_;
		WebSocketServer() noexcept;
    };

	inline WebSocketServer * WebSocketServer() { return WebSocketServer::instance(); }

} //namespace

#endif //UCENTRAL_UCENTRALWEBSOCKETSERVER_H
