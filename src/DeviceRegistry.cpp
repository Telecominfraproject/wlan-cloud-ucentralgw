//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Object.h"
#include "AP_WS_Server.h"
#include "DeviceRegistry.h"

namespace OpenWifi {

	int DeviceRegistry::Start() {
		std::lock_guard		Guard(Mutex_);
        Logger().notice("Starting ");
        return 0;
    }

    void DeviceRegistry::Stop() {
		std::lock_guard		Guard(Mutex_);
        Logger().notice("Stopping...");
    }

    bool DeviceRegistry::GetStatistics(uint64_t SerialNumber, std::string & Statistics) {
		std::shared_lock	Guard(M_);
        auto Device = SerialNumbers_.find(SerialNumber);
        if(Device == SerialNumbers_.end())
			return false;
		Statistics = Device->second.first->LastStats;
		return true;
    }

    void DeviceRegistry::SetStatistics(uint64_t SerialNumber, const std::string &Statistics) {
		std::unique_lock	Guard(M_);

        auto Device = SerialNumbers_.find(SerialNumber);
        if(Device == SerialNumbers_.end())
			return;

		Device->second.first->State_.LastContact = OpenWifi::Now();
		Device->second.first->LastStats = Statistics;
    }

    bool DeviceRegistry::GetState(uint64_t SerialNumber, GWObjects::ConnectionState & State) {
		std::shared_lock	Guard(M_);
        auto Device = SerialNumbers_.find(SerialNumber);
        if(Device == SerialNumbers_.end())
			return false;

		State = Device->second.first->State_;
		return true;
    }

	void DeviceRegistry::EndSession(std::uint64_t connection_id, AP_WS_Connection * connection, std::uint64_t serial_number) {
		std::unique_lock	G(M_);

		auto Session = Sessions_.find(connection_id);
		if(Session==end(Sessions_)) {
			return;
		}

		auto hint = SerialNumbers_.find(serial_number);
		if(	(hint != end(SerialNumbers_)) &&
			(hint->second.second == connection) &&
			(connection_id == hint->second.first->ConnectionId)) {
			SerialNumbers_.erase(serial_number);
		}
		Sessions_.erase(connection_id);
	}

    void DeviceRegistry::SetState(uint64_t SerialNumber, const GWObjects::ConnectionState & State) {
		std::unique_lock	Guard(M_);
        auto Device = SerialNumbers_.find(SerialNumber);
        if(Device == SerialNumbers_.end())
			return;
		Device->second.first->State_.LastContact = OpenWifi::Now();
		Device->second.first->State_ = State;
    }

	bool DeviceRegistry::GetHealthcheck(uint64_t SerialNumber, GWObjects::HealthCheck & CheckData) {
		std::shared_lock	Guard(M_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device == SerialNumbers_.end())
			return false;

		CheckData = Device->second.first->LastHealthcheck;

		return true;
	}

	void DeviceRegistry::SetHealthcheck(uint64_t SerialNumber, const GWObjects::HealthCheck & CheckData) {
		std::unique_lock	Guard(M_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device == SerialNumbers_.end())
			return;

		Device->second.first->LastHealthcheck = CheckData;
	}

    bool DeviceRegistry::Connected(uint64_t SerialNumber) {
		std::shared_lock Guard(M_);
		return SerialNumbers_.find(SerialNumber) != SerialNumbers_.end();
	}

	bool DeviceRegistry::SendFrame(uint64_t SerialNumber, const std::string & Payload) {
		std::shared_lock	Guard(M_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==SerialNumbers_.end())
			return false;

		try {
			// std::cout << "Device connection pointer: " << (std::uint64_t) Device->second.second << std::endl;
			return Device->second.second->Send(Payload);
		} catch (...) {
			Logger().debug(fmt::format(": SendFrame: Could not send data to device '{}'", Utils::IntToSerialNumber(SerialNumber)));
		}
		return false;
	}

	void DeviceRegistry::StopWebSocketTelemetry(uint64_t SerialNumber) {
		std::shared_lock	Guard(M_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_))
			return;
		Device->second.second->StopWebSocketTelemetry();
	}

	void DeviceRegistry::SetWebSocketTelemetryReporting(uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime) {
		std::shared_lock	Guard(M_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_))
			return;
		Device->second.second->SetWebSocketTelemetryReporting(Interval, Lifetime);
	}

	void DeviceRegistry::SetKafkaTelemetryReporting(uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime) {
		std::shared_lock	Guard(M_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_))
			return;
		Device->second.second->SetKafkaTelemetryReporting(Interval, Lifetime);
	}

	void DeviceRegistry::StopKafkaTelemetry(uint64_t SerialNumber) {
		std::shared_lock	Guard(M_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_))
			return;
		Device->second.second->StopKafkaTelemetry();
	}

	void DeviceRegistry::GetTelemetryParameters(uint64_t SerialNumber , bool & TelemetryRunning,
								uint64_t & TelemetryInterval,
								uint64_t & TelemetryWebSocketTimer,
								uint64_t & TelemetryKafkaTimer,
								uint64_t & TelemetryWebSocketCount,
								uint64_t & TelemetryKafkaCount,
								uint64_t & TelemetryWebSocketPackets,
								uint64_t & TelemetryKafkaPackets) {
		std::shared_lock	Guard(M_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_))
			return;
		Device->second.second->GetTelemetryParameters(TelemetryRunning,
													  TelemetryInterval,
													  TelemetryWebSocketTimer,
													  TelemetryKafkaTimer,
													  TelemetryWebSocketCount,
													  TelemetryKafkaCount,
													  TelemetryWebSocketPackets,
													  TelemetryKafkaPackets);
	}

	bool DeviceRegistry::SendRadiusAccountingData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_lock	Guard(M_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end())
			return false;

		try {
			return Device->second.second->SendRadiusAccountingData(buffer,size);
		} catch (...) {
			Logger().debug(fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	bool DeviceRegistry::SendRadiusAuthenticationData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_lock	Guard(M_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end())
			return false;

		try {
			return Device->second.second->SendRadiusAuthenticationData(buffer,size);
		} catch (...) {
			Logger().debug(fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	bool DeviceRegistry::SendRadiusCoAData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_lock	Guard(M_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end())
			return false;

		try {
			return Device->second.second->SendRadiusCoAData(buffer,size);
		} catch (...) {
			Logger().debug(fmt::format(": SendRadiusCoAData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	void DeviceRegistry::SetPendingUUID(uint64_t SerialNumber, uint64_t PendingUUID) {
		std::unique_lock		Guard(M_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==SerialNumbers_.end())
			return;

		Device->second.first->State_.PendingUUID = PendingUUID;
	}

}  // namespace