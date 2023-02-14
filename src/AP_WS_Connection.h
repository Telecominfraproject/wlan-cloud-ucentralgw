//
// Created by stephane bourque on 2022-02-03.
//

#pragma once

#include <string>
#include <shared_mutex>

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Logger.h"
#include "Poco/Net/WebSocket.h"

#include "RESTObjects/RESTAPI_GWobjects.h"

namespace OpenWifi {

	class AP_WS_Connection {
		static constexpr int BufSize = 256000;
	  public:
		explicit AP_WS_Connection(	Poco::Net::HTTPServerRequest &request,
									Poco::Net::HTTPServerResponse &response,
									uint64_t connection_id,
									Poco::Logger &L,
									Poco::Net::SocketReactor &R);
		~AP_WS_Connection();

		void EndConnection();
		void ProcessJSONRPCEvent(Poco::JSON::Object::Ptr & Doc);
		void ProcessJSONRPCResult(Poco::JSON::Object::Ptr Doc);
		void ProcessIncomingFrame();
		void ProcessIncomingRadiusData(const Poco::JSON::Object::Ptr &Doc);

		[[nodiscard]] bool Send(const std::string &Payload);

		bool SendRadiusAuthenticationData(const unsigned char * buffer, std::size_t size);
		bool SendRadiusAccountingData(const unsigned char * buffer, std::size_t size);
		bool SendRadiusCoAData(const unsigned char * buffer, std::size_t size);

		void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
		void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);
		bool LookForUpgrade(uint64_t UUID, uint64_t & UpgradedUUID);
		static bool ExtractBase64CompressedData(const std::string & CompressedData, std::string & UnCompressedData, uint64_t compress_sz);
		void LogException(const Poco::Exception &E);
		inline Poco::Logger & Logger() { return Logger_; }
		bool SetWebSocketTelemetryReporting(uint64_t RPCID, uint64_t interval, uint64_t TelemetryWebSocketTimer, const std::vector<std::string> & TelemetryTypes);
		bool SetKafkaTelemetryReporting(uint64_t RPCID, uint64_t interval, uint64_t TelemetryKafkaTimer, const std::vector<std::string> & TelemetryTypes);
		bool StopWebSocketTelemetry(uint64_t RPCID);
		bool StopKafkaTelemetry(uint64_t RPCID);

		inline void GetLastStats(std::string &LastStats) const {
			std::shared_lock	G(ConnectionMutex_);
			LastStats = RawLastStats_;
		}

		inline void SetLastStats(const std::string &LastStats) {
			std::unique_lock	G(ConnectionMutex_);
			RawLastStats_ = LastStats;
		}

		inline void SetLastHealthCheck(const GWObjects::HealthCheck &H) {
			std::unique_lock	G(ConnectionMutex_);
			RawLastHealthcheck_ = H;
		}

		inline void GetLastHealthCheck(GWObjects::HealthCheck &H) {
			std::shared_lock	G(ConnectionMutex_);
			H = RawLastHealthcheck_;
		}

		inline void GetState(GWObjects::ConnectionState &State) const {
			std::shared_lock	G(ConnectionMutex_);
			State = State_;
		}

		inline void GetRestrictions(GWObjects::DeviceRestrictions & R) const {
			std::shared_lock	G(ConnectionMutex_);
			R = Restrictions_;
		}

		void Process_connect(Poco::JSON::Object::Ptr ParamsObj, const std::string &Serial);
		void Process_state(Poco::JSON::Object::Ptr ParamsObj);
		void Process_healthcheck(Poco::JSON::Object::Ptr ParamsObj);
		void Process_log(Poco::JSON::Object::Ptr ParamsObj);
		void Process_crashlog(Poco::JSON::Object::Ptr ParamsObj);
		void Process_ping(Poco::JSON::Object::Ptr ParamsObj);
		void Process_cfgpending(Poco::JSON::Object::Ptr ParamsObj);
		void Process_recovery(Poco::JSON::Object::Ptr ParamsObj);
		void Process_deviceupdate(Poco::JSON::Object::Ptr ParamsObj, std::string &Serial);
		void Process_telemetry(Poco::JSON::Object::Ptr ParamsObj);
		void Process_venuebroadcast(Poco::JSON::Object::Ptr ParamsObj);
		void Process_event(Poco::JSON::Object::Ptr ParamsObj);
		void Process_wifiscan(Poco::JSON::Object::Ptr ParamsObj);
		void Process_alarm(Poco::JSON::Object::Ptr ParamsObj);

		bool ValidatedDevice();

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

		friend class AP_WS_Server;

        inline GWObjects::DeviceRestrictions Restrictions() const {
			std::shared_lock	G(ConnectionMutex_);
			return Restrictions_;
		}

	  private:
		mutable std::shared_mutex	 		ConnectionMutex_;
		std::shared_mutex					TelemetryMutex_;
		Poco::Logger                    	&Logger_;
		Poco::Net::SocketReactor			&Reactor_;
		std::unique_ptr<Poco::Net::WebSocket> WS_;
		std::string                         SerialNumber_;
		uint64_t 							SerialNumberInt_=0;
		std::string 						Compatible_;
		std::atomic_bool                    Registered_ = false ;
		std::string 						CId_;
		std::string							CN_;
		uint64_t 							Errors_=0;
		Poco::Net::IPAddress				PeerAddress_;
		volatile bool 						TelemetryReporting_ = false;
		volatile uint64_t				    TelemetryWebSocketRefCount_ = 0;
		volatile uint64_t				    TelemetryKafkaRefCount_ = 0;
		volatile uint64_t 				    TelemetryWebSocketTimer_ = 0;
		volatile uint64_t 				    TelemetryKafkaTimer_ = 0 ;
		volatile uint64_t 				    TelemetryInterval_ = 0;
		volatile uint64_t				    TelemetryWebSocketPackets_=0;
		volatile uint64_t				    TelemetryKafkaPackets_=0;
		GWObjects::ConnectionState			State_;
		std::string        					RawLastStats_;
		GWObjects::HealthCheck				RawLastHealthcheck_;
		std::chrono::time_point<std::chrono::high_resolution_clock> ConnectionStart_ = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> ConnectionCompletionTime_{0.0};
		std::atomic_flag 					Dead_=false;
		std::atomic_bool 					DeviceValidated_=false;
		std::atomic_bool 					Valid_=false;
        OpenWifi::GWObjects::DeviceRestrictions       Restrictions_;

		static inline std::atomic_uint64_t 	ConcurrentStartingDevices_=0;

		bool StartTelemetry(uint64_t RPCID, const std::vector<std::string> & TelemetryTypes);
		bool StopTelemetry(uint64_t RPCID);
		void UpdateCounts();
	};

}