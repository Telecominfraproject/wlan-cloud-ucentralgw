//
// Created by stephane bourque on 2021-07-12.
//

#include <string>

#pragma once

#include "RESTAPI_SecurityObjects.h"
#include "framework/OpenWifiTypes.h"

namespace OpenWifi::FMSObjects {

    struct Firmware {
        std::string     id;
        std::string     release;
        std::string     deviceType;
        std::string     description;
        std::string     revision;
        std::string     uri;
        std::string     image;
        uint64_t        imageDate=0;
        uint64_t        size=0;
        uint64_t        downloadCount=0;
        std::string     firmwareHash;
        std::string     owner;
        std::string     location;
        std::string     uploader;
        std::string     digest;
        bool            latest=false;
        SecurityObjects::NoteInfoVec    notes;
        uint64_t        created=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<Firmware>    FirmwareVec;

    struct FirmwareList {
        FirmwareVec  firmwares;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DeviceType {
        std::string id;
        std::string deviceType;
        std::string manufacturer;
        std::string model;
        std::string policy;
        SecurityObjects::NoteInfoVec notes;
        uint64_t lastUpdate=0;
        uint64_t created=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<DeviceType> DeviceTypeVec;

    struct DeviceTypeList {
        DeviceTypeVec   deviceTypes;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct RevisionHistoryEntry {
        std::string id;
        std::string serialNumber;
        std::string fromRelease;
        std::string toRelease;
        std::string commandUUID;
        std::string revisionId;
        uint64_t    upgraded;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<RevisionHistoryEntry>   RevisionHistoryEntryVec;

    struct RevisionHistoryEntryList {
        RevisionHistoryEntryVec history;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct FirmwareAgeDetails {
        std::string latestId;
        std::string image;
        uint64_t imageDate;
        std::string revision;
        std::string uri;
        uint64_t age=0;
        bool latest=true;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DeviceConnectionInformation {
        std::string serialNumber;
        std::string revision;
        std::string deviceType;
        std::string endPoint;
        uint64_t    lastUpdate;
        std::string status;
        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DeviceReport {
        uint64_t               snapshot=0;
        uint64_t               numberOfDevices=0;
        Types::CountedMap      OUI_;
        Types::CountedMap      Revisions_;
        Types::CountedMap      DeviceTypes_;
        Types::CountedMap      Status_;
        Types::CountedMap      EndPoints_;
        Types::CountedMap      UsingLatest_;
        Types::CountedMap      UnknownFirmwares_;
        Types::CountedMap      totalSecondsOld_;
        void to_json(Poco::JSON::Object &Obj) const;
        void reset();
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DeviceInformation {
        std::string                 serialNumber;
        RevisionHistoryEntryList    history;
        std::string                 currentFirmware;
        uint64_t                    currentFirmwareDate=0;
        std::string                 latestFirmware;
        uint64_t                    latestFirmwareDate=0;
        bool                        latestFirmwareAvailable;
        std::string                 latestFirmwareURI;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DeviceCurrentInfo {
        std::string                 serialNumber;
        std::string                 revision;
        uint64_t                    upgraded=0;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DeviceCurrentInfoList {
        std::vector<DeviceCurrentInfo>  devices;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

}
