//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_RESTAPI_OBJECTS_H
#define UCENTRAL_RESTAPI_OBJECTS_H

#include "Poco/JSON/Object.h"

struct uCentralConnectionState {
	uint64_t        MessageCount;
	std::string     SerialNumber;
	std::string     Address;
	uint64_t        UUID;
	uint64_t        PendingUUID;
	uint64_t        TX, RX;
	bool            Connected;
	uint64_t        LastContact;
	std::string     Firmware;
	bool 			VerifiedCertificate;
	void 			to_json(Poco::JSON::Object  & Obj) const;
};

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
	std::string Firmware;
	void 		to_json(Poco::JSON::Object & Obj) const;
	void 		to_json_with_status(Poco::JSON::Object & Obj) const;
    bool 		from_json(Poco::JSON::Object::Ptr Obj);
    void 		Print() const;
};

struct uCentralStatistics {
    uint64_t    UUID;
    std::string Data;
    uint64_t    Recorded;
	void 			to_json(Poco::JSON::Object  & Obj) const;
};

struct uCentralHealthCheck {
    uint64_t    UUID;
    std::string Data;
    uint64_t    Recorded;
    uint64_t    Sanity;
	void 			to_json(Poco::JSON::Object  & Obj) const;
};

struct uCentralCapabilities {
    std::string Capabilities;
    uint64_t    FirstUpdate;
    uint64_t    LastUpdate;
	void 			to_json(Poco::JSON::Object  & Obj) const;
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
	void 			to_json(Poco::JSON::Object  & Obj) const;
};

struct uCentralDefaultConfiguration {
    std::string     Name;
    std::string     Configuration;
    std::string     Models;
    std::string     Description;
    uint64_t        Created;
    uint64_t        LastModified;
	void 			to_json(Poco::JSON::Object  & Obj) const;
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
	uint64_t 		AttachSize;
	std::string 	AttachType;
	void 			to_json(Poco::JSON::Object  & Obj) const;
};

struct uCentralBlackListedDevice {
	std::string 	SerialNumber;
	std::string 	Reason;
	std::string 	Author;
	uint64_t 		Created;
	void 			to_json(Poco::JSON::Object  & Obj) const;
};

#endif //UCENTRAL_RESTAPI_OBJECTS_H
