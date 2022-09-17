//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <shared_mutex>

#include "Poco/JSON/Object.h"
#include "Poco/Timer.h"
#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/MicroService.h"

namespace OpenWifi {

	class AP_WS_Connection;
    class DeviceRegistry : public SubSystemServer {
    public:

		DeviceRegistry() noexcept:
			SubSystemServer("DeviceRegistry", "DevStatus", "devicestatus") {
		}

        static auto instance() {
            static auto instance_ = new DeviceRegistry;
            return instance_;
        }

		int Start() override;
		void Stop() override;

		inline bool GetStatistics(const std::string &SerialNumber, std::string & Statistics) const {
			return GetStatistics(Utils::SerialNumberToInt(SerialNumber),Statistics);
		}
		bool GetStatistics(std::uint64_t SerialNumber, std::string & Statistics) const;

		inline bool GetState(const std::string & SerialNumber, GWObjects::ConnectionState & State) const {
			return GetState(Utils::SerialNumberToInt(SerialNumber), State);
		}
		bool GetState(std::uint64_t SerialNumber, GWObjects::ConnectionState & State) const;

		inline bool GetHealthcheck(const std::string &SerialNumber, GWObjects::HealthCheck & CheckData) const {
			return GetHealthcheck(Utils::SerialNumberToInt(SerialNumber), CheckData);
		}
		bool GetHealthcheck(std::uint64_t SerialNumber, GWObjects::HealthCheck & CheckData) const ;

		bool Connected(uint64_t SerialNumber) const ;

		inline bool SendFrame(const std::string & SerialNumber, const std::string & Payload) const {
			return SendFrame(Utils::SerialNumberToInt(SerialNumber), Payload);
		}

		bool SendFrame(std::uint64_t SerialNumber, const std::string & Payload) const ;

/*		inline void SetPendingUUID(const std::string & SerialNumber, uint64_t PendingUUID) {
			return SetPendingUUID(Utils::SerialNumberToInt(SerialNumber), PendingUUID);
		}

		void SetPendingUUID(std::uint64_t SerialNumber, std::uint64_t PendingUUID);
*/
		bool SendRadiusAuthenticationData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);
		bool SendRadiusAccountingData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);
		bool SendRadiusCoAData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);

		inline void StartSession(uint64_t ConnectionId, AP_WS_Connection * connection) {
			std::unique_lock	G(LocalMutex_);
			Sessions_[ConnectionId] = connection;
		}

		inline void SetSessionDetails(std::uint64_t connection_id, AP_WS_Connection * connection, uint64_t SerialNumber) {
			std::unique_lock	G(LocalMutex_);
			auto Hint = Sessions_.find(connection_id);
			if(Hint!=Sessions_.end() && Hint->second==connection) {
				Logger().information(fmt::format("Starting session {}, serial {}.", connection_id, Utils::IntToSerialNumber(SerialNumber)));
				SerialNumbers_[SerialNumber] = std::make_pair(connection_id,connection);
			}
		}

		bool EndSession(std::uint64_t connection_id, AP_WS_Connection * connection, std::uint64_t serial_number);

		void SetWebSocketTelemetryReporting(uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime);
		void StopWebSocketTelemetry(uint64_t SerialNumber);
		void SetKafkaTelemetryReporting(uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime);
		void StopKafkaTelemetry(uint64_t SerialNumber);
		void GetTelemetryParameters(uint64_t SerialNumber , bool & TelemetryRunning,
									uint64_t & TelemetryInterval,
									uint64_t & TelemetryWebSocketTimer,
									uint64_t & TelemetryKafkaTimer,
									uint64_t & TelemetryWebSocketCount,
									uint64_t & TelemetryKafkaCount,
									uint64_t & TelemetryWebSocketPackets,
									uint64_t & TelemetryKafkaPackets);

		void onConnectionJanitor(Poco::Timer & timer);

		inline void AverageDeviceStatistics( std::uint64_t & Connections, std::uint64_t & AverageConnectionTime ) const {
			Connections = NumberOfConnectedDevices_;
			AverageConnectionTime = AverageDeviceConnectionTime_;
		}

	  private:
		mutable std::shared_mutex														LocalMutex_;
		std::map<std::uint64_t, AP_WS_Connection *>  									Sessions_;
		std::map<std::uint64_t, std::pair<std::uint64_t, AP_WS_Connection *>>			SerialNumbers_;

		std::unique_ptr<Poco::TimerCallback<DeviceRegistry>>   ArchiverCallback_;
		Poco::Timer                     		Timer_;
		Poco::Thread							ConnectionJanitor_;
		std::atomic_uint64_t 					NumberOfConnectedDevices_=0;
		std::atomic_uint64_t 					AverageDeviceConnectionTime_=0;

	};

	inline auto DeviceRegistry() { return DeviceRegistry::instance(); }

}  // namespace

