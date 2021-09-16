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
#include "Poco/Net/ParallelSocketAcceptor.h"
#include "Poco/Environment.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServer.h"

namespace OpenWifi {

	class ReactorPool {
	  public:
		ReactorPool( unsigned int NumberOfThreads = Poco::Environment::processorCount() )
			: NumberOfThreads_(NumberOfThreads)
		{
		}

		void Start() {
			for(auto i=0;i<NumberOfThreads_;++i) {
				auto NewReactor = std::make_unique<Poco::Net::SocketReactor>();
				auto NewThread = std::make_unique<Poco::Thread>();
				NewThread->start(*NewReactor);
				Reactors_.emplace_back( std::move(NewReactor));
				Threads_.emplace_back( std::move(NewThread));
			}
		}

		void Stop() {
			for(auto &i:Reactors_)
				i->stop();
			for(auto &i:Threads_) {
				i->join();
			}
		}

		Poco::Net::SocketReactor & NextReactor() {
			NextReactor_ ++;
			NextReactor_ %= NumberOfThreads_;
			return *Reactors_[NextReactor_];
		}

	  private:
		unsigned int NumberOfThreads_;
		unsigned int NextReactor_=0;
		std::vector<std::unique_ptr<Poco::Net::SocketReactor>> 	Reactors_;
		std::vector<std::unique_ptr<Poco::Thread>>				Threads_;
	};

	class WSConnection {
        static constexpr int BufSize = 64000;
    public:
		WSConnection(Poco::SharedPtr<Poco::Net::WebSocket> WS, Poco::Net::SocketReactor& Reactor, Poco::Logger &Logger);
        ~WSConnection();

        void ProcessJSONRPCEvent(Poco::JSON::Object::Ptr & Doc);
        void ProcessJSONRPCResult(Poco::JSON::Object::Ptr &	Doc);
        void ProcessIncomingFrame();
		bool Send(const std::string &Payload);
        void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
        void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
        void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);
        bool LookForUpgrade(uint64_t UUID);
		static bool ExtractCompressedData(const std::string & CompressedData, std::string & UnCompressedData);
		void LogException(const Poco::Exception &E);
		[[nodiscard]] GWObjects::CertificateValidation CertificateValidation() const { return CertValidation_; };
    private:
		std::recursive_mutex                Mutex_;
		Poco::SharedPtr<Poco::Net::WebSocket> WS_;
		Poco::Net::SocketReactor			& Reactor_;
        Poco::Logger                    	&Logger_;
        Poco::Net::StreamSocket       		Socket_;
        std::string                         SerialNumber_;
		std::string 						Compatible_;
		GWObjects::ConnectionState 	* Conn_ = nullptr;
        bool                                Registered_ = false ;
		std::string 						CId_;
		std::string							CN_;
		GWObjects::CertificateValidation	CertValidation_ = GWObjects::CertificateValidation::NO_CERTIFICATE;
		uint64_t 							Errors_=0;
		std::unique_ptr<StateProcessor>		StatsProcessor_;

		void CompleteStartup();
    };

	class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		explicit WebSocketRequestHandler(ReactorPool &Pool, Poco::Logger &Logger) :
 			Pool_(Pool), Logger_(Logger) {}

		void handleRequest(Poco::Net::HTTPServerRequest & Request, Poco::Net::HTTPServerResponse & Response) final {
			try {
				std::cout << __LINE__ << std::endl;
				auto WS = Poco::SharedPtr<Poco::Net::WebSocket>(new Poco::Net::WebSocket(Request, Response));
				std::cout << __LINE__ << std::endl;
				new WSConnection(WS, Pool_.NextReactor(), Logger_);
				std::cout << __LINE__ << std::endl;

			} catch (const Poco::Exception &E) {
				std::cout << E.what() << " " << E.name() << " " << E.displayText() << std::endl;
			} catch (...) {
				std::cout << __LINE__ << std::endl;
			}
		}
	  private:
		ReactorPool		&Pool_;
		Poco::Logger	&Logger_;
	};


	class WebSocketRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		explicit WebSocketRequestHandlerFactory(ReactorPool &  Pool, Poco::Logger & Logger) :
 			Pool_(Pool),
			Logger_(Logger)
		{}

		inline Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest & Request) final {
			std::cout << __LINE__ << std::endl;
			return new WebSocketRequestHandler(Pool_,Logger_);
		}

	  private:
	Poco::Logger 			& Logger_;
		ReactorPool			& Pool_;
	};


    class WebSocketServer : public SubSystemServer {
    public:
        static WebSocketServer *instance() {
            if (instance_ == nullptr) {
                instance_ = new WebSocketServer;
            }
            return instance_;
        }

		int Start() override;
		void Stop() override;
		bool IsCertOk() { return IssuerCert_!= nullptr; }
		const Poco::Crypto::X509Certificate & Certificate() const { return *IssuerCert_; }
		bool ValidateCertificate(const std::string & ConnectionId, const Poco::Crypto::X509Certificate & Certificate);

    private:
        static WebSocketServer 								*instance_;
		std::unique_ptr<Poco::Crypto::X509Certificate>		IssuerCert_;
		ReactorPool											ReactorPool_;
		std::vector<std::unique_ptr<Poco::Net::HTTPServer>> WebServers_;
		Poco::ThreadPool									Pool_;

		WebSocketServer() noexcept: SubSystemServer("WebSocketServer", "WS-SVR", "nano")
			{ }
    };

	inline WebSocketServer * WebSocketServer() { return WebSocketServer::instance(); }

} //namespace

#endif //UCENTRAL_UCENTRALWEBSOCKETSERVER_H
