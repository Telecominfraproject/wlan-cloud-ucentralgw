//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "RESTObjects/RESTAPI_SecurityObjects.h"

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

    struct SerialNumberList {
        Types::UUIDvec_t    serialNumbers;

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
        Types::UUID_t       venue;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<ManagementPolicy>      ManagementPolicyVec;

    struct RRMAlgorithmDetails {
        std::string     name;
        std::string     parameters;
        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct RRMDetails {
        std::string     vendor;
        std::string     schedule;
        std::vector<RRMAlgorithmDetails>    algorithms;
        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct DeviceRules {
        std::string     rcOnly{"inherit"};
        std::string     rrm{"inherit"};
        std::string     firmwareUpgrade{"inherit"};

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

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
        DeviceRules             deviceRules;
        Types::StringVec        sourceIP;
        Types::UUIDvec_t        variables;
        Types::UUIDvec_t        managementPolicies;
        Types::UUIDvec_t        managementRoles;
        Types::UUIDvec_t        maps;
        Types::UUIDvec_t        configurations;

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
        Types::UUIDvec_t    contacts;
        std::string         location;
        DeviceRules         deviceRules;
        Types::StringVec    sourceIP;
        Types::UUIDvec_t    variables;
        Types::UUIDvec_t    configurations;
        Types::UUIDvec_t    maps;
        Types::UUIDvec_t    managementPolicies;
        Types::UUIDvec_t    managementRoles;
        Types::UUIDvec_t    boards;

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
        Types::UUID_t       venue;

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

    struct OperatorLocation {
        ObjectInfo          info;
        std::string         type;
        std::string         buildingName;
        Types::StringVec    addressLines;
        std::string         city;
        std::string         state;
        std::string         postal;
        std::string         country;
        Types::StringVec    phones;
        Types::StringVec    mobiles;
        std::string         geoCode;
        Types::UUID_t       operatorId;
        Types::UUID_t       subscriberDeviceId;
        Types::UUID_t       managementPolicy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };
    typedef std::vector<Location>      LocationVec;

    struct SubLocation {
        std::string         type;
        std::string         buildingName;
        Types::StringVec    addressLines;
        std::string         city;
        std::string         state;
        std::string         postal;
        std::string         country;
        Types::StringVec    phones;
        Types::StringVec    mobiles;
        std::string         geoCode;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct OperatorLocationList {
        std::vector<OperatorLocation>    locations;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };


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

    struct OperatorContact {
        ObjectInfo      info;
        std::string     type;
        std::string     title;
        std::string     salutation;
        std::string     firstname;
        std::string     lastname;
        std::string     initials;
        std::string     visual;
        Types::StringVec mobiles;
        Types::StringVec phones;
        std::string     primaryEmail;
        std::string     secondaryEmail;
        std::string     accessPIN;
        Types::UUID_t   operatorId;
        Types::UUID_t   subscriberDeviceId;
        Types::UUID_t   managementPolicy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct SubContact {
        std::string     type;
        std::string     title;
        std::string     salutation;
        std::string     firstname;
        std::string     lastname;
        std::string     initials;
        std::string     visual;
        Types::StringVec mobiles;
        Types::StringVec phones;
        std::string     primaryEmail;
        std::string     secondaryEmail;
        std::string     accessPIN;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct OperatorContactList {
        std::vector<OperatorContact>    contacts;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef std::vector<OperatorContact>      OperatorContactVec;

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
        ObjectInfo                      info;
        Types::UUID_t                   managementPolicy;
        Types::StringVec                deviceTypes;
        DeviceConfigurationElementVec   configuration;
        Types::StringVec                inUse;
        Types::UUIDvec_t                variables;
        DeviceRules                     deviceRules;
        bool                            subscriberOnly=false;
        std::string                     venue;
        std::string                     entity;
        std::string                     subscriber;

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
        DeviceRules     deviceRules;
        Types::UUID_t   managementPolicy;
        std::string     state;
        std::string     devClass;
        std::string     locale;
        std::string     realMacAddress;
        bool            doNotAllowOverrides=false;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    typedef std::vector<InventoryTag>      InventoryTagVec;

    struct InventoryTagList {
        InventoryTagVec     taglist;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct InventoryConfigApplyResult {
        std::string         appliedConfiguration;
        Types::StringVec    errors;
        Types::StringVec    warnings;
        uint64_t            errorCode;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

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
        Types::UUIDvec_t    list;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    enum ACLACCESS {
        NONE = 0, READ=1, MODIFY=2, CREATE=3, DELETE=4
    };

    struct ObjectACL {
        UuidList        users;
        UuidList        roles;
        uint64_t        access = (uint64_t) NONE;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ObjectACLList {
        std::vector<ObjectACL>  list;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Map {
        ObjectInfo          info;
        std::string         data;
        std::string         entity;
        std::string         creator;
        std::string         visibility{"private"};
        ObjectACLList       access;
        Types::UUID_t       managementPolicy;
        std::string         venue;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct MapList {
        std::vector<Map>    list;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    enum SignupStatusCodes {
        SignupCreated = 0 ,
        SignupWaitingForEmail,
        SignupWaitingForDevice,
        SignupSuccess,
        SignupFailure,
        SignupCanceled,
        SignupTimedOut
    };

    struct SignupEntry {
        ObjectInfo          info;
        std::string         email;
        std::string         userId;
        std::string         macAddress;
        std::string         serialNumber;
        uint64_t            submitted = 0 ;
        uint64_t            completed = 0 ;
        std::string         status;
        uint64_t            error=0;
        uint64_t            statusCode=0;
        std::string         deviceID;
        std::string         registrationId;
        std::string         operatorId;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Variable {
        std::string         type;
        uint64_t            weight=0;
        std::string         prefix;
        std::string         value;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct VariableList {
        std::vector<Variable>   variables;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct VariableBlock {
        ObjectInfo                  info;
        std::vector<Variable>       variables;
        std::string                 entity;
        std::string                 venue;
        std::string                 subscriber;
        std::string                 inventory;
        Types::UUIDvec_t            configurations;
        Types::UUID_t               managementPolicy;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct VariableBlockList {
        std::vector<VariableBlock>      variableBlocks;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct Operator {
        ObjectInfo                      info;
        Types::UUID_t                   managementPolicy;
        Types::UUIDvec_t                managementRoles;
        DeviceRules                     deviceRules;
        std::vector<Variable>           variables;
        bool                            defaultOperator=false;
        Types::StringVec                sourceIP;
        std::string                     registrationId;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct OperatorList {
        std::vector<Operator>            operators;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct VenueDeviceList {
        std::string         id;
        std::string         name;
        std::string         description;
        Types::UUIDvec_t    devices;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ServiceClass {
        ObjectInfo                      info;
        Types::UUID_t                   operatorId;
        Types::UUID_t                   managementPolicy;
        double                          cost=0.0;
        std::string                     currency;
        std::string                     period;
        std::string                     billingCode;
        std::vector<Variable>           variables;
        bool                            defaultService=false;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ServiceClassList {
        std::vector<ServiceClass>            serviceClasses;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ConfigurationDetails {
        DeviceConfigurationElementVec   configuration;
        std::string                     rrm{"inherit"};
        std::string                     firmwareUpgrade{"inherit"};
        std::string                     firmwareRCOnly{"inherit"};

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct SubscriberDevice {
        ObjectInfo                      info;
        std::string                     serialNumber;
        std::string                     deviceType;
        Types::UUID_t                   operatorId;
        Types::UUID_t                   subscriberId;
        SubLocation                     location;
        SubContact                      contact;
        Types::UUID_t                   managementPolicy;
        Types::UUID_t                   serviceClass;
        std::string                     qrCode;
        std::string                     geoCode;
        DeviceRules                     deviceRules;
        std::string                     state;
        std::string                     locale;
        std::string                     billingCode;
        DeviceConfigurationElementVec   configuration;
        bool                            suspended=false;
        std::string                     realMacAddress;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct SubscriberDeviceList {
        std::vector<SubscriberDevice>       subscriberDevices;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ConfigurationOverride {
        std::string     source;
        std::string     reason;
        std::string     parameterName;
        std::string     parameterType;
        std::string     parameterValue;
        std::uint64_t   modified;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    struct ConfigurationOverrideList {
        std::string     serialNumber;
        Types::UUID_t   managementPolicy;
        std::vector<ConfigurationOverride>  overrides;

        void to_json(Poco::JSON::Object &Obj) const;
        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    bool UpdateObjectInfo(const Poco::JSON::Object::Ptr &O, const SecurityObjects::UserInfo &U, ObjectInfo &I);
    bool CreateObjectInfo(const Poco::JSON::Object::Ptr &O, const SecurityObjects::UserInfo &U, ObjectInfo &I);
    bool CreateObjectInfo(const SecurityObjects::UserInfo &U, ObjectInfo &I);
};
