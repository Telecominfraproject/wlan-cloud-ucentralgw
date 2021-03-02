//
// Created by stephane bourque on 2021-03-01.
//

#include "DeviceStatusServer.h"

DeviceStatusServer * DeviceStatusServer::instance_ = nullptr;

DeviceStatusServer::DeviceStatusServer() noexcept:
        SubSystemServer("DeviceStatus","DevStatus","devicestatus")
{

}

int DeviceStatusServer::start() {

    std::lock_guard<std::mutex> guard(mutex_);

    SubSystemServer::logger().information("Starting ");

    return 0;
}

void DeviceStatusServer::stop() {

    std::lock_guard<std::mutex> guard(mutex_);

    SubSystemServer::logger().information("Stopping ");

}

void DeviceStatusServer::Connect(const std::string &SerialNumber, const std::string & address)
{
    std::lock_guard<std::mutex> guard(mutex_);

}

void DeviceStatusServer::Disconnect(const std::string &SerialNumber) {
    std::lock_guard<std::mutex> guard(mutex_);

}

const std::string DeviceStatusServer::LastStats(const std::string &SerialNumber) {
    std::lock_guard<std::mutex> guard(mutex_);

    return "";
}

void DeviceStatusServer::SetStats(const std::string &SerialNumber,const std::string &stats) {
    std::lock_guard<std::mutex> guard(mutex_);

}
