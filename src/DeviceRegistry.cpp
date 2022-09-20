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
		poco_notice(Logger(),"Starting");

		ArchiverCallback_ = std::make_unique<Poco::TimerCallback<DeviceRegistry>>(*this,&DeviceRegistry::onConnectionJanitor);
		Timer_.setStartInterval(60 * 1000);
		Timer_.setPeriodicInterval(20 * 1000); // every minute
		Timer_.start(*ArchiverCallback_, MicroService::instance().TimerPool());

		return 0;
    }

    void DeviceRegistry::Stop() {
		std::vector<AP_WS_Connection *>	connections;

		poco_notice(Logger(),"Stopping...");
		std::lock_guard		Guard(Mutex_);
		Timer_.stop();
		{
			std::unique_lock	G(LocalMutex_);
			for(const auto &[_,Session]:Sessions_)
				connections.emplace_back(Session);
		}
		for(auto &c:connections) {
			std::cout << "Deleting connection..." << std::endl;
			c->EndConnection();
		}
		poco_notice(Logger(),"Stopped...");
    }

	void DeviceRegistry::onConnectionJanitor([[maybe_unused]] Poco::Timer &timer) {

		static std::uint64_t last_log = OpenWifi::Now();

		using session_tuple=std::tuple<std::uint64_t,AP_WS_Connection *,std::uint64_t>;
		std::vector<session_tuple> connections;
		{
			std::shared_lock Guard(LocalMutex_);

			NumberOfConnectedDevices_ = 0;
			AverageDeviceConnectionTime_ = 0;
			std::uint64_t	total_connected_time=0;

			auto now = OpenWifi::Now();
			for (const auto &[serial_number, connection_info] : SerialNumbers_) {
				if ((now - connection_info.second->State_.LastContact) > 500) {
					session_tuple S{serial_number,connection_info.second,connection_info.second->State_.sessionId};
					connections.emplace_back(S);
				} else {
					NumberOfConnectedDevices_++;
					total_connected_time += (now - connection_info.second->State_.started);
				}
			}
			AverageDeviceConnectionTime_ = (NumberOfConnectedDevices_!=0) ? total_connected_time/NumberOfConnectedDevices_ : 0;
			if((now-last_log)>120) {
				last_log = now;
				poco_information(Logger(),
					fmt::format("Active AP connections: {} Average connection time: {} seconds",
								NumberOfConnectedDevices_, AverageDeviceConnectionTime_));
			}
		}

		for(auto [serial_number,ws_connection,id]:connections) {
			poco_information(Logger(),fmt::format("Removing orphaned AP Session {} for {}", id, Utils::IntToSerialNumber(serial_number)));
			// delete ws_connection;
		}
	}

    bool DeviceRegistry::GetStatistics(uint64_t SerialNumber, std::string & Statistics) const {
		std::shared_lock	Guard(LocalMutex_);
        auto Device = SerialNumbers_.find(SerialNumber);
        if(Device == SerialNumbers_.end())
			return false;
		Statistics = Device->second.second->LastStats_;
		return true;
    }

    bool DeviceRegistry::GetState(uint64_t SerialNumber, GWObjects::ConnectionState & State) const {
		std::shared_lock	Guard(LocalMutex_);
        auto Device = SerialNumbers_.find(SerialNumber);
        if(Device == SerialNumbers_.end())
			return false;
		State = Device->second.second->State_;
		return true;
    }

	bool DeviceRegistry::EndSession(std::uint64_t connection_id, [[maybe_unused]] AP_WS_Connection * connection, std::uint64_t serial_number) {
		std::unique_lock	G(LocalMutex_);

		auto Session = Sessions_.find(connection_id);
		if(Session==end(Sessions_)) {
			return false;
		}

		auto hint = SerialNumbers_.find(serial_number);

		bool SessionDeleted = false;

		if(	(hint != end(SerialNumbers_)) &&
			(connection_id == hint->second.second->State_.sessionId)) {
			poco_debug(Logger(),fmt::format("Ending session {}, serial {}.", connection_id, Utils::IntToSerialNumber(serial_number)));
			SerialNumbers_.erase(serial_number);
			SessionDeleted = true;
		} else {
			poco_debug(Logger(),fmt::format("Not Ending session {}, serial {}. This is an old session.", connection_id, Utils::IntToSerialNumber(serial_number)));
		}
		Sessions_.erase(Session);
		return SessionDeleted;
	}

	bool DeviceRegistry::GetHealthcheck(uint64_t SerialNumber, GWObjects::HealthCheck & CheckData) const {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device == SerialNumbers_.end())
			return false;

		CheckData = Device->second.second->LastHealthcheck_;
		return true;
	}

    bool DeviceRegistry::Connected(uint64_t SerialNumber) const {
		std::shared_lock Guard(LocalMutex_);
		return SerialNumbers_.find(SerialNumber) != SerialNumbers_.end();
	}

	bool DeviceRegistry::SendFrame(uint64_t SerialNumber, const std::string & Payload) const {
		std::shared_lock	Guard(LocalMutex_);
		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==SerialNumbers_.end())
			return false;

		try {
			// std::cout << "Device connection pointer: " << (std::uint64_t) Device->second.second << std::endl;
			return Device->second.second->Send(Payload);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendFrame: Could not send data to device '{}'", Utils::IntToSerialNumber(SerialNumber)));
		}
		return false;
	}

	void DeviceRegistry::StopWebSocketTelemetry(uint64_t SerialNumber) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_))
			return;
		Device->second.second->StopWebSocketTelemetry();
	}

	void DeviceRegistry::SetWebSocketTelemetryReporting(uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_))
			return;
		Device->second.second->SetWebSocketTelemetryReporting(Interval, Lifetime);
	}

	void DeviceRegistry::SetKafkaTelemetryReporting(uint64_t SerialNumber, uint64_t Interval, uint64_t Lifetime) {
		std::shared_lock	Guard(LocalMutex_);

		auto Device = SerialNumbers_.find(SerialNumber);
		if(Device==end(SerialNumbers_))
			return;
		Device->second.second->SetKafkaTelemetryReporting(Interval, Lifetime);
	}

	void DeviceRegistry::StopKafkaTelemetry(uint64_t SerialNumber) {
		std::shared_lock	Guard(LocalMutex_);

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
		std::shared_lock	Guard(LocalMutex_);

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
		std::shared_lock	Guard(LocalMutex_);
		auto Device = 		SerialNumbers_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device==SerialNumbers_.end())
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
		if(Device==SerialNumbers_.end())
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
		if(Device==SerialNumbers_.end())
			return false;

		try {
			return Device->second.second->SendRadiusCoAData(buffer,size);
		} catch (...) {
			poco_debug(Logger(),fmt::format(": SendRadiusCoAData: Could not send data to device '{}'", SerialNumber));
		}
		return false;
	}

}  // namespace