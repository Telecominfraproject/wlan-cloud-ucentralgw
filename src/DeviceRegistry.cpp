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
#include "CommandManager.h"

#include "framework/WebSocketClientNotifications.h"

namespace OpenWifi {

	int DeviceRegistry::Start() {
		std::lock_guard		Guard(Mutex_);
		poco_notice(Logger(),"Starting");

		ArchiverCallback_ = std::make_unique<Poco::TimerCallback<DeviceRegistry>>(*this,&DeviceRegistry::onConnectionJanitor);
		Timer_.setStartInterval(60 * 1000);
		Timer_.setPeriodicInterval(20 * 1000); // every minute
		Timer_.start(*ArchiverCallback_, MicroService::instance().TimerPool());

		return 0;
    }

    void DeviceRegistry::Stop() {
		poco_notice(Logger(),"Stopping...");
		std::lock_guard		Guard(Mutex_);
		Timer_.stop();
		poco_notice(Logger(),"Stopped...");
    }

	void DeviceRegistry::onConnectionJanitor([[maybe_unused]] Poco::Timer &timer) {

		static std::uint64_t last_log = OpenWifi::Now();

		std::shared_lock Guard(LocalMutex_);

		NumberOfConnectedDevices_ = 0;
		NumberOfConnectingDevices_ = 0;
		AverageDeviceConnectionTime_ = 0;
		std::uint64_t	total_connected_time=0;

		auto now = OpenWifi::Now();
		for (auto connection=SerialNumbers_.begin(); connection!=SerialNumbers_.end();) {

			if(connection->second.second== nullptr) {
				connection++;
				continue;
			}

			if (connection->second.second->State_.Connected) {
				NumberOfConnectedDevices_++;
				total_connected_time += (now - connection->second.second->State_.started);
				connection++;
			} else {
				NumberOfConnectingDevices_++;
				connection++;
			}
		}

		AverageDeviceConnectionTime_ = (NumberOfConnectedDevices_!=0) ? total_connected_time/NumberOfConnectedDevices_ : 0;
		if((now-last_log)>120) {
			last_log = now;
			poco_information(Logger(),
				fmt::format("Active AP connections: {} Connecting: {} Average connection time: {} seconds",
							NumberOfConnectedDevices_, NumberOfConnectingDevices_, AverageDeviceConnectionTime_));
		}
		WebSocketClientNotificationNumberOfConnections(NumberOfConnectedDevices_,
													   AverageDeviceConnectionTime_,
													   NumberOfConnectingDevices_);
	}

    bool DeviceRegistry::GetStatistics(uint64_t SerialNumber, std::string & Statistics) const {
		std::shared_lock	Guard(LocalMutex_);
        auto Device = SerialNumbers_.find(SerialNumber);
        if(Device == SerialNumbers_.end() || Device->second.second==nullptr)
			return false;
		Statistics = Device->second.second->LastStats_;
		return true;
    }

    bool DeviceRegistry::GetState(uint64_t SerialNumber, GWObjects::ConnectionState & State) const {
		std::shared_lock	Guard(LocalMutex_);
        auto Device = SerialNumbers_.find(SerialNumber);
        if(Device == SerialNumbers_.end() || Device->second.second==nullptr)
			return false;
		State = Device->second.second->State_;
		return true;
    }

	bool DeviceRegistry::GetHealthcheck(uint64_t SerialNumber, GWObjects::HealthCheck & CheckData) const {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device == SerialNumbers_.end() || Device->second.second==nullptr)
			return false;

		CheckData = Device->second.second->LastHealthcheck_;
		return true;
	}

	bool DeviceRegistry::EndSession(std::uint64_t connection_id, std::uint64_t serial_number) {

		std::unique_lock G(LocalMutex_);
		auto Connection = SerialNumbers_.find(serial_number);
		if (Connection == end(SerialNumbers_)) {
			return false;
		}

		if (Connection->second.first != connection_id) {
			return false;
		}

		SerialNumbers_.erase(Connection);

		return true;
	}

	void DeviceRegistry::SetSessionDetails(std::uint64_t connection_id, uint64_t SerialNumber) {
		auto Connection = AP_WS_Server()->FindConnection(connection_id);

		if(Connection== nullptr)
			return;

		std::unique_lock	G(LocalMutex_);
		auto CurrentSerialNumber = SerialNumbers_.find(SerialNumber);
		if(	(CurrentSerialNumber==SerialNumbers_.end())	||
			(CurrentSerialNumber->second.first<connection_id)) {
			SerialNumbers_[SerialNumber] = std::make_pair(connection_id, Connection);
			return;
		}
	}

    bool DeviceRegistry::Connected(uint64_t SerialNumber) const {
		std::shared_lock Guard(LocalMutex_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second== nullptr)
			return false;

		return  Device->second.second->State_.Connected;
	}

	bool DeviceRegistry::SendFrame(uint64_t SerialNumber, const std::string & Payload) const {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==SerialNumbers_.end() || Device->second.second== nullptr)
			return false;

		try {
			// std::cout << "Device connection pointer: " << (std::uint64_t) Device->second.second << std::endl;
			return Device->second.second->Send(Payload);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendFrame: Could not send data to device '{}'", Utils::IntToSerialNumber(SerialNumber)));
		}
		return false;
	}

	void DeviceRegistry::StopWebSocketTelemetry(std::uint64_t RPCID, uint64_t SerialNumber) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second==nullptr)
			return;
		Device->second.second->StopWebSocketTelemetry(RPCID);
	}

	void DeviceRegistry::SetWebSocketTelemetryReporting(std::uint64_t RPCID, uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second==nullptr)
			return;
		Device->second.second->SetWebSocketTelemetryReporting(RPCID, Interval, Lifetime);
	}

	void DeviceRegistry::SetKafkaTelemetryReporting(std::uint64_t RPCID, uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second== nullptr)
			return;
		Device->second.second->SetKafkaTelemetryReporting(RPCID, Interval, Lifetime);
	}

	void DeviceRegistry::StopKafkaTelemetry(std::uint64_t RPCID, uint64_t SerialNumber) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_) || Device->second.second== nullptr)
			return;
		Device->second.second->StopKafkaTelemetry(RPCID);
	}

	void DeviceRegistry::GetTelemetryParameters(uint64_t SerialNumber , bool & TelemetryRunning,
								uint64_t & TelemetryInterval,
								uint64_t & TelemetryWebSocketTimer,
								uint64_t & TelemetryKafkaTimer,
								uint64_t & TelemetryWebSocketCount,
								uint64_t & TelemetryKafkaCount,
								uint64_t & TelemetryWebSocketPackets,
								uint64_t & TelemetryKafkaPackets) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_)|| Device->second.second== nullptr)
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
		std::shared_lock	Guard(LocalMutex_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end() || Device->second.second== nullptr)
			return false;

		try {
			return Device->second.second->SendRadiusAccountingData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	bool DeviceRegistry::SendRadiusAuthenticationData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end() || Device->second.second== nullptr)
			return false;

		try {
			return Device->second.second->SendRadiusAuthenticationData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusAuthenticationData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

	bool DeviceRegistry::SendRadiusCoAData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end() || Device->second.second== nullptr)
			return false;

		try {
			return Device->second.second->SendRadiusCoAData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusCoAData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

}  // namespace