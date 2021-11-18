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
#include "WebSocketServer.h"
#include "framework/MicroService.h"
#include "OUIServer.h"

namespace OpenWifi {

	int DeviceRegistry::Start() {
		std::lock_guard		Guard(Mutex_);
        Logger_.notice("Starting ");
        return 0;
    }

    void DeviceRegistry::Stop() {
		std::lock_guard		Guard(Mutex_);
        Logger_.notice("Stopping ");
    }

    bool DeviceRegistry::GetStatistics(const std::string &SerialNumber, std::string & Statistics) {
		std::lock_guard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);
        if(Device == Devices_.end())
			return false;

		Statistics = Device->second->LastStats;
		return true;
    }

    void DeviceRegistry::SetStatistics(const std::string &SerialNumber, const std::string &Statistics) {
		std::lock_guard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
			Device->second->Conn_.LastContact = time(nullptr);
            Device->second->LastStats = Statistics;
        }
    }

    bool DeviceRegistry::GetState(const std::string &SerialNumber, GWObjects::ConnectionState & State) {
		std::lock_guard		Guard(Mutex_);
        auto Device = Devices_.find(SerialNumber);
        if(Device == Devices_.end())
			return false;

		State = Device->second->Conn_;
		return true;
    }

    void DeviceRegistry::SetState(const std::string & SerialNumber, GWObjects::ConnectionState & State) {
		std::lock_guard		Guard(Mutex_);
        auto Device = Devices_.find(SerialNumber);
        if(Device != Devices_.end())
        {
			Device->second->Conn_.LastContact = time(nullptr);
            Device->second->Conn_ = State;
        }
    }

	bool DeviceRegistry::GetHealthcheck(const std::string &SerialNumber, GWObjects::HealthCheck & CheckData) {
		std::lock_guard		Guard(Mutex_);

		auto Device = Devices_.find(SerialNumber);
		if(Device != Devices_.end()) {
			CheckData = Device->second->LastHealthcheck;
			return true;
		}
		return false;
	}

	void DeviceRegistry::SetHealthcheck(const std::string &SerialNumber, const GWObjects::HealthCheck & CheckData) {
		std::lock_guard		Guard(Mutex_);

		auto Device = Devices_.find(SerialNumber);

		if(Device != Devices_.end())
		{
			Device->second->LastHealthcheck = CheckData;
		}
	}

	std::shared_ptr<DeviceRegistry::ConnectionEntry> DeviceRegistry::Register(const std::string & SerialNumber, WSConnection *Ptr)
    {
		std::lock_guard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);
        if( Device == Devices_.end()) {
        	auto E = Devices_[SerialNumber] = std::make_shared<ConnectionEntry>();

            E->WSConn_ = Ptr;
            E->Conn_.SerialNumber = SerialNumber;
            E->Conn_.LastContact = std::time(nullptr);
            E->Conn_.Connected = true ;
            E->Conn_.UUID = 0 ;
            E->Conn_.MessageCount = 0 ;
            E->Conn_.Address = "";
            E->Conn_.TX = 0 ;
            E->Conn_.RX = 0;
			E->Conn_.VerifiedCertificate = GWObjects::CertificateValidation::NO_CERTIFICATE;
            return E;
        }
        else
        {
            Device->second->WSConn_ = Ptr;
            Device->second->Conn_.Connected = true;
            Device->second->Conn_.LastContact = std::time(nullptr);
			Device->second->Conn_.VerifiedCertificate = GWObjects::CertificateValidation::NO_CERTIFICATE;
            return Device->second;
        }
    }

    bool DeviceRegistry::Connected(const std::string & SerialNumber) {
		std::lock_guard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device == Devices_.end())
            return false;

        return Device->second->Conn_.Connected;
    }

    void DeviceRegistry::UnRegister(const std::string & SerialNumber, WSConnection *Ptr) {
		std::lock_guard		Guard(Mutex_);
		Devices_.erase(SerialNumber);
    }

	bool DeviceRegistry::SendFrame(const std::string & SerialNumber, const std::string & Payload) {
		std::lock_guard		Guard(Mutex_);
		auto Device = Devices_.find(SerialNumber);
		if(Device!=Devices_.end() && Device->second->WSConn_!= nullptr) {
			try {
				return Device->second->WSConn_->Send(Payload);
			} catch (...) {
				Logger_.debug(Poco::format("Could not send data to device '%s'", SerialNumber));
				Device->second->Conn_.Address = "";
				Device->second->WSConn_ = nullptr;
				Device->second->Conn_.Connected = false;
				Device->second->Conn_.VerifiedCertificate = GWObjects::NO_CERTIFICATE;
			}
		}
		return false;
	}

	void DeviceRegistry::SetPendingUUID(const std::string & SerialNumber, uint64_t PendingUUID) {
		std::lock_guard		Guard(Mutex_);
		auto Device = Devices_.find(SerialNumber);
		if(Device!=Devices_.end()) {
			Device->second->Conn_.PendingUUID = PendingUUID;
		}
	}

	std::string ComputeCertificateTag( GWObjects::CertificateValidation V) {
		switch(V) {
		case GWObjects::NO_CERTIFICATE: return "no certificate";
		case GWObjects::VALID_CERTIFICATE: return "non TIP certificate";
		case GWObjects::MISMATCH_SERIAL: return "serial mismatch";
		case GWObjects::VERIFIED: return "verified";
		}
		return "unknown";
	}

	const uint64_t SECONDS_MONTH = 30*24*60*60;
	const uint64_t SECONDS_WEEK = 7*24*60*60;
	const uint64_t SECONDS_DAY = 1*24*60*60;
	const uint64_t SECONDS_HOUR = 1*24*60*60;

	std::string ComputeUpLastContactTag(uint64_t T1) {
		uint64_t T = T1 - std::time(nullptr);
		if( T>SECONDS_MONTH) return ">month";
		if( T>SECONDS_WEEK) return ">week";
		if( T>SECONDS_DAY) return ">day";
		if( T>SECONDS_HOUR) return ">hour";
		return "now";
	}

	std::string ComputeSanityTag(uint64_t T) {
		if( T==100) return "100%";
		if( T>90) return ">90%";
		if( T>60) return ">60%";
		return "<60%";
	}

	std::string ComputeUpTimeTag(uint64_t T) {
		if( T>SECONDS_MONTH) return ">month";
		if( T>SECONDS_WEEK) return ">week";
		if( T>SECONDS_DAY) return ">day";
		if( T>SECONDS_HOUR) return ">hour";
		return "now";
	}

	std::string ComputeLoadTag(uint64_t T) {
		float V=100.0*((float)T/65536.0);
		if(V<5.0) return "< 5%";
		if(V<25.0) return "< 25%";
		if(V<50.0) return "< 50%";
		if(V<75.0) return "< 75%";
		return ">75%";
	}

	std::string ComputeFreeMemoryTag(uint64_t Free, uint64_t Total) {
		float V = 100.0 * ((float)Free/(float(Total)));
		if(V<5.0) return "< 5%";
		if(V<25.0) return "< 25%";
		if(V<50.0) return "< 50%";
		if(V<75.0) return "< 75%";
		return ">75%";
	}

	bool DeviceRegistry::AnalyzeRegistry(GWObjects::Dashboard &D) {
		std::lock_guard		Guard(Mutex_);

		for(auto const &[SerialNumber,Connection]:Devices_) {
			UpdateCountedMap(D.status, Connection->Conn_.Connected ? "connected" : "not connected");
			UpdateCountedMap(D.vendors, OUIServer()->GetManufacturer(SerialNumber));
			UpdateCountedMap(D.certificates, ComputeCertificateTag(Connection->Conn_.VerifiedCertificate));
			UpdateCountedMap(D.lastContact, ComputeUpLastContactTag(Connection->Conn_.LastContact));
			UpdateCountedMap(D.healths, ComputeSanityTag(Connection->LastHealthcheck.Sanity));
			UpdateCountedMap(D.deviceType, Connection->Conn_.Compatible);
			if(!Connection->LastStats.empty()) {
				Poco::JSON::Parser	P;

				auto RawObject = P.parse(Connection->LastStats).extract<Poco::JSON::Object::Ptr>();

				if(RawObject->has("unit")) {
					auto Unit = RawObject->getObject("unit");
					if (Unit->has("uptime")) {
						UpdateCountedMap(D.upTimes, ComputeUpTimeTag(Unit->get("uptime")));
					}
					if (Unit->has("memory")) {
						auto Memory = Unit->getObject("memory");
						uint64_t Free = Memory->get("free");
						uint64_t Total = Memory->get("total");
						UpdateCountedMap(D.memoryUsed, ComputeFreeMemoryTag(Free, Total));
					}
					if (Unit->has("load")) {
						auto Load = Unit->getArray("load");
						UpdateCountedMap(D.load1,
												ComputeLoadTag(Load->getElement<uint64_t>(0)));
						UpdateCountedMap(D.load5,
												ComputeLoadTag(Load->getElement<uint64_t>(1)));
						UpdateCountedMap(D.load15,
												ComputeLoadTag(Load->getElement<uint64_t>(2)));
					}
				}
			}
		}

		return false;
	}

}  // namespace