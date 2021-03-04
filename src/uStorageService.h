//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_USTORAGESERVICE_H
#define UCENTRAL_USTORAGESERVICE_H

#include "SubSystemServer.h"

#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/SQLite.h"

struct uCentralDevice {
    std::string SerialNumber;
    std::string DeviceType;
    std::string MACAddress;
    std::string Manufacturer;
    uint64_t    UUID;
    std::string Configuration;
    std::string Notes;
    uint64_t    CreationTimestamp;
    uint64_t    LastConfigurationChange;
    uint64_t    LastConfigurationDownload;
};

struct uCentralStatistics {
    std::string SerialNumber;
    uint64_t    UUID;
    std::string Data;
    uint64_t    Recorded;
};

struct uCentralCapabilities {
    std::string SerialNumber;
    std::string Capabilities;
    uint64_t    FirstUpdate;
    uint64_t    LastUpdate;
};

class uStorageService : public SubSystemServer {

public:
    uStorageService() noexcept;

    int start();
    void stop();

    Logger & logger() { return SubSystemServer::logger(); };

    bool AddStatisticsData(std::string &SerialNUmber, uint64_t CfgUUID, std::string &NewStats);
    bool GetStatisticsData(std::string &SerialNUmber, uint32_t From, uint32_t HowMany, std::vector<uCentralStatistics> &Stats);

    bool UpdateDeviceConfiguration(std::string &SerialNUmber, std::string &Configuration );
    bool CreateDevice(uCentralDevice &);
    bool GetDevice(std::string &SerialNUmber, uCentralDevice & );
    bool DeleteDevice(std::string &SerialNUmber);
    bool UpdateDevice(uCentralDevice &);
    bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);

    bool UpdateDeviceCapabilities(std::string &SerialNUmber, std::string &State );
    bool GetDeviceCapabilities(std::string &SerialNUmber, uCentralCapabilities & );

    static uStorageService *instance() {
        if(instance_== nullptr) {
            instance_ = new uStorageService;
        }
        return instance_;
    }

private:
    static uStorageService *instance_;
    std::shared_ptr<Poco::Data::Session>    session_;
    std::mutex mutex_;
};


#endif //UCENTRAL_USTORAGESERVICE_H
