//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"

#include "DeviceRegistry.h"
#include "WS_Server.h"
#include "OUIServer.h"

namespace OpenWifi {

	int DeviceRegistry::Start() {
		std::lock_guard		Guard(Mutex_);
        Logger().notice("Starting ");
        return 0;
    }

    void DeviceRegistry::Stop() {
		std::lock_guard		Guard(Mutex_);
        Logger().notice("Stopping ");
    }

    bool DeviceRegistry::GetStatistics(uint64_t SerialNumber, std::string & Statistics) {
		std::lock_guard		Guard(Mutex_);
        auto Device = Devices_.find(SerialNumber);
        if(Device == Devices_.end())
			return false;
		Statistics = Device->second->LastStats;
		return true;
    }

    void DeviceRegistry::SetStatistics(uint64_t SerialNumber, const std::string &Statistics) {
		std::lock_guard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);
        if(Device != Devices_.end())
        {
			Device->second->Conn_.LastContact = time(nullptr);
            Device->second->LastStats = Statistics;
        }
    }

    bool DeviceRegistry::GetState(uint64_t SerialNumber, GWObjects::ConnectionState & State) {
		std::lock_guard		Guard(Mutex_);
        auto Device = Devices_.find(SerialNumber);
        if(Device == Devices_.end())
			return false;

		State = Device->second->Conn_;
		return true;
    }

    void DeviceRegistry::SetState(uint64_t SerialNumber, const GWObjects::ConnectionState & State) {
		std::lock_guard		Guard(Mutex_);
        auto Device = Devices_.find(SerialNumber);
        if(Device != Devices_.end())
        {
			Device->second->Conn_.LastContact = time(nullptr);
            Device->second->Conn_ = State;
        }
    }

	bool DeviceRegistry::GetHealthcheck(uint64_t SerialNumber, GWObjects::HealthCheck & CheckData) {
		std::lock_guard		Guard(Mutex_);

		auto Device = Devices_.find(SerialNumber);
		if(Device != Devices_.end()) {
			CheckData = Device->second->LastHealthcheck;
			return true;
		}
		return false;
	}

	void DeviceRegistry::SetHealthcheck(uint64_t SerialNumber, const GWObjects::HealthCheck & CheckData) {
		std::lock_guard		Guard(Mutex_);

		auto Device = Devices_.find(SerialNumber);
		if(Device != Devices_.end())
		{
			Device->second->LastHealthcheck = CheckData;
		}
	}

	std::shared_ptr<DeviceRegistry::ConnectionEntry> DeviceRegistry::Register(uint64_t SerialNumber, WSConnection *Ptr, uint64_t & ConnectionId )
    {
		std::lock_guard		Guard(Mutex_);

		const auto & E = Devices_[SerialNumber] = std::make_shared<ConnectionEntry>();
		E->WSConn_ = Ptr;
		E->Conn_.LastContact = OpenWifi::Now();
		E->Conn_.Connected = true ;
		E->Conn_.UUID = 0 ;
		E->Conn_.MessageCount = 0 ;
		E->Conn_.Address = "";
		E->Conn_.TX = 0 ;
		E->Conn_.RX = 0;
		E->Conn_.VerifiedCertificate = GWObjects::CertificateValidation::NO_CERTIFICATE;
		ConnectionId = E->ConnectionId = ++Id_;
		return E;
    }

    bool DeviceRegistry::Connected(uint64_t SerialNumber) {
		std::lock_guard		Guard(Mutex_);
        auto Device = Devices_.find(SerialNumber);
        if(Device == Devices_.end())
            return false;
        return Device->second->Conn_.Connected;
    }

    void DeviceRegistry::UnRegister(uint64_t SerialNumber, uint64_t ConnectionId) {
		std::lock_guard		Guard(Mutex_);
		auto It = Devices_.find(SerialNumber);
		if(It!=Devices_.end()) {
			if(It->second->ConnectionId == ConnectionId)
				Devices_.erase(SerialNumber);
		}
	}

	bool DeviceRegistry::SendFrame(uint64_t SerialNumber, const std::string & Payload) {
		std::lock_guard		Guard(Mutex_);
		auto Device = Devices_.find(SerialNumber);
		if(Device!=Devices_.end() && Device->second->WSConn_!= nullptr) {
			try {
				return Device->second->WSConn_->Send(Payload);
			} catch (...) {
				Logger().debug(fmt::format("Could not send data to device '{}'", SerialNumber));
				Device->second->Conn_.Address = "";
				Device->second->WSConn_ = nullptr;
				Device->second->Conn_.Connected = false;
				Device->second->Conn_.VerifiedCertificate = GWObjects::NO_CERTIFICATE;
			}
		}
		return false;
	}

	bool DeviceRegistry::SendRadiusAccountingData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::lock_guard		Guard(Mutex_);
		auto Device = 		Devices_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device!=Devices_.end() && Device->second->WSConn_!= nullptr) {
			try {
				return Device->second->WSConn_->SendRadiusAccountingData(buffer,size);
			} catch (...) {
				Logger().debug(fmt::format("Could not send data to device '{}'", SerialNumber));
				Device->second->Conn_.Address = "";
				Device->second->WSConn_ = nullptr;
				Device->second->Conn_.Connected = false;
				Device->second->Conn_.VerifiedCertificate = GWObjects::NO_CERTIFICATE;
			}
		}
		return false;
	}

	bool DeviceRegistry::SendRadiusAuthenticationData(const std::string & SerialNumber, const unsigned char * buffer, std::size_t size) {
		std::lock_guard		Guard(Mutex_);
		auto Device = 		Devices_.find(Utils::SerialNumberToInt(SerialNumber));
		if(Device!=Devices_.end() && Device->second->WSConn_!= nullptr) {
			try {
				return Device->second->WSConn_->SendRadiusAuthenticationData(buffer,size);
			} catch (...) {
				Logger().debug(fmt::format("Could not send data to device '{}'", SerialNumber));
				Device->second->Conn_.Address = "";
				Device->second->WSConn_ = nullptr;
				Device->second->Conn_.Connected = false;
				Device->second->Conn_.VerifiedCertificate = GWObjects::NO_CERTIFICATE;
			}
		}
		return false;
	}

	void DeviceRegistry::SetPendingUUID(uint64_t SerialNumber, uint64_t PendingUUID) {
		std::lock_guard		Guard(Mutex_);
		auto Device = Devices_.find(SerialNumber);
		if(Device!=Devices_.end()) {
			Device->second->Conn_.PendingUUID = PendingUUID;
		}
	}

}  // namespace