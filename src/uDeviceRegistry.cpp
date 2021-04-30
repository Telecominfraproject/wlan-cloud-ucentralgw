//
// Created by stephane bourque on 2021-03-01.
//

#include "uDeviceRegistry.h"

#include "uCentralWebSocketServer.h"
#include "RESTAPI_handler.h"

namespace uCentral::DeviceRegistry {
    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("DeviceStatus", "DevStatus", "devicestatus") {
    }

    int Start() {
        return uCentral::DeviceRegistry::Service::instance()->Start();
    }

    void Stop() {
        uCentral::DeviceRegistry::Service::instance()->Stop();
    }

    bool GetStatistics(const std::string &SerialNumber, std::string & Statistics) {
        return uCentral::DeviceRegistry::Service::instance()->GetStatistics(SerialNumber,Statistics);
    }

    void SetStatistics(const std::string &SerialNumber, const std::string &Stats) {
        uCentral::DeviceRegistry::Service::instance()->SetStatistics(SerialNumber,Stats);
    }

    bool GetState(const std::string & SerialNumber, ConnectionState & State) {
        return uCentral::DeviceRegistry::Service::instance()->GetState(SerialNumber,State);
    }

    void SetState(const std::string & SerialNumber, ConnectionState & State) {
        return uCentral::DeviceRegistry::Service::instance()->SetState(SerialNumber,State);
    }

    ConnectionState *  Register(const std::string & SerialNumber, void *Ptr) {
        return uCentral::DeviceRegistry::Service::instance()->Register(SerialNumber,Ptr);
    }

    void UnRegister(const std::string & SerialNumber, void *Ptr) {
        uCentral::DeviceRegistry::Service::instance()->UnRegister(SerialNumber,Ptr);
    }

    bool SendCommand(uCentralCommandDetails & Command) {
        return uCentral::DeviceRegistry::Service::instance()->SendCommand(Command);
    }

    bool Connected(const std::string & SerialNumber) {
        return uCentral::DeviceRegistry::Service::instance()->Connected(SerialNumber);
    }

    int Service::Start() {
        std::lock_guard<std::mutex> guard(Mutex_);
        Logger_.notice("Starting ");
        return 0;
    }

    void Service::Stop() {
        std::lock_guard<std::mutex> guard(Mutex_);
        Logger_.notice("Stopping ");
    }

    bool Service::GetStatistics(const std::string &SerialNumber, std::string & Statistics) {
        std::lock_guard<std::mutex> guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            Statistics = Device->second.LastStats;
            return true;
        }

        return false;
    }

    void Service::SetStatistics(const std::string &SerialNumber, const std::string &Statistics) {
        std::lock_guard<std::mutex> guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            Device->second.LastStats = Statistics;
        }
    }

    bool Service::GetState(const std::string &SerialNumber, ConnectionState & State) {
        std::lock_guard<std::mutex> guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            State = *Device->second.Conn_;
            return true;
        }

        return false;
    }

    void Service::SetState(const std::string & SerialNumber, ConnectionState & State) {
        std::lock_guard<std::mutex> guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            *Device->second.Conn_ = State;
        }
    }

    ConnectionState * Service::Register(const std::string & SerialNumber, void *Ptr)
    {
        std::lock_guard<std::mutex> guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if( Device == Devices_.end()) {

            ConnectionEntry E;

            E.WSConn_ = Ptr;
            E.Conn_ = new ConnectionState;
            E.Conn_->SerialNumber = SerialNumber;
            E.Conn_->LastContact = time(nullptr);
            E.Conn_->Connected = true ;
            E.Conn_->UUID = 0 ;
            E.Conn_->MessageCount = 0 ;
            E.Conn_->Address = "";
            E.Conn_->TX = 0 ;
            E.Conn_->RX = 0;

            Devices_[SerialNumber] = E;

            return E.Conn_;
        }
        else
        {
            Device->second.WSConn_ = Ptr;
            Device->second.Conn_->Connected = true;
            Device->second.Conn_->LastContact = time(nullptr);

            return Device->second.Conn_;
        }
    }

    bool Service::Connected(const std::string & SerialNumber) {
        std::lock_guard<std::mutex> guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device == Devices_.end())
            return false;

        return Device->second.Conn_->Connected;
    }

    void Service::UnRegister(const std::string & SerialNumber, void *Ptr) {
        std::lock_guard<std::mutex> guard(Mutex_);

        auto Device = Devices_.find(SerialNumber);

        if( Device != Devices_.end() && Device->second.WSConn_==Ptr) {
            Device->second.Conn_->Address = "";
            Device->second.WSConn_ = nullptr;
            Device->second.Conn_->Connected = false;
            Device->second.Conn_->LastContact = time(nullptr);
        }
    }

    bool Service::SendCommand(uCentralCommandDetails & Cmd)
    {
        std::lock_guard<std::mutex> guard(Mutex_);

        auto Device = Devices_.find(Cmd.SerialNumber);

        try {
            if (Device != Devices_.end()) {
                if (Device->second.Conn_->Connected) {
                    if (Device->second.WSConn_ != nullptr) {
                        auto *WSConn = static_cast<uCentral::WebSocket::WSConnection *>(Device->second.WSConn_);
                        WSConn->SendCommand(Cmd);
                        return true;
                    }
                }
            }
        } catch(...) {
            std::cout << "Problem sending..." << std::endl;
        }
        return false;
    }

    Poco::JSON::Object ConnectionState::to_JSON() const
    {
        Poco::JSON::Object  Obj;

        Obj.set("serialNumber", SerialNumber);
        Obj.set("ipAddress",Address);
        Obj.set("txBytes",TX);
        Obj.set("rxBytes",RX);
        Obj.set("messageCount",MessageCount);
        Obj.set("UUID",UUID);
        Obj.set("connected",Connected);
        Obj.set("firmware",Firmware);
        Obj.set("lastContact",RESTAPIHandler::to_RFC3339(LastContact));
		Obj.set("verifiedCertificate", VerifiedCertificate);

        return Obj;
    }

}  // namespace