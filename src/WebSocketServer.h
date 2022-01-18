//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <mutex>
#include <thread>
#include <array>
#include <ctime>

#include "DeviceRegistry.h"
#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/MicroService.h"

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
				NewThread->setName("DeviceWebSocketReactor_" + std::to_string(i));
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
		WSConnection(Poco::Net::StreamSocket& Socket, Poco::Net::SocketReactor& Reactor);
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
		inline Poco::Logger & Logger() { return Logger_; }
		bool SetWebSocketTelemetryReporting(uint64_t interval, uint64_t TelemetryWebSocketTimer);
		bool SetKafkaTelemetryReporting(uint64_t interval, uint64_t TelemetryKafkaTimer);
		bool StopWebSocketTelemetry();
		bool StopKafkaTelemetry();
		inline bool GetTelemetryParameters(bool & Reporting, uint64_t & Interval,
										   uint64_t & WebSocketTimer, uint64_t & KafkaTimer,
										   uint64_t &WebSocketCount, uint64_t & KafkaCount,
										   uint64_t &WebSocketPackets,
										   uint64_t &KafkaPackets ) const {
			Reporting = TelemetryReporting_;
			WebSocketTimer = TelemetryWebSocketTimer_;
			KafkaTimer = TelemetryKafkaTimer_;
			WebSocketCount = TelemetryWebSocketRefCount_;
			KafkaCount = TelemetryKafkaRefCount_;
			Interval = TelemetryInterval_;
			WebSocketPackets = TelemetryWebSocketPackets_;
			KafkaPackets = TelemetryKafkaPackets_;
			return true;
		}

	  private:
		std::recursive_mutex                Mutex_;
		Poco::Logger                    	&Logger_;
		Poco::Net::StreamSocket       		Socket_;
		Poco::Net::SocketReactor			& Reactor_;
		std::unique_ptr<Poco::Net::WebSocket> WS_;
		std::string                         SerialNumber_;
		std::string 						Compatible_;
		std::shared_ptr<DeviceRegistry::ConnectionEntry> 	Conn_;
		bool                                Registered_ = false ;
		std::string 						CId_;
		std::string							CN_;
		GWObjects::CertificateValidation	CertValidation_ = GWObjects::CertificateValidation::NO_CERTIFICATE;
		uint64_t 							Errors_=0;
		bool 								Connected_=false;
		uint64_t 							ConnectionId_=0;
		Poco::Net::IPAddress				PeerAddress_;
		mutable std::atomic_bool 			TelemetryReporting_ = false;
		mutable uint64_t 					TelemetryWebSocketRefCount_ = 0;
		mutable uint64_t 					TelemetryKafkaRefCount_ = 0;
		mutable uint64_t 					TelemetryWebSocketTimer_ = 0;
		mutable uint64_t 					TelemetryKafkaTimer_ = 0 ;
		mutable uint64_t 					TelemetryInterval_ = 0;
		mutable uint64_t 					TelemetryWebSocketPackets_=0;
		mutable uint64_t 					TelemetryKafkaPackets_=0;

		void CompleteStartup();
		bool StartTelemetry();
		bool StopTelemetry();
		void UpdateCounts();
	};

	class WebSocketServer : public SubSystemServer {
	  public:
		static auto instance() {
		    static auto instance_ = new WebSocketServer;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		bool IsCertOk() { return IssuerCert_!= nullptr; }
		const Poco::Crypto::X509Certificate & Certificate() const { return *IssuerCert_; }
		bool ValidateCertificate(const std::string & ConnectionId, const Poco::Crypto::X509Certificate & Certificate);
		Poco::Net::SocketReactor & GetNextReactor() { return ReactorPool_.NextReactor(); }

		inline bool IsSimSerialNumber(const std::string & SerialNumber) const {
			return IsSim(SerialNumber) && SerialNumber == SimulatorId_;
		}

		inline static bool IsSim(const std::string & SerialNumber) {
			return SerialNumber.substr(0,6) == "53494d";
		}

		inline bool IsSimEnabled() const {
			return SimulatorEnabled_;
		}

		inline bool UseProvisioning() const { return LookAtProvisioning_; }
		inline bool UseDefaults() const { return UseDefaultConfig_; }

	  private:
		std::unique_ptr<Poco::Crypto::X509Certificate>	IssuerCert_;
		std::vector<std::unique_ptr<Poco::Net::ParallelSocketAcceptor<WSConnection, Poco::Net::SocketReactor>>>	Acceptors_;
		Poco::Net::SocketReactor		Reactor_;
		Poco::Thread					ReactorThread_;
		ReactorPool						ReactorPool_;
		std::string 					SimulatorId_;
		bool 							LookAtProvisioning_ = false;
		bool 							UseDefaultConfig_ = true;
		bool 							SimulatorEnabled_=false;

		WebSocketServer() noexcept:
		    SubSystemServer("WebSocketServer", "WS-SVR", "ucentral.websocket") {

		}
	};

	inline auto WebSocketServer() { return WebSocketServer::instance(); }

} //namespace
