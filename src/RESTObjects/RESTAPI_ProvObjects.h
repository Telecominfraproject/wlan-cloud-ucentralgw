//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <string>
#include "RESTAPI_SecurityObjects.h"

namespace OpenWifi::ProvObjects {

    enum FIRMWARE_UPGRADE_RULES {
        dont_upgrade,
        upgrade_inherit,
        upgrade_release_only,
        upgrade_latest
    };

    struct ObjectInfo {
        Types::UUID_t   id;
        std::string     name;
        std::string     description;
        SecurityObjects::NoteInfoVec notes;
        uint64_t        created=0;
        uint64_t        modified=0;
        Types::TagList  tags;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ManagementPolicyEntry {
        Types::UUIDvec_t users;
        Types::UUIDvec_t resources;
        Types::StringVec access;
        std::string policy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ManagementPolicy {
        ObjectInfo          info;
        std::vector<ManagementPolicyEntry>  entries;
        Types::StringVec    inUse;
        Types::UUID_t       entity;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<ManagementPolicy>      ManagementPolicyVec;

    struct Entity {
        ObjectInfo              info;
        Types::UUID_t           parent;
        Types::UUIDvec_t        children;
        Types::UUIDvec_t        venues;
        Types::UUIDvec_t        contacts;       // all contacts associated in this entity
        Types::UUIDvec_t        locations;      // all locations associated in this entity
        Types::UUID_t           managementPolicy;
        Types::UUIDvec_t        deviceConfiguration;
        Types::UUIDvec_t        devices;
        std::string             rrm;
        Types::StringVec        sourceIP;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<Entity>      EntityVec;

    struct DiGraphEntry {
        Types::UUID_t parent;
        Types::UUID_t child;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef std::vector<DiGraphEntry>   DiGraph;

    struct Venue {
        ObjectInfo          info;
        Types::UUID_t       entity;
        Types::UUID_t       parent;
        Types::UUIDvec_t    children;
        Types::UUID_t       managementPolicy;
        Types::UUIDvec_t    devices;
        DiGraph             topology;
        std::string         design;
        Types::UUIDvec_t    deviceConfiguration;
        std::string         contact;
        std::string         location;
        std::string         rrm;
        Types::StringVec    sourceIP;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<Venue>      VenueVec;

    struct UserInfoDigest {
        std::string     id;
        std::string     loginId;
        std::string     userType;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ManagementRole {
        ObjectInfo          info;
        Types::UUID_t       managementPolicy;
        Types::UUIDvec_t    users;
        Types::StringVec    inUse;
        Types::UUID_t       entity;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<ManagementRole>      ManagementRoleVec;

    enum LocationType {
        LT_SERVICE, LT_EQUIPMENT, LT_AUTO, LT_MANUAL,
        LT_SPECIAL, LT_UNKNOWN, LT_CORPORATE
    };

    inline std::string to_string(LocationType L) {
        switch(L) {
            case LT_SERVICE: return "SERVICE";
            case LT_EQUIPMENT: return "EQUIPMENT";
            case LT_AUTO: return "AUTO";
            case LT_MANUAL: return "MANUAL";
            case LT_SPECIAL: return "SPECIAL";
            case LT_UNKNOWN: return "UNKNOWN";
            case LT_CORPORATE: return "CORPORATE";
            default: return "UNKNOWN";
        }
    }

    inline LocationType location_from_string(const std::string &S) {
        if(!Poco::icompare(S,"SERVICE"))
            return LT_SERVICE;
        else if(!Poco::icompare(S,"EQUIPMENT"))
            return LT_EQUIPMENT;
        else if(!Poco::icompare(S,"AUTO"))
            return LT_AUTO;
        else if(!Poco::icompare(S,"MANUAL"))
            return LT_MANUAL;
        else if(!Poco::icompare(S,"SPECIAL"))
            return LT_SPECIAL;
        else if(!Poco::icompare(S,"UNKNOWN"))
            return LT_UNKNOWN;
        else if(!Poco::icompare(S,"CORPORATE"))
            return LT_CORPORATE;
        return LT_UNKNOWN;
    }

    struct Location {
        ObjectInfo          info;
        LocationType        type;
        std::string         buildingName;
        Types::StringVec    addressLines;
        std::string         city;
        std::string         state;
        std::string         postal;
        std::string         country;
        Types::StringVec    phones;
        Types::StringVec    mobiles;
        std::string         geoCode;
        Types::StringVec    inUse;
        Types::UUID_t       entity;
        Types::UUID_t       managementPolicy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<Location>      LocationVec;

    enum ContactType {
        CT_SUBSCRIBER, CT_USER, CT_INSTALLER, CT_CSR, CT_MANAGER,
        CT_BUSINESSOWNER, CT_TECHNICIAN, CT_CORPORATE, CT_UNKNOWN
    };

    inline std::string to_string(ContactType L) {
        switch(L) {
            case CT_SUBSCRIBER: return "SUBSCRIBER";
            case CT_USER: return "USER";
            case CT_INSTALLER: return "INSTALLER";
            case CT_CSR: return "CSR";
            case CT_MANAGER: return "MANAGER";
            case CT_BUSINESSOWNER: return "BUSINESSOWNER";
            case CT_TECHNICIAN: return "TECHNICIAN";
            case CT_CORPORATE: return "CORPORATE";
            case CT_UNKNOWN: return "UNKNOWN";
            default: return "UNKNOWN";
        }
    }

    inline ContactType contact_from_string(const std::string &S) {
        if(!Poco::icompare(S,"SUBSCRIBER"))
            return CT_SUBSCRIBER;
        else if(!Poco::icompare(S,"USER"))
            return CT_USER;
        else if(!Poco::icompare(S,"INSTALLER"))
            return CT_INSTALLER;
        else if(!Poco::icompare(S,"CSR"))
            return CT_CSR;
        else if(!Poco::icompare(S,"BUSINESSOWNER"))
            return CT_BUSINESSOWNER;
        else if(!Poco::icompare(S,"TECHNICIAN"))
            return CT_TECHNICIAN;
        else if(!Poco::icompare(S,"CORPORATE"))
            return CT_CORPORATE;
        else if(!Poco::icompare(S,"UNKNOWN"))
            return CT_UNKNOWN;
        return CT_UNKNOWN;
    }

    struct Contact {
        ObjectInfo  info;
        ContactType type=CT_USER;
        std::string title;
        std::string salutation;
        std::string firstname;
        std::string lastname;
        std::string initials;
        std::string visual;
        Types::StringVec mobiles;
        Types::StringVec phones;
        std::string primaryEmail;
        std::string secondaryEmail;
        std::string accessPIN;
        Types::StringVec inUse;
        Types::UUID_t   entity;
        Types::UUID_t   managementPolicy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<Contact>      ContactVec;

    struct DeviceConfigurationElement {
        std::string name;
        std::string description;
        uint64_t    weight;
        std::string configuration;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<DeviceConfigurationElement> DeviceConfigurationElementVec;

    struct DeviceConfiguration {
    ObjectInfo                          info;
        Types::UUID_t                   managementPolicy;
        Types::StringVec                deviceTypes;
        DeviceConfigurationElementVec   configuration;
        Types::StringVec                inUse;
        Types::StringPairVec            variables;
        std::string                     rrm;
        std::string                     firmwareUpgrade;
        bool                            firmwareRCOnly=false;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<DeviceConfiguration>      DeviceConfigurationVec;

    struct InventoryTag {
        ObjectInfo      info;
        std::string     serialNumber;
        std::string     venue;
        std::string     entity;
        std::string     subscriber;
        std::string     deviceType;
        std::string     qrCode;
        std::string     geoCode;
        std::string     location;
        std::string     contact;
        std::string     deviceConfiguration;
        std::string     rrm;
        Types::UUID_t   managementPolicy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<InventoryTag>      InventoryTagVec;

    struct Report {
        uint64_t            snapShot=0;
        Types::CountedMap   tenants;

        void        reset();
        void to_json(Poco::JSON::Object &Obj) const;
    };

    struct ExpandedUseEntry {
        std::string uuid;
        std::string name;
        std::string description;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ExpandedUseEntryList {
        std::string                     type;
        std::vector<ExpandedUseEntry>   entries;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ExpandedUseEntryMapList {
        std::vector<ExpandedUseEntryList>    entries;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct UuidList {
        std::vector<std::string>    list;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    enum ACLACCESS {
        NONE, READ, MODIFY, CREATE, DELETE
    };

    struct ObjectACL {
        UuidList        users;
        UuidList        roles;
        ACLACCESS       access = NONE;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ObjectACLList {
        std::vector<ObjectACL>  list;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    enum VISIBILITY {
        PUBLIC, PRIVATE, SELECT
    };

    std::string to_string(VISIBILITY A);
    VISIBILITY visibility_from_string(const std::string &V);

    struct Map {
        ObjectInfo          info;
        std::string         data;
        std::string         entity;
        std::string         creator;
        VISIBILITY          visibility = PRIVATE;
        ObjectACLList       access;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct MapList {
        std::vector<Map>    list;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    bool UpdateObjectInfo(const Poco::JSON::Object::Ptr &O, const SecurityObjects::UserInfo &U, ObjectInfo &I);
    bool CreateObjectInfo(const Poco::JSON::Object::Ptr &O, const SecurityObjects::UserInfo &U, ObjectInfo &I);
};
