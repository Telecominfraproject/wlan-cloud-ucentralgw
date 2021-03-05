//
// Created by stephane bourque on 2021-03-01.
//

#include "DeviceStatusServer.h"

namespace uCentral::DeviceStatus {
    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("DeviceStatus", "DevStatus", "devicestatus") {

    }

    int Service::start() {

        std::lock_guard<std::mutex> guard(mutex_);

        SubSystemServer::logger().information("Starting ");

        return 0;
    }

    void Service::stop() {

        std::lock_guard<std::mutex> guard(mutex_);

        SubSystemServer::logger().information("Stopping ");

    }

    void Service::Connect(const std::string &SerialNumber, const std::string &address) {
        std::lock_guard<std::mutex> guard(mutex_);

    }

    void Service::Disconnect(const std::string &SerialNumber) {
        std::lock_guard<std::mutex> guard(mutex_);

    }

    const std::string Service::LastStats(const std::string &SerialNumber) {
        std::lock_guard<std::mutex> guard(mutex_);

        return "";
    }

    void Service::SetStats(const std::string &SerialNumber, const std::string &stats) {
        std::lock_guard<std::mutex> guard(mutex_);

    }

};  // namespace