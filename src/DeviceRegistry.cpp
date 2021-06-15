//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "DeviceRegistry.h"

#include "RESTAPI_handler.h"
#include "WebSocketServer.h"

#include "DeviceRegistry.h"

namespace uCentral {
    class DeviceRegistry *DeviceRegistry::instance_ = nullptr;

	DeviceRegistry::DeviceRegistry() noexcept:
		SubSystemServer("DeviceRegistry", "DevStatus", "devicestatus") {
    }

	int DeviceRegistry::Start() {
		SubMutexGuard		Guard(Mutex_);
        Logger_.notice("Starting ");
        return 0;
    }

    void DeviceRegistry::Stop() {
		SubMutexGuard		Guard(Mutex_);
        Logger_.notice("Stopping ");
    }

    bool DeviceRegistry::GetStatistics(const std::string &SerialNumber, std::string & Statistics) {
		SubMutexGuard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);
        if(Device != Devices_.end()) {
            Statistics = Device->second->LastStats;
            return true;
        }
        return false;
    }

    void DeviceRegistry::SetStatistics(const std::string &SerialNumber, const std::string &Statistics) {
		SubMutexGuard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
			Device->second->Conn_.LastContact = time(nullptr);
            Device->second->LastStats = Statistics;
        }
    }

    bool DeviceRegistry::GetState(const std::string &SerialNumber, uCentral::Objects::ConnectionState & State) {
		SubMutexGuard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            State = Device->second->Conn_;
            return true;
        }

        return false;
    }

    void DeviceRegistry::SetState(const std::string & SerialNumber, uCentral::Objects::ConnectionState & State) {
		SubMutexGuard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
			Device->second->Conn_.LastContact = time(nullptr);
            Device->second->Conn_ = State;
        }
    }

	bool DeviceRegistry::GetHealthcheck(const std::string &SerialNumber, std::string & CheckData) {
		SubMutexGuard		Guard(Mutex_);

		auto Device = Devices_.find(SerialNumber);
		if(Device != Devices_.end()) {
			CheckData = Device->second->LastHealthcheck;
			return true;
		}
		return false;
	}

	void DeviceRegistry::SetHealthcheck(const std::string &SerialNumber, const std::string &CheckData) {
		SubMutexGuard		Guard(Mutex_);

		auto Device = Devices_.find(SerialNumber);

		if(Device != Devices_.end())
		{
			Device->second->LastHealthcheck = CheckData;
		}
	}

	uCentral::Objects::ConnectionState * DeviceRegistry::Register(const std::string & SerialNumber, void *Ptr)
    {
		SubMutexGuard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

		auto Connection = static_cast<WSConnection *>(Ptr);

        if( Device == Devices_.end()) {
            auto E = std::make_unique<ConnectionEntry>();

            E->WSConn_ = Ptr;
            E->Conn_.SerialNumber = SerialNumber;
            E->Conn_.LastContact = time(nullptr);
            E->Conn_.Connected = true ;
            E->Conn_.UUID = 0 ;
            E->Conn_.MessageCount = 0 ;
            E->Conn_.Address = "";
            E->Conn_.TX = 0 ;
            E->Conn_.RX = 0;
			E->Conn_.VerifiedCertificate = Connection->CertificateValidation();
			auto R=&E->Conn_;
            Devices_[SerialNumber] = std::move(E);
            return R;
        }
        else
        {
            Device->second->WSConn_ = Ptr;
            Device->second->Conn_.Connected = true;
            Device->second->Conn_.LastContact = time(nullptr);
			Device->second->Conn_.VerifiedCertificate = Connection->CertificateValidation();
            return &Device->second->Conn_;
        }
    }

    bool DeviceRegistry::Connected(const std::string & SerialNumber) {
		SubMutexGuard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device == Devices_.end())
            return false;

        return Device->second->Conn_.Connected;
    }

    void DeviceRegistry::UnRegister(const std::string & SerialNumber, void *Ptr) {
		SubMutexGuard		Guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if( Device != Devices_.end() && Device->second->WSConn_==Ptr) {
            Device->second->Conn_.Address = "";
            Device->second->WSConn_ = nullptr;
            Device->second->Conn_.Connected = false;
            Device->second->Conn_.LastContact = time(nullptr);
			Device->second->Conn_.VerifiedCertificate = uCentral::Objects::NO_CERTIFICATE;
        }

    }

	bool DeviceRegistry::SendFrame(const std::string & SerialNumber, const std::string & Payload) {
		SubMutexGuard		Guard(Mutex_);
		auto Device = Devices_.find(SerialNumber);
		if(Device!=Devices_.end() && Device->second->WSConn_!= nullptr) {
			auto *WSConn =
				static_cast<WSConnection *>(Device->second->WSConn_);
			return WSConn->Send(Payload);
		}
		return false;
	}

	void DeviceRegistry::SetPendingUUID(const std::string & SerialNumber, uint64_t PendingUUID) {
		SubMutexGuard		Guard(Mutex_);
		auto Device = Devices_.find(SerialNumber);
		if(Device!=Devices_.end()) {
			Device->second->Conn_.PendingUUID = PendingUUID;
		}
	}

/*	bool Service::SendCommand(uCentral::Objects::CommandDetails & Cmd)
    {
		SubMutexGuard		Guard(Mutex_);

        auto Device = Devices_.find(Cmd.SerialNumber);

        try {
            if (Device != Devices_.end()) {
                if (Device->second->Conn_.Connected) {
                    if (Device->second->WSConn_ != nullptr) {
                        auto *WSConn = static_cast<uCentral::WebSocket::WSConnection *>(Device->second->WSConn_);
                        WSConn->SendCommand(Cmd);
                        return true;
                    }
                }
            }
        } catch(...) {
            Logger_.error(Poco::format("COMMAND(%s): Cannot send command %s.",Cmd.SerialNumber, Cmd.Command));
        }
        return false;
    }
*/
}  // namespace