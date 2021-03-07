//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_UDEVICEREGISTRY_H
#define UCENTRAL_UDEVICEREGISTRY_H

#include "SubSystemServer.h"

#include "Poco/Tuple.h"
#include "Poco/JSON/Object.h"

namespace uCentral::DeviceRegistry {

    struct ConnectionState {
        uint64_t    MessageCount;
        std::string SerialNumber;
        std::string Address;
        uint64_t    UUID;
        uint64_t    TX, RX;
        Poco::JSON::Object to_JSON();
    };

    class Service : public SubSystemServer {
    public:

        typedef Poco::Tuple<void *,ConnectionState,std::string> DeviceRecord;
        Service() noexcept;

        int Start() override;
        void Stop() override;

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

        bool GetStatistics(const std::string &SerialNumber, std::string & Statistics);
        void SetStatistics(const std::string &SerialNumber, const std::string &stats);
        bool GetState(const std::string & SerialNumber, ConnectionState & State);
        void SetState(const std::string & SerialNumber, const ConnectionState & State);
        void Register(const std::string & SerialNumber, void *);
        void UnRegister(const std::string & SerialNumber, void *);
        bool SendCommand(const std::string & SerialNumber, const std::string &Cmd);

    private:
        static Service                          *instance_;
        std::mutex                              mutex_;
        std::map<std::string,DeviceRecord>      Devices_;
    };

};  // namespace

#endif //UCENTRAL_UDEVICEREGISTRY_H
