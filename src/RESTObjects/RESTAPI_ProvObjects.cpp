//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_ProvObjects.h"
#include "framework/MicroService.h"

namespace OpenWifi::ProvObjects {

    void ObjectInfo::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj,"id",id);
        RESTAPI_utils::field_to_json(Obj,"name",name);
        RESTAPI_utils::field_to_json(Obj,"description",description);
        RESTAPI_utils::field_to_json(Obj,"created",created);
        RESTAPI_utils::field_to_json(Obj,"modified",modified);
        RESTAPI_utils::field_to_json(Obj,"notes",notes);
        RESTAPI_utils::field_to_json(Obj,"tags",tags);
    }

    bool ObjectInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj,"id",id);
            RESTAPI_utils::field_from_json(Obj,"name",name);
            RESTAPI_utils::field_from_json(Obj,"description",description);
            RESTAPI_utils::field_from_json(Obj,"created",created);
            RESTAPI_utils::field_from_json(Obj,"modified",modified);
            RESTAPI_utils::field_from_json(Obj,"notes",notes);
            RESTAPI_utils::field_from_json(Obj,"tags",tags);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ManagementPolicyEntry::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"users",users);
        RESTAPI_utils::field_to_json( Obj,"resources",resources);
        RESTAPI_utils::field_to_json( Obj,"access",access);
        RESTAPI_utils::field_to_json( Obj,"policy",policy);
    }

    bool ManagementPolicyEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"users",users);
            RESTAPI_utils::field_from_json( Obj,"resources",resources);
            RESTAPI_utils::field_from_json( Obj,"access",access);
            RESTAPI_utils::field_from_json( Obj,"policy",policy);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ManagementPolicy::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json(Obj, "entries", entries);
        RESTAPI_utils::field_to_json(Obj, "inUse", inUse);
        RESTAPI_utils::field_to_json(Obj, "entity", entity);
    }

    bool ManagementPolicy::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json(Obj, "entries", entries);
            RESTAPI_utils::field_from_json(Obj, "inUse", inUse);
            RESTAPI_utils::field_from_json(Obj, "entity", entity);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Entity::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"parent",parent);
        RESTAPI_utils::field_to_json( Obj,"venues",venues);
        RESTAPI_utils::field_to_json( Obj,"children",children);
        RESTAPI_utils::field_to_json( Obj,"contacts",contacts);
        RESTAPI_utils::field_to_json( Obj,"locations",locations);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
        RESTAPI_utils::field_to_json( Obj,"deviceConfiguration",deviceConfiguration);
        RESTAPI_utils::field_to_json( Obj,"devices",devices);
        RESTAPI_utils::field_to_json( Obj,"rrm",rrm);
        RESTAPI_utils::field_to_json( Obj,"sourceIP",sourceIP);
    }

    bool Entity::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"parent",parent);
            RESTAPI_utils::field_from_json( Obj,"venues",venues);
            RESTAPI_utils::field_from_json( Obj,"children",children);
            RESTAPI_utils::field_from_json( Obj,"contacts",contacts);
            RESTAPI_utils::field_from_json( Obj,"locations",locations);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            RESTAPI_utils::field_from_json( Obj,"deviceConfiguration",deviceConfiguration);
            RESTAPI_utils::field_from_json( Obj,"devices",devices);
            RESTAPI_utils::field_from_json( Obj,"rrm",rrm);
            RESTAPI_utils::field_from_json( Obj,"sourceIP",sourceIP);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DiGraphEntry::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"parent",parent);
        RESTAPI_utils::field_to_json( Obj,"child",child);
    }

    bool DiGraphEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"parent",parent);
            RESTAPI_utils::field_from_json( Obj,"child",child);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Venue::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"parent",parent);
        RESTAPI_utils::field_to_json( Obj,"entity",entity);
        RESTAPI_utils::field_to_json( Obj,"children",children);
        RESTAPI_utils::field_to_json( Obj,"devices",devices);
        RESTAPI_utils::field_to_json( Obj,"topology",topology);
        RESTAPI_utils::field_to_json( Obj,"parent",parent);
        RESTAPI_utils::field_to_json( Obj,"design",design);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
        RESTAPI_utils::field_to_json( Obj,"deviceConfiguration",deviceConfiguration);
        RESTAPI_utils::field_to_json( Obj,"contact",contact);
        RESTAPI_utils::field_to_json( Obj,"location",location);
        RESTAPI_utils::field_to_json( Obj,"rrm",rrm);
        RESTAPI_utils::field_to_json( Obj,"sourceIP",sourceIP);
    }

    bool Venue::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"parent",parent);
            RESTAPI_utils::field_from_json( Obj,"entity",entity);
            RESTAPI_utils::field_from_json( Obj,"children",children);
            RESTAPI_utils::field_from_json( Obj,"devices",devices);
            RESTAPI_utils::field_from_json( Obj,"topology",topology);
            RESTAPI_utils::field_from_json( Obj,"parent",parent);
            RESTAPI_utils::field_from_json( Obj,"design",design);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            RESTAPI_utils::field_from_json( Obj,"deviceConfiguration",deviceConfiguration);
            RESTAPI_utils::field_from_json( Obj,"contact",contact);
            RESTAPI_utils::field_from_json( Obj,"location",location);
            RESTAPI_utils::field_from_json( Obj,"rrm",rrm);
            RESTAPI_utils::field_from_json( Obj,"sourceIP",sourceIP);
            return true;
        } catch (...) {

        }
        return false;
    }

    void UserInfoDigest::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"id",id);
        RESTAPI_utils::field_to_json( Obj,"entity",loginId);
        RESTAPI_utils::field_to_json( Obj,"children",userType);
    }

    bool UserInfoDigest::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"id",id);
            RESTAPI_utils::field_from_json( Obj,"entity",loginId);
            RESTAPI_utils::field_from_json( Obj,"children",userType);
            return true;
        } catch(...) {
        }
        return false;
    }

    void ManagementRole::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
        RESTAPI_utils::field_to_json( Obj,"users",users);
        RESTAPI_utils::field_to_json( Obj,"entity",entity);
    }

    bool ManagementRole::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            RESTAPI_utils::field_from_json( Obj,"users",users);
            RESTAPI_utils::field_from_json( Obj,"entity",entity);
            return true;
        } catch(...) {
        }
        return false;
    }

    void Location::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"type",OpenWifi::ProvObjects::to_string(type));
        RESTAPI_utils::field_to_json( Obj,"buildingName",buildingName);
        RESTAPI_utils::field_to_json( Obj,"addressLines",addressLines);
        RESTAPI_utils::field_to_json( Obj,"city",city);
        RESTAPI_utils::field_to_json( Obj,"state",state);
        RESTAPI_utils::field_to_json( Obj,"postal",postal);
        RESTAPI_utils::field_to_json( Obj,"country",country);
        RESTAPI_utils::field_to_json( Obj,"phones",phones);
        RESTAPI_utils::field_to_json( Obj,"mobiles",mobiles);
        RESTAPI_utils::field_to_json( Obj,"geoCode",geoCode);
        RESTAPI_utils::field_to_json( Obj,"inUse",inUse);
        RESTAPI_utils::field_to_json( Obj,"entity",entity);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool Location::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            std::string tmp_type;
            RESTAPI_utils::field_from_json( Obj,"type", tmp_type);
            type = location_from_string(tmp_type);
            RESTAPI_utils::field_from_json( Obj,"buildingName",buildingName);
            RESTAPI_utils::field_from_json( Obj,"addressLines",addressLines);
            RESTAPI_utils::field_from_json( Obj,"city",city);
            RESTAPI_utils::field_from_json( Obj,"state",state);
            RESTAPI_utils::field_from_json( Obj,"postal",postal);
            RESTAPI_utils::field_from_json( Obj,"country",country);
            RESTAPI_utils::field_from_json( Obj,"phones",phones);
            RESTAPI_utils::field_from_json( Obj,"mobiles",mobiles);
            RESTAPI_utils::field_from_json( Obj,"geoCode",geoCode);
            RESTAPI_utils::field_from_json( Obj,"inUse",inUse);
            RESTAPI_utils::field_from_json( Obj,"entity",entity);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Contact::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"type", to_string(type));
        RESTAPI_utils::field_to_json( Obj,"title",title);
        RESTAPI_utils::field_to_json( Obj,"salutation",salutation);
        RESTAPI_utils::field_to_json( Obj,"firstname",firstname);
        RESTAPI_utils::field_to_json( Obj,"lastname",lastname);
        RESTAPI_utils::field_to_json( Obj,"initials",initials);
        RESTAPI_utils::field_to_json( Obj,"visual",visual);
        RESTAPI_utils::field_to_json( Obj,"mobiles",mobiles);
        RESTAPI_utils::field_to_json( Obj,"phones",phones);
        RESTAPI_utils::field_to_json( Obj,"primaryEmail",primaryEmail);
        RESTAPI_utils::field_to_json( Obj,"secondaryEmail",secondaryEmail);
        RESTAPI_utils::field_to_json( Obj,"accessPIN",accessPIN);
        RESTAPI_utils::field_to_json( Obj,"inUse",inUse);
        RESTAPI_utils::field_to_json( Obj,"entity",entity);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool Contact::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            std::string tmp_type;
            RESTAPI_utils::field_from_json( Obj,"type", tmp_type);
            type = contact_from_string(tmp_type);
            RESTAPI_utils::field_from_json( Obj,"title",title);
            RESTAPI_utils::field_from_json( Obj,"salutation",salutation);
            RESTAPI_utils::field_from_json( Obj,"firstname",firstname);
            RESTAPI_utils::field_from_json( Obj,"lastname",lastname);
            RESTAPI_utils::field_from_json( Obj,"initials",initials);
            RESTAPI_utils::field_from_json( Obj,"visual",visual);
            RESTAPI_utils::field_from_json( Obj,"mobiles",mobiles);
            RESTAPI_utils::field_from_json( Obj,"phones",phones);
            RESTAPI_utils::field_from_json( Obj,"primaryEmail",primaryEmail);
            RESTAPI_utils::field_from_json( Obj,"secondaryEmail",secondaryEmail);
            RESTAPI_utils::field_from_json( Obj,"accessPIN",accessPIN);
            RESTAPI_utils::field_from_json( Obj,"inUse",inUse);
            RESTAPI_utils::field_from_json( Obj,"entity",entity);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch (...) {

        }
        return false;
    }

    void InventoryTag::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json(Obj, "serialNumber", serialNumber);
        RESTAPI_utils::field_to_json(Obj, "venue", venue);
        RESTAPI_utils::field_to_json(Obj, "entity", entity);
        RESTAPI_utils::field_to_json(Obj, "subscriber", subscriber);
        RESTAPI_utils::field_to_json(Obj, "deviceType", deviceType);
        RESTAPI_utils::field_to_json(Obj, "qrCode", qrCode);
        RESTAPI_utils::field_to_json(Obj, "geoCode", geoCode);
        RESTAPI_utils::field_to_json(Obj, "location", location);
        RESTAPI_utils::field_to_json(Obj, "contact", contact);
        RESTAPI_utils::field_to_json( Obj,"deviceConfiguration",deviceConfiguration);
        RESTAPI_utils::field_to_json( Obj,"rrm",rrm);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool InventoryTag::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"serialNumber",serialNumber);
            RESTAPI_utils::field_from_json( Obj,"venue",venue);
            RESTAPI_utils::field_from_json( Obj,"entity",entity);
            RESTAPI_utils::field_from_json( Obj,"subscriber",subscriber);
            RESTAPI_utils::field_from_json( Obj,"deviceType",deviceType);
            RESTAPI_utils::field_from_json(Obj, "qrCode", qrCode);
            RESTAPI_utils::field_from_json( Obj,"geoCode",geoCode);
            RESTAPI_utils::field_from_json( Obj,"location",location);
            RESTAPI_utils::field_from_json( Obj,"contact",contact);
            RESTAPI_utils::field_from_json( Obj,"deviceConfiguration",deviceConfiguration);
            RESTAPI_utils::field_from_json( Obj,"rrm",rrm);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceConfigurationElement::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"name", name);
        RESTAPI_utils::field_to_json( Obj,"description", description);
        RESTAPI_utils::field_to_json( Obj,"weight", weight);
        RESTAPI_utils::field_to_json( Obj,"configuration", configuration);
    }

    bool DeviceConfigurationElement::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"name",name);
            RESTAPI_utils::field_from_json( Obj,"description",description);
            RESTAPI_utils::field_from_json( Obj,"weight",weight);
            RESTAPI_utils::field_from_json( Obj,"configuration",configuration);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceConfiguration::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"managementPolicy",managementPolicy);
        RESTAPI_utils::field_to_json( Obj,"deviceTypes",deviceTypes);
        RESTAPI_utils::field_to_json( Obj,"configuration",configuration);
        RESTAPI_utils::field_to_json( Obj,"inUse",inUse);
        RESTAPI_utils::field_to_json( Obj,"variables",variables);
        RESTAPI_utils::field_to_json( Obj,"rrm",rrm);
        RESTAPI_utils::field_to_json( Obj,"firmwareUpgrade",firmwareUpgrade);
        RESTAPI_utils::field_to_json( Obj,"firmwareRCOnly",firmwareRCOnly);
    }

    bool DeviceConfiguration::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy",managementPolicy);
            RESTAPI_utils::field_from_json( Obj,"deviceTypes",deviceTypes);
            RESTAPI_utils::field_from_json( Obj,"configuration",configuration);
            RESTAPI_utils::field_from_json( Obj,"inUse",inUse);
            RESTAPI_utils::field_from_json( Obj,"variables",variables);
            RESTAPI_utils::field_from_json( Obj,"rrm",rrm);
            RESTAPI_utils::field_from_json( Obj,"firmwareUpgrade",firmwareUpgrade);
            RESTAPI_utils::field_from_json( Obj,"firmwareRCOnly",firmwareRCOnly);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Report::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj, "snapshot", snapShot);
        RESTAPI_utils::field_to_json(Obj, "devices", tenants);
    };

    void Report::reset() {
        tenants.clear();
    }

    void ExpandedUseEntry::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj, "uuid", uuid);
        RESTAPI_utils::field_to_json(Obj, "name", name);
        RESTAPI_utils::field_to_json(Obj, "description", description);
    }

    bool ExpandedUseEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"uuid",uuid);
            RESTAPI_utils::field_from_json( Obj,"name",name);
            RESTAPI_utils::field_from_json( Obj,"description",description);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ExpandedUseEntryList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj, "type", type);
        RESTAPI_utils::field_to_json(Obj, "entries", entries);
    }

    bool ExpandedUseEntryList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"type",type);
            RESTAPI_utils::field_from_json( Obj,"entries",entries);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ExpandedUseEntryMapList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj, "entries", entries);
    }

    bool ExpandedUseEntryMapList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"entries",entries);
            return true;
        } catch(...) {

        }
        return false;
    }

    bool UpdateObjectInfo(const Poco::JSON::Object::Ptr &O, const SecurityObjects::UserInfo &U, ObjectInfo &I) {
        if(O->has("name"))
            I.name = O->get("name").toString();
        if(O->has("description"))
            I.description = O->get("description").toString();
        SecurityObjects::MergeNotes(O,U,I.notes);
        I.modified = std::time(nullptr);
        return true;
    }

};
