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
#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/MicroService.h"

namespace OpenWifi {

	class AP_WS_Connection;
    class DeviceRegistry : public SubSystemServer {
    public:
		struct RegistryConnectionEntry {
			GWObjects::ConnectionState 	State_;
			std::string        			LastStats;
			GWObjects::HealthCheck		LastHealthcheck;
			uint64_t 					ConnectionId=0;
			uint64_t 					SerialNumber_=0;
		};

        static auto instance() {
            static auto instance_ = new DeviceRegistry;
            return instance_;
        }

		int Start() override;
		void Stop() override;

		inline bool GetStatistics(const std::string &SerialNumber, std::string & Statistics) {
			return GetStatistics(Utils::SerialNumberToInt(SerialNumber),Statistics);
		}
		bool GetStatistics(uint64_t SerialNumber, std::string & Statistics);

		inline void SetStatistics(const std::string &SerialNumber, const std::string &Statistics) {
			return SetStatistics(Utils::SerialNumberToInt(SerialNumber),Statistics);
		}
		void SetStatistics(uint64_t SerialNumber, const std::string &stats);

		inline bool GetState(const std::string & SerialNumber, GWObjects::ConnectionState & State) {
			return GetState(Utils::SerialNumberToInt(SerialNumber), State);
		}
		bool GetState(uint64_t SerialNumber, GWObjects::ConnectionState & State);

		inline void SetState(const std::string & SerialNumber, const GWObjects::ConnectionState & State) {
			return SetState(Utils::SerialNumberToInt(SerialNumber), State);
		}
		void SetState(uint64_t SerialNumber, const GWObjects::ConnectionState & State);

		inline bool GetHealthcheck(const std::string &SerialNumber, GWObjects::HealthCheck & CheckData) {
			return GetHealthcheck(Utils::SerialNumberToInt(SerialNumber), CheckData);
		}
		bool GetHealthcheck(uint64_t SerialNumber, GWObjects::HealthCheck & CheckData);

		inline void SetHealthcheck(const std::string &SerialNumber, const GWObjects::HealthCheck &H) {
			return SetHealthcheck(Utils::SerialNumberToInt(SerialNumber),H);
		}
		void SetHealthcheck(uint64_t SerialNumber, const GWObjects::HealthCheck &H);

		inline bool Connected(const std::string & SerialNumber) {
			return Connected(Utils::SerialNumberToInt(SerialNumber));
		}

		bool Connected(uint64_t SerialNumber);

		inline bool SendFrame(const std::string & SerialNumber, const std::string & Payload) {
			return SendFrame(Utils::SerialNumberToInt(SerialNumber), Payload);
		}

		bool SendFrame(uint64_t SerialNumber, const std::string & Payload);

		inline void SetPendingUUID(const std::string & SerialNumber, uint64_t PendingUUID) {
			return SetPendingUUID(Utils::SerialNumberToInt(SerialNumber), PendingUUID);
		}
		void SetPendingUUID(uint64_t SerialNumber, uint64_t PendingUUID);

/*
		[[nodiscard]] inline std::shared_ptr<RegistryConnectionEntry> GetDeviceConnection(const std::string & SerialNumber) {
			return GetDeviceConnection(Utils::SerialNumberToInt(SerialNumber));
		}

		[[nodiscard]] inline std::shared_ptr<RegistryConnectionEntry> GetDeviceConnection(uint64_t SerialNumber) {
			std::lock_guard		Guard(Mutex_);
			auto Device = SerialNumbers_.find(SerialNumber);
			if(Device!=SerialNumbers_.end()) {
				return Device->second.first;
			}
			return nullptr;
		}
*/

		bool SendRadiusAuthenticationData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);
		bool SendRadiusAccountingData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);
		bool SendRadiusCoAData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size);

		[[nodiscard]] inline std::shared_ptr<RegistryConnectionEntry> StartSession( AP_WS_Connection * connection ) {
			std::unique_lock	G(M_);
			auto NewSession = std::make_shared<RegistryConnectionEntry>();
			Sessions_[connection] = NewSession;
			return NewSession;
		}

		inline void SetSessionDetails(AP_WS_Connection * connection, uint64_t SerialNumber, uint64_t & ConnectionId ) {
			std::unique_lock	G(M_);
			auto Hint = Sessions_.find(connection);
			if(Hint!=Sessions_.end()) {
				Hint->second->SerialNumber_ = SerialNumber;
				ConnectionId = Hint->second->ConnectionId = Id_++;
				Hint->second->State_.Connected = true;
				Hint->second->State_.LastContact = OpenWifi::Now();
				Hint->second->State_.VerifiedCertificate = GWObjects::CertificateValidation::NO_CERTIFICATE;
				SerialNumbers_[SerialNumber] = std::make_pair(Hint->second,connection);
			}
		}

		inline void EndSession(AP_WS_Connection * connection, std::uint64_t serial_number ) {
			std::unique_lock	G(M_);

			auto Session = Sessions_.find(connection);
			if(Session==Sessions_.end()) {
				return;
			}

			//	if there was a serial number
			// 	if we know the serial number && this is for teh same connection: in the case a device disconnected and reconnected before we detect the
			// 	disconnection
			if(Session->second->SerialNumber_ && Session->second->SerialNumber_==serial_number) {
				auto hint = SerialNumbers_.find(Session->second->SerialNumber_);
				if((hint != end(SerialNumbers_)) && (hint->second.second == connection)) {
					SerialNumbers_.erase(Session->second->SerialNumber_);
				}
			}
			Sessions_.erase(Session);
		}

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

	  private:
		std::shared_mutex														M_;
		inline static std::atomic_uint64_t 										Id_=1;
		std::map<AP_WS_Connection * ,std::shared_ptr<RegistryConnectionEntry>>  Sessions_;
		std::map<uint64_t, std::pair<std::shared_ptr<RegistryConnectionEntry>,AP_WS_Connection *>>			SerialNumbers_;

		DeviceRegistry() noexcept:
    		SubSystemServer("DeviceRegistry", "DevStatus", "devicestatus") {
		}
	};

	inline auto DeviceRegistry() { return DeviceRegistry::instance(); }

}  // namespace

