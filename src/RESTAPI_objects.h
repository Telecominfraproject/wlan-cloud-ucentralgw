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
	std::string Owner;
	std::string Location;
    [[nodiscard]] Poco::JSON::Object to_json() const;
    bool from_json(Poco::JSON::Object::Ptr Obj);
    void Print() const;
};

struct uCentralStatistics {
    uint64_t    UUID;
    std::string Data;
    uint64_t    Recorded;
    [[nodiscard]] Poco::JSON::Object to_json() const;
};

struct uCentralHealthcheck {
    uint64_t    UUID;
    std::string Data;
    uint64_t    Recorded;
    uint64_t    Sanity;
    [[nodiscard]] Poco::JSON::Object to_json() const;
};

struct uCentralCapabilities {
    std::string Capabilities;
    uint64_t    FirstUpdate;
    uint64_t    LastUpdate;
    [[nodiscard]] Poco::JSON::Object to_json() const;
};

struct uCentralDeviceLog {

    enum Level {
        LOG_EMERG = 0,       /* system is unusable */
        LOG_ALERT = 1,       /* action must be taken immediately */
        LOG_CRIT = 2,       /* critical conditions */
        LOG_ERR = 3,       /* error conditions */
        LOG_WARNING = 4,       /* warning conditions */
        LOG_NOTICE = 5,       /* normal but significant condition */
        LOG_INFO = 6,       /* informational */
        LOG_DEBUG = 7       /* debug-level messages */
    };

    std::string Log;
    std::string Data;
    uint64_t    Severity;
    uint64_t    Recorded;
    uint64_t    LogType;
    [[nodiscard]] Poco::JSON::Object to_json() const;
};

struct uCentralDefaultConfiguration {
    std::string     Name;
    std::string     Configuration;
    std::string     Models;
    std::string     Description;
    uint64_t        Created;
    uint64_t        LastModified;
    [[nodiscard]] Poco::JSON::Object to_json() const;
    bool from_json(Poco::JSON::Object::Ptr Obj);
};

struct uCentralCommandDetails {
    std::string     UUID;
    std::string     SerialNumber;
    std::string     Command;
    std::string     Status;
    std::string     SubmittedBy;
    std::string     Results;
    std::string     Details;
    std::string     ErrorText;
    uint64_t        Submitted;
    uint64_t        Executed;
    uint64_t        Completed;
    uint64_t        RunAt;
    uint64_t        ErrorCode;
    uint64_t        Custom;
    uint64_t        WaitingForFile;
    uint64_t        AttachDate;
    [[nodiscard]] Poco::JSON::Object to_json() const;
};

#endif //UCENTRAL_RESTAPI_OBJECTS_H
