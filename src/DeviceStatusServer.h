//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_DEVICESTATUSSERVER_H
#define UCENTRAL_DEVICESTATUSSERVER_H

#include "SubSystemServer.h"

namespace uCentral::DeviceStatus {

    class Service : public SubSystemServer {
    public:
        Service() noexcept;

        int start();

        void stop();

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

        std::string process_message(const char *msg);

        void Connect(const std::string &SerialNumber, const std::string &address);

        void Disconnect(const std::string &SerialNumber);

        const std::string LastStats(const std::string &SerialNumber);

        void SetStats(const std::string &SerialNumber, const std::string &stats);

        Logger &logger() { return SubSystemServer::logger(); };

    private:
        static Service *instance_;
        std::mutex mutex_;
    };

};  // namespace

#endif //UCENTRAL_DEVICESTATUSSERVER_H
