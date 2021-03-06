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

        int start() override;
        void stop() override;

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

        void Connect(const std::string &SerialNumber, const std::string &address);
        void Disconnect(const std::string &SerialNumber);
        std::string LastStats(const std::string &SerialNumber);
        void SetStats(const std::string &SerialNumber, const std::string &stats);

    private:
        static Service *instance_;
        std::mutex mutex_;
    };

};  // namespace

#endif //UCENTRAL_DEVICESTATUSSERVER_H
