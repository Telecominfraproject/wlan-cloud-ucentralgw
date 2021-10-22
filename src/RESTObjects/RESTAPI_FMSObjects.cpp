//
// Created by stephane bourque on 2021-07-12.
//

#include "RESTAPI_FMSObjects.h"
#include "framework/MicroService.h"

using OpenWifi::RESTAPI_utils::field_to_json;
using OpenWifi::RESTAPI_utils::field_from_json;

namespace OpenWifi::FMSObjects {

    void Firmware::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "id", id);
        field_to_json(Obj, "release", release);
        field_to_json(Obj, "deviceType", deviceType);
        field_to_json(Obj, "description", description);
        field_to_json(Obj, "revision", revision);
        field_to_json(Obj, "uri", uri);
        field_to_json(Obj, "image", image);
        field_to_json(Obj, "imageDate", imageDate);
        field_to_json(Obj, "size", size);
        field_to_json(Obj, "downloadCount", downloadCount);
        field_to_json(Obj, "firmwareHash", firmwareHash);
        field_to_json(Obj, "owner", owner);
        field_to_json(Obj, "location", location);
        field_to_json(Obj, "uploader", uploader);
        field_to_json(Obj, "digest", digest);
        field_to_json(Obj, "latest", latest);
        field_to_json(Obj, "notes", notes);
        field_to_json(Obj, "created", created);
    };

    bool Firmware::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "id", id);
            field_from_json(Obj, "release", release);
            field_from_json(Obj, "deviceType", deviceType);
            field_from_json(Obj, "description", description);
            field_from_json(Obj, "revision", revision);
            field_from_json(Obj, "uri", uri);
            field_from_json(Obj, "image", image);
            field_from_json(Obj, "imageDate", imageDate);
            field_from_json(Obj, "size", size);
            field_from_json(Obj, "downloadCount", downloadCount);
            field_from_json(Obj, "firmwareHash", firmwareHash);
            field_from_json(Obj, "owner", owner);
            field_from_json(Obj, "location", location);
            field_from_json(Obj, "uploader", uploader);
            field_from_json(Obj, "digest", digest);
            field_from_json(Obj, "latest", latest);
            field_from_json(Obj, "notes", notes);
            field_from_json(Obj, "created", created);
            return true;
        } catch (...) {

        }
        return true;
    }

    void FirmwareList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"firmwares",firmwares);
    }

    bool FirmwareList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "firmwares", firmwares);
            return true;
        } catch (...) {

        }
        return false;
    }

    void DeviceType::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "id", id);
        field_to_json(Obj, "deviceType", deviceType);
        field_to_json(Obj, "manufacturer", manufacturer);
        field_to_json(Obj, "model", model);
        field_to_json(Obj, "policy", policy);
        field_to_json(Obj, "notes", notes);
        field_to_json(Obj, "lastUpdate", lastUpdate);
        field_to_json(Obj, "created", created);
        field_to_json(Obj, "id", id);
        field_to_json(Obj, "id", id);
        field_to_json(Obj, "id", id);
    }

    bool DeviceType::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "id", id);
            field_from_json(Obj, "deviceType", deviceType);
            field_from_json(Obj, "manufacturer", manufacturer);
            field_from_json(Obj, "model", model);
            field_from_json(Obj, "policy", policy);
            field_from_json(Obj, "notes", notes);
            field_from_json(Obj, "lastUpdate", lastUpdate);
            field_from_json(Obj, "created", created);
            field_from_json(Obj, "id", id);
            field_from_json(Obj, "id", id);
            field_from_json(Obj, "id", id);
            return true;
        } catch (...) {

        }
        return false;
    }

    void DeviceTypeList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"deviceTypes", deviceTypes);
    }

    bool DeviceTypeList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"deviceTypes", deviceTypes);
            return true;
        } catch(...) {

        }
        return false;
    }

    void RevisionHistoryEntry::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "id", id);
        field_to_json(Obj, "serialNumber", serialNumber);
        field_to_json(Obj, "fromRelease", fromRelease);
        field_to_json(Obj, "toRelease", toRelease);
        field_to_json(Obj, "commandUUID", commandUUID);
        field_to_json(Obj, "revisionId", revisionId);
        field_to_json(Obj, "upgraded", upgraded);
    }

    bool RevisionHistoryEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "id", id);
            field_from_json(Obj, "serialNumber", serialNumber);
            field_from_json(Obj, "fromRelease", fromRelease);
            field_from_json(Obj, "toRelease", toRelease);
            field_from_json(Obj, "commandUUID", commandUUID);
            field_from_json(Obj, "revisionId", revisionId);
            field_from_json(Obj, "upgraded", upgraded);
            return true;
        } catch(...) {

        }
        return false;
    }

    void RevisionHistoryEntryList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"deviceTypes", history);
    }

    bool RevisionHistoryEntryList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"deviceTypes", history);
            return true;
        } catch(...) {

        }
        return false;
    }

    void FirmwareAgeDetails::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"latestId", latestId);
        field_to_json(Obj,"image", image);
        field_to_json(Obj,"imageDate", imageDate);
        field_to_json(Obj,"revision", revision);
        field_to_json(Obj,"uri", uri);
        field_to_json(Obj,"age", age);
        field_to_json(Obj,"latest",latest);
    }

    bool FirmwareAgeDetails::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"latestId", latestId);
            field_from_json(Obj,"image", image);
            field_from_json(Obj,"imageDate", imageDate);
            field_from_json(Obj,"revision", revision);
            field_from_json(Obj,"uri", uri);
            field_from_json(Obj,"age", age);
            field_from_json(Obj,"latest", latest);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceConnectionInformation::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "serialNumber", serialNumber);
        field_to_json(Obj, "revision", revision);
        field_to_json(Obj, "deviceType", deviceType);
        field_to_json(Obj, "endPoint", endPoint);
        field_to_json(Obj, "lastUpdate", lastUpdate);
        field_to_json(Obj, "status", status);
    }

    bool DeviceConnectionInformation::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "serialNumber", serialNumber);
            field_from_json(Obj, "revision", revision);
            field_from_json(Obj, "deviceType", deviceType);
            field_from_json(Obj, "endPoint", endPoint);
            field_from_json(Obj, "lastUpdate", lastUpdate);
            field_from_json(Obj, "status", status);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceReport::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "ouis",OUI_);
        field_to_json(Obj, "revisions", Revisions_);
        field_to_json(Obj, "deviceTypes", DeviceTypes_);
        field_to_json(Obj, "status", Status_);
        field_to_json(Obj, "endPoints", EndPoints_);
        field_to_json(Obj, "usingLatest", UsingLatest_);
        field_to_json(Obj, "unknownFirmwares", UnknownFirmwares_);
        field_to_json(Obj,"snapshot",snapshot);
        field_to_json(Obj,"numberOfDevices",numberOfDevices);
        field_to_json(Obj, "totalSecondsOld", totalSecondsOld_);
    }

    void DeviceReport::reset() {
        OUI_.clear();
        Revisions_.clear();
        DeviceTypes_.clear();
        Status_.clear();
        EndPoints_.clear();
        UsingLatest_.clear();
        UnknownFirmwares_.clear();
        totalSecondsOld_.clear();
        numberOfDevices = 0 ;
        snapshot = std::time(nullptr);
    }

    bool DeviceReport::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {

            return true;
        } catch (...) {

        }
        return false;
    }
}
