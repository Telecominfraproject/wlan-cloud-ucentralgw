//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_RESTAPI_OBJECTS_H
#define UCENTRAL_RESTAPI_OBJECTS_H

#include "Poco/JSON/Object.h"

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
    [[nodiscard]] Poco::JSON::Object to_json() const;
    bool from_JSON(Poco::JSON::Object::Ptr Obj);
    void Print() const;
};

struct uCentralStatistics {
    std::string SerialNumber;
    uint64_t    UUID;
    std::string Values;
    uint64_t    Recorded;
    [[nodiscard]] Poco::JSON::Object to_json() const;
};

struct uCentralCapabilities {
    std::string SerialNumber;
    std::string Capabilities;
    uint64_t    FirstUpdate;
    uint64_t    LastUpdate;
    [[nodiscard]] Poco::JSON::Object to_json() const;
};

struct uCentralDeviceLog {
    std::string Log;
    uint64_t    Recorded;
    [[nodiscard]] Poco::JSON::Object to_json() const;
};

#endif //UCENTRAL_RESTAPI_OBJECTS_H
