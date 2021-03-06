//
// Created by stephane bourque on 2021-03-01.
//

#include "uDeviceRegistry.h"

#include "uCentralWebSocketServer.h"

namespace uCentral::DeviceRegistry {
    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("DeviceStatus", "DevStatus", "devicestatus") {

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
            Statistics = Device->second.get<2>();
            return true;
        }

        return false;
    }

    void Service::SetStatistics(const std::string &SerialNumber, const std::string &Statistics) {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            Device->second.set<2>(Statistics);
        }
    }

    void Service::SetState(const std::string &SerialNumber, const ConnectionState & State) {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            Device->second.set<1>(State);
        }
    }

    bool Service::GetState(const std::string &SerialNumber, ConnectionState & State) {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end())
        {
            State = Device->second.get<1>();
            return true;
        }

        return false;
    }

    void Service::Register(const std::string & SerialNumber, void *Ptr)
    {
        std::lock_guard<std::mutex> guard(mutex_);

        DeviceRecord R;

        R.set<0>(Ptr);
        R.set<1>(ConnectionState());
        R.set<2>("");

        Devices_[SerialNumber] = R;
    }

    void Service::UnRegister(const std::string & SerialNumber, void *Ptr) {
        std::lock_guard<std::mutex> guard(mutex_);

        Devices_.erase(SerialNumber);
    }

    bool Service::SendCommand(const std::string & SerialNumber, const std::string &Cmd)
    {
        std::lock_guard<std::mutex> guard(mutex_);

        auto Device = Devices_.find(SerialNumber);

        if(Device != Devices_.end()) {
            auto *WSConn = static_cast<uCentral::WebSocket::WSConnection *>(Device->second.get<0>());

            WSConn->SendCommand(Cmd);

            return true;
        }

        return false;
    }



};  // namespace