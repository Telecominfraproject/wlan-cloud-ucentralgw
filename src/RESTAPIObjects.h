//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_RESTAPIOBJECTS_H
#define UCENTRAL_RESTAPIOBJECTS_H

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
    Poco::JSON::Object  to_json();
    bool from_JSON(Poco::JSON::Object::Ptr Obj);
    void Print();
};

struct uCentralStatistics {
    std::string SerialNumber;
    uint64_t    UUID;
    std::string Data;
    uint64_t    Recorded;
    Poco::JSON::Object  to_json();
};

struct uCentralCapabilities {
    std::string SerialNumber;
    std::string Capabilities;
    uint64_t    FirstUpdate;
    uint64_t    LastUpdate;
    Poco::JSON::Object  to_json();
};

#endif //UCENTRAL_RESTAPIOBJECTS_H
