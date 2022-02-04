//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "Poco/JSON/Object.h"

#include "RESTObjects//RESTAPI_GWobjects.h"
#include "framework/MicroService.h"

// class uCentral::WebSocket::WSConnection;

namespace OpenWifi {

	class WSConnection;
    class DeviceRegistry : public SubSystemServer {
    public:
		struct ConnectionEntry {
			WSConnection 				* WSConn_ = nullptr;
			GWObjects::ConnectionState 	Conn_;
			std::string        			LastStats;
			GWObjects::HealthCheck		LastHealthcheck;
			uint64_t 					ConnectionId=0;
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

		std::shared_ptr<ConnectionEntry> Register(uint64_t SerialNumber, WSConnection *Conn, uint64_t & ConnectionId);

		inline void UnRegister(const std::string & SerialNumber, uint64_t ConnectionId) {
			return UnRegister(Utils::SerialNumberToInt(SerialNumber),ConnectionId);
		}
		void UnRegister(uint64_t SerialNumber, uint64_t ConnectionId);

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

		bool AnalyzeRegistry(GWObjects::Dashboard &D);

		[[nodiscard]] inline std::shared_ptr<ConnectionEntry> GetDeviceConnection(const std::string & SerialNumber) {
			return GetDeviceConnection(Utils::SerialNumberToInt(SerialNumber));
		}

		[[nodiscard]] inline std::shared_ptr<ConnectionEntry> GetDeviceConnection(uint64_t SerialNumber) {
			std::lock_guard		Guard(Mutex_);
			auto Device = Devices_.find(SerialNumber);
			if(Device!=Devices_.end() && Device->second->WSConn_!= nullptr) {
				return Device->second;
			}
			return nullptr;
		}

	  private:
		inline static std::atomic_uint64_t 						Id_=1;
		std::map<uint64_t ,std::shared_ptr<ConnectionEntry>>  	Devices_;

		DeviceRegistry() noexcept:
    		SubSystemServer("DeviceRegistry", "DevStatus", "devicestatus") {
		}
	};

	inline auto DeviceRegistry() { return DeviceRegistry::instance(); }

}  // namespace

