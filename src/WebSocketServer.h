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

namespace uCentral {

	class WSConnection {
        static constexpr int BufSize = 64000;
    public:
        WSConnection(Poco::Net::StreamSocket& Socket, Poco::Net::SocketReactor& Reactor);
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
        Poco::Logger                    	&Logger_;
        Poco::Net::StreamSocket       		Socket_;
		Poco::Net::SocketReactor			& Reactor_;
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
        static WebSocketServer *instance_;
		std::unique_ptr<Poco::Crypto::X509Certificate>	IssuerCert_;
		std::vector<std::unique_ptr<Poco::Net::ParallelSocketAcceptor<WSConnection, Poco::Net::SocketReactor>>>	Acceptors_;
		Poco::Net::SocketReactor						Reactor_;
		WebSocketServer() noexcept;
    };

	inline WebSocketServer * WebSocketServer() { return WebSocketServer::instance(); }

} //namespace

#endif //UCENTRAL_UCENTRALWEBSOCKETSERVER_H
