//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <array>
#include <ctime>
#include <mutex>
#include <thread>

#include "Poco/AutoPtr.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/ParallelSocketAcceptor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Timer.h"

#include "AP_WS_Connection.h"
#include "AP_WS_ReactorPool.h"

#include "framework/SubSystemServer.h"
#include "framework/utils.h"

namespace OpenWifi {

	class AP_WS_RequestHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		explicit AP_WS_RequestHandler(Poco::Logger &L, uint64_t id) : Logger_(L), id_(id){};

		void handleRequest(Poco::Net::HTTPServerRequest &request,
						   Poco::Net::HTTPServerResponse &response) override;

	  private:
		Poco::Logger &Logger_;
		uint64_t id_ = 0;
	};

	class AP_WS_RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		inline explicit AP_WS_RequestHandlerFactory(Poco::Logger &L) : Logger_(L) {}

		inline Poco::Net::HTTPRequestHandler *
		createRequestHandler(const Poco::Net::HTTPServerRequest &request) override {
			if (request.find("Upgrade") != request.end() &&
				Poco::icompare(request["Upgrade"], "websocket") == 0) {
				Utils::SetThreadName("ws:conn-init");
				return new AP_WS_RequestHandler(Logger_, id_++);
			} else {
				return nullptr;
			}
		}

	  private:
		Poco::Logger &Logger_;
		inline static uint64_t id_ = 1;
	};

	class AP_WS_Server : public SubSystemServer {
	  public:
		static auto instance() {
			static auto instance_ = new AP_WS_Server;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		bool IsCertOk() { return IssuerCert_ != nullptr; }
		bool ValidateCertificate(const std::string &ConnectionId,
								 const Poco::Crypto::X509Certificate &Certificate);
		// Poco::Net::SocketReactor & GetNextReactor() { return ReactorPool_.NextReactor(); }

		inline bool IsSimSerialNumber(const std::string &SerialNumber) const {
			return IsSim(Poco::toLower(SerialNumber)) &&
				   Poco::toLower(SerialNumber) == Poco::toLower(SimulatorId_);
		}

		inline static bool IsSim(const std::string &SerialNumber) {
			return SerialNumber.substr(0, 6) == "53494d";
		}

		inline bool IsSimEnabled() const { return SimulatorEnabled_; }

		inline bool AllowSerialNumberMismatch() const { return AllowSerialNumberMismatch_; }

		inline uint64_t MismatchDepth() const { return MismatchDepth_; }

		inline bool UseProvisioning() const { return LookAtProvisioning_; }
		inline bool UseDefaults() const { return UseDefaultConfig_; }

		[[nodiscard]] inline Poco::Net::SocketReactor &NextReactor() {
			return Reactor_pool_->NextReactor();
		}
		[[nodiscard]] inline bool Running() const { return Running_; }

		inline void AddConnection(uint64_t session_id,
								  std::shared_ptr<AP_WS_Connection> Connection) {
			std::lock_guard Lock(WSServerMutex_);
			Sessions_[session_id] = std::move(Connection);
		}

		inline std::shared_ptr<AP_WS_Connection> FindConnection(uint64_t session_id) const {
			std::lock_guard Lock(WSServerMutex_);

			auto Connection = Sessions_.find(session_id);
			if (Connection != end(Sessions_))
				return Connection->second;
			return nullptr;
		}

		inline bool DeviceRequiresSecureRtty(uint64_t serialNumber) const {
			std::lock_guard Lock(WSServerMutex_);

			auto Connection = SerialNumbers_.find(serialNumber);
			if (Connection==end(SerialNumbers_) || Connection->second.second==nullptr)
				return false;
			return Connection->second.second->RttyMustBeSecure_;
		}

		inline bool GetStatistics(const std::string &SerialNumber, std::string &Statistics) const {
			return GetStatistics(Utils::SerialNumberToInt(SerialNumber), Statistics);
		}
		bool GetStatistics(uint64_t SerialNumber, std::string &Statistics) const;

		inline bool GetState(const std::string &SerialNumber,
							 GWObjects::ConnectionState &State) const {
			return GetState(Utils::SerialNumberToInt(SerialNumber), State);
		}
		bool GetState(uint64_t SerialNumber, GWObjects::ConnectionState &State) const;

		inline bool GetHealthcheck(const std::string &SerialNumber,
								   GWObjects::HealthCheck &CheckData) const {
			return GetHealthcheck(Utils::SerialNumberToInt(SerialNumber), CheckData);
		}
		bool GetHealthcheck(uint64_t SerialNumber, GWObjects::HealthCheck &CheckData) const;

		bool Connected(uint64_t SerialNumber, GWObjects::DeviceRestrictions &Restrictions) const;
		bool Connected(uint64_t SerialNumber) const;

		inline bool SendFrame(const std::string &SerialNumber, const std::string &Payload) const {
			return SendFrame(Utils::SerialNumberToInt(SerialNumber), Payload);
		}

		bool SendFrame(uint64_t SerialNumber, const std::string &Payload) const;

		bool SendRadiusAuthenticationData(const std::string &SerialNumber,
										  const unsigned char *buffer, std::size_t size);
		bool SendRadiusAccountingData(const std::string &SerialNumber, const unsigned char *buffer,
									  std::size_t size);
		bool SendRadiusCoAData(const std::string &SerialNumber, const unsigned char *buffer,
							   std::size_t size);

		void SetSessionDetails(uint64_t connection_id, uint64_t SerialNumber);
		bool EndSession(uint64_t connection_id, uint64_t serial_number);

		void SetWebSocketTelemetryReporting(uint64_t RPCID, uint64_t SerialNumber,
											uint64_t Interval, uint64_t Lifetime,
											const std::vector<std::string> &TelemetryTypes);
		void StopWebSocketTelemetry(uint64_t RPCID, uint64_t SerialNumber);
		void SetKafkaTelemetryReporting(uint64_t RPCID, uint64_t SerialNumber, uint64_t Interval,
										uint64_t Lifetime,
										const std::vector<std::string> &TelemetryTypes);
		void StopKafkaTelemetry(uint64_t RPCID, uint64_t SerialNumber);
		void GetTelemetryParameters(uint64_t SerialNumber, bool &TelemetryRunning,
									uint64_t &TelemetryInterval, uint64_t &TelemetryWebSocketTimer,
									uint64_t &TelemetryKafkaTimer,
									uint64_t &TelemetryWebSocketCount,
									uint64_t &TelemetryKafkaCount,
									uint64_t &TelemetryWebSocketPackets,
									uint64_t &TelemetryKafkaPackets);

		void onGarbageCollecting(Poco::Timer &timer);

		inline void AverageDeviceStatistics(uint64_t &Connections, uint64_t &AverageConnectionTime,
											uint64_t &NumberOfConnectingDevices) const {
			Connections = NumberOfConnectedDevices_;
			AverageConnectionTime = AverageDeviceConnectionTime_;
			NumberOfConnectingDevices = NumberOfConnectingDevices_;
		}

		inline void AddRX(std::uint64_t bytes) {
			std::lock_guard		G(StatsMutex_);
			RX_ += bytes;
		}

		inline void AddTX(std::uint64_t bytes) {
			std::lock_guard		G(StatsMutex_);
			TX_ += bytes;
		}

		inline void GetTotalDataStatistics(std::uint64_t &TX, std::uint64_t &RX) const {
			std::lock_guard		G(StatsMutex_);
			TX = TX_;
			RX = RX_;
		}

		inline bool GetHealthDevices(std::uint64_t lowLimit, std::uint64_t  highLimit, std::vector<std::string> & SerialNumbers) {
			std::lock_guard		G(WSServerMutex_);

			for(const auto &connection:Sessions_) {
				if(	connection.second->RawLastHealthcheck_.Sanity>=lowLimit 	&&
					connection.second->RawLastHealthcheck_.Sanity<=highLimit) {
					SerialNumbers.push_back(connection.second->SerialNumber_);
				}
			}
			return true;
		}
	  private:
		mutable std::recursive_mutex WSServerMutex_;
		std::unique_ptr<Poco::Crypto::X509Certificate> IssuerCert_;
		std::list<std::unique_ptr<Poco::Net::HTTPServer>> WebServers_;
		Poco::Net::SocketReactor Reactor_;
		Poco::Thread ReactorThread_;
		std::string SimulatorId_;
		Poco::ThreadPool DeviceConnectionPool_{"ws:dev-pool", 2, 64};
		bool LookAtProvisioning_ = false;
		bool UseDefaultConfig_ = true;
		bool SimulatorEnabled_ = false;
		std::unique_ptr<AP_WS_ReactorThreadPool> Reactor_pool_;
		std::atomic_bool Running_ = false;
		std::map<std::uint64_t, std::shared_ptr<AP_WS_Connection>> Sessions_;
		std::map<uint64_t, std::pair<uint64_t, std::shared_ptr<AP_WS_Connection>>> SerialNumbers_;
		std::atomic_bool AllowSerialNumberMismatch_ = true;
		std::atomic_uint64_t MismatchDepth_ = 2;

		std::atomic_uint64_t NumberOfConnectedDevices_ = 0;
		std::atomic_uint64_t AverageDeviceConnectionTime_ = 0;
		std::atomic_uint64_t NumberOfConnectingDevices_ = 0;

		mutable std::mutex		StatsMutex_;
		std::atomic_uint64_t 	TX_=0,RX_=0;

		std::vector<std::shared_ptr<AP_WS_Connection>> Garbage_;

		std::unique_ptr<Poco::TimerCallback<AP_WS_Server>> GarbageCollectorCallback_;
		Poco::Timer Timer_;
		Poco::Thread GarbageCollector_;

		AP_WS_Server() noexcept
			: SubSystemServer("WebSocketServer", "WS-SVR", "ucentral.websocket") {}
	};

	inline auto AP_WS_Server() { return AP_WS_Server::instance(); }

} // namespace OpenWifi