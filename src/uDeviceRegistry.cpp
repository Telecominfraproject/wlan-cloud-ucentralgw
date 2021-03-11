//
// Created by stephane bourque on 2021-03-01.
//

#include "uDeviceRegistry.h"

#include "uCentralWebSocketServer.h"
#include "RESTAPI_Handler.h"

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

    std::shared_ptr<ConnectionState> Register(const std::string & SerialNumber, void *Ptr) {
        return uCentral::DeviceRegistry::Service::instance()->Register(SerialNumber,Ptr);
    }

    void UnRegister(const std::string & SerialNumber, void *Ptr) {
        uCentral::DeviceRegistry::Service::instance()->UnRegister(SerialNumber,Ptr);
    }

    bool SendCommand(const std::string & SerialNumber, const std::string &Cmd) {
        return uCentral::DeviceRegistry::Service::instance()->SendCommand(SerialNumber,Cmd);
    }

    int Service::Start() {
        std::lock_guard<std::mutex> guard(mutex_);

        SubSystemServer::logger().information("Starting ");

        return 0;
    }

    void Service::Stop() {
        std::lock_guard<std::mutex> guard(mutex_);

        SubSystemServer::logger().information("Stopping ");
    }

    bool Service::GetStatistics(const std::string &SerialNumber, std::string & Statistics) {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            Statistics = Device->second.LastStats;
            return true;
        }

        return false;
    }

    void Service::SetStatistics(const std::string &SerialNumber, const std::string &Statistics) {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            Device->second.LastStats = Statistics;
        }
    }

    bool Service::GetState(const std::string &SerialNumber, ConnectionState & State) {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            State = *Device->second.Conn_;
            return true;
        }

        return false;
    }

    void Service::SetState(const std::string & SerialNumber, ConnectionState & State) {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            *Device->second.Conn_ = State;
        }
    }

    std::shared_ptr<ConnectionState> Service::Register(const std::string & SerialNumber, void *Ptr)
    {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if( Device == Devices_.end()) {

            ConnectionEntry E;

            E.WSConn_ = Ptr;
            E.Conn_ = std::shared_ptr<ConnectionState>(new ConnectionState);
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

    void Service::UnRegister(const std::string & SerialNumber, void *Ptr) {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if( Device != Devices_.end()) {
            Device->second.Conn_->Address = "";
            Device->second.WSConn_ = nullptr;
            Device->second.Conn_->Connected = false;
            Device->second.Conn_->LastContact = time(nullptr);
        }
    }

    bool Service::SendCommand(const std::string & SerialNumber, const std::string &Cmd)
    {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end()) {
            if(Device->second.WSConn_!= nullptr) {
                auto *WSConn = static_cast<uCentral::WebSocket::WSConnection *>(Device->second.WSConn_);
                WSConn->SendCommand(Cmd);
                return true;
            }
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
        Obj.set("protocol",Protocol);
        Obj.set("firmware",Firmware);
        Obj.set("lastContact",RESTAPIHandler::to_RFC3339(LastContact));

        return Obj;
    }

};  // namespace