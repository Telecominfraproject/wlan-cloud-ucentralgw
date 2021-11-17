//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#include "RESTAPI_ProvObjects.h"
#include "framework/MicroService.h"

using OpenWifi::RESTAPI_utils::field_to_json;
using OpenWifi::RESTAPI_utils::field_from_json;

namespace OpenWifi::ProvObjects {

    void ObjectInfo::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"id",id);
        field_to_json(Obj,"name",name);
        field_to_json(Obj,"description",description);
        field_to_json(Obj,"created",created);
        field_to_json(Obj,"modified",modified);
        field_to_json(Obj,"notes",notes);
        field_to_json(Obj,"tags",tags);
    }

    bool ObjectInfo::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"id",id);
            field_from_json(Obj,"name",name);
            field_from_json(Obj,"description",description);
            field_from_json(Obj,"created",created);
            field_from_json(Obj,"modified",modified);
            field_from_json(Obj,"notes",notes);
            field_from_json(Obj,"tags",tags);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ManagementPolicyEntry::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"users",users);
        field_to_json( Obj,"resources",resources);
        field_to_json( Obj,"access",access);
        field_to_json( Obj,"policy",policy);
    }

    bool ManagementPolicyEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"users",users);
            field_from_json( Obj,"resources",resources);
            field_from_json( Obj,"access",access);
            field_from_json( Obj,"policy",policy);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ManagementPolicy::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json(Obj, "entries", entries);
        field_to_json(Obj, "inUse", inUse);
        field_to_json(Obj, "entity", entity);
    }

    bool ManagementPolicy::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json(Obj, "entries", entries);
            field_from_json(Obj, "inUse", inUse);
            field_from_json(Obj, "entity", entity);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Entity::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"parent",parent);
        field_to_json( Obj,"venues",venues);
        field_to_json( Obj,"children",children);
        field_to_json( Obj,"contacts",contacts);
        field_to_json( Obj,"locations",locations);
        field_to_json( Obj,"managementPolicy",managementPolicy);
        field_to_json( Obj,"deviceConfiguration",deviceConfiguration);
        field_to_json( Obj,"devices",devices);
        field_to_json( Obj,"rrm",rrm);
        field_to_json( Obj,"sourceIP",sourceIP);
    }

    bool Entity::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"parent",parent);
            field_from_json( Obj,"venues",venues);
            field_from_json( Obj,"children",children);
            field_from_json( Obj,"contacts",contacts);
            field_from_json( Obj,"locations",locations);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"deviceConfiguration",deviceConfiguration);
            field_from_json( Obj,"devices",devices);
            field_from_json( Obj,"rrm",rrm);
            field_from_json( Obj,"sourceIP",sourceIP);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DiGraphEntry::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"parent",parent);
        field_to_json( Obj,"child",child);
    }

    bool DiGraphEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"parent",parent);
            field_from_json( Obj,"child",child);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Venue::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"parent",parent);
        field_to_json( Obj,"entity",entity);
        field_to_json( Obj,"children",children);
        field_to_json( Obj,"devices",devices);
        field_to_json( Obj,"topology",topology);
        field_to_json( Obj,"parent",parent);
        field_to_json( Obj,"design",design);
        field_to_json( Obj,"managementPolicy",managementPolicy);
        field_to_json( Obj,"deviceConfiguration",deviceConfiguration);
        field_to_json( Obj,"contact",contact);
        field_to_json( Obj,"location",location);
        field_to_json( Obj,"rrm",rrm);
        field_to_json( Obj,"sourceIP",sourceIP);
    }

    bool Venue::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"parent",parent);
            field_from_json( Obj,"entity",entity);
            field_from_json( Obj,"children",children);
            field_from_json( Obj,"devices",devices);
            field_from_json( Obj,"topology",topology);
            field_from_json( Obj,"parent",parent);
            field_from_json( Obj,"design",design);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"deviceConfiguration",deviceConfiguration);
            field_from_json( Obj,"contact",contact);
            field_from_json( Obj,"location",location);
            field_from_json( Obj,"rrm",rrm);
            field_from_json( Obj,"sourceIP",sourceIP);
            return true;
        } catch (...) {

        }
        return false;
    }

    void UserInfoDigest::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"id",id);
        field_to_json( Obj,"entity",loginId);
        field_to_json( Obj,"children",userType);
    }

    bool UserInfoDigest::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"id",id);
            field_from_json( Obj,"entity",loginId);
            field_from_json( Obj,"children",userType);
            return true;
        } catch(...) {
        }
        return false;
    }

    void ManagementRole::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"managementPolicy",managementPolicy);
        field_to_json( Obj,"users",users);
        field_to_json( Obj,"entity",entity);
    }

    bool ManagementRole::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"users",users);
            field_from_json( Obj,"entity",entity);
            return true;
        } catch(...) {
        }
        return false;
    }

    void Location::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"type",OpenWifi::ProvObjects::to_string(type));
        field_to_json( Obj,"buildingName",buildingName);
        field_to_json( Obj,"addressLines",addressLines);
        field_to_json( Obj,"city",city);
        field_to_json( Obj,"state",state);
        field_to_json( Obj,"postal",postal);
        field_to_json( Obj,"country",country);
        field_to_json( Obj,"phones",phones);
        field_to_json( Obj,"mobiles",mobiles);
        field_to_json( Obj,"geoCode",geoCode);
        field_to_json( Obj,"inUse",inUse);
        field_to_json( Obj,"entity",entity);
        field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool Location::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            std::string tmp_type;
            field_from_json( Obj,"type", tmp_type);
            type = location_from_string(tmp_type);
            field_from_json( Obj,"buildingName",buildingName);
            field_from_json( Obj,"addressLines",addressLines);
            field_from_json( Obj,"city",city);
            field_from_json( Obj,"state",state);
            field_from_json( Obj,"postal",postal);
            field_from_json( Obj,"country",country);
            field_from_json( Obj,"phones",phones);
            field_from_json( Obj,"mobiles",mobiles);
            field_from_json( Obj,"geoCode",geoCode);
            field_from_json( Obj,"inUse",inUse);
            field_from_json( Obj,"entity",entity);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Contact::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"type", to_string(type));
        field_to_json( Obj,"title",title);
        field_to_json( Obj,"salutation",salutation);
        field_to_json( Obj,"firstname",firstname);
        field_to_json( Obj,"lastname",lastname);
        field_to_json( Obj,"initials",initials);
        field_to_json( Obj,"visual",visual);
        field_to_json( Obj,"mobiles",mobiles);
        field_to_json( Obj,"phones",phones);
        field_to_json( Obj,"primaryEmail",primaryEmail);
        field_to_json( Obj,"secondaryEmail",secondaryEmail);
        field_to_json( Obj,"accessPIN",accessPIN);
        field_to_json( Obj,"inUse",inUse);
        field_to_json( Obj,"entity",entity);
        field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool Contact::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            std::string tmp_type;
            field_from_json( Obj,"type", tmp_type);
            type = contact_from_string(tmp_type);
            field_from_json( Obj,"title",title);
            field_from_json( Obj,"salutation",salutation);
            field_from_json( Obj,"firstname",firstname);
            field_from_json( Obj,"lastname",lastname);
            field_from_json( Obj,"initials",initials);
            field_from_json( Obj,"visual",visual);
            field_from_json( Obj,"mobiles",mobiles);
            field_from_json( Obj,"phones",phones);
            field_from_json( Obj,"primaryEmail",primaryEmail);
            field_from_json( Obj,"secondaryEmail",secondaryEmail);
            field_from_json( Obj,"accessPIN",accessPIN);
            field_from_json( Obj,"inUse",inUse);
            field_from_json( Obj,"entity",entity);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch (...) {

        }
        return false;
    }

    void InventoryTag::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json(Obj, "serialNumber", serialNumber);
        field_to_json(Obj, "venue", venue);
        field_to_json(Obj, "entity", entity);
        field_to_json(Obj, "subscriber", subscriber);
        field_to_json(Obj, "deviceType", deviceType);
        field_to_json(Obj, "qrCode", qrCode);
        field_to_json(Obj, "geoCode", geoCode);
        field_to_json(Obj, "location", location);
        field_to_json(Obj, "contact", contact);
        field_to_json( Obj,"deviceConfiguration",deviceConfiguration);
        field_to_json( Obj,"rrm",rrm);
        field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool InventoryTag::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"serialNumber",serialNumber);
            field_from_json( Obj,"venue",venue);
            field_from_json( Obj,"entity",entity);
            field_from_json( Obj,"subscriber",subscriber);
            field_from_json( Obj,"deviceType",deviceType);
            field_from_json(Obj, "qrCode", qrCode);
            field_from_json( Obj,"geoCode",geoCode);
            field_from_json( Obj,"location",location);
            field_from_json( Obj,"contact",contact);
            field_from_json( Obj,"deviceConfiguration",deviceConfiguration);
            field_from_json( Obj,"rrm",rrm);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceConfigurationElement::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"name", name);
        field_to_json( Obj,"description", description);
        field_to_json( Obj,"weight", weight);
        field_to_json( Obj,"configuration", configuration);
    }

    bool DeviceConfigurationElement::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"name",name);
            field_from_json( Obj,"description",description);
            field_from_json( Obj,"weight",weight);
            field_from_json( Obj,"configuration",configuration);
            return true;
        } catch(...) {

        }
        return false;
    }

    void DeviceConfiguration::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"managementPolicy",managementPolicy);
        field_to_json( Obj,"deviceTypes",deviceTypes);
        field_to_json( Obj,"configuration",configuration);
        field_to_json( Obj,"inUse",inUse);
        field_to_json( Obj,"variables",variables);
        field_to_json( Obj,"rrm",rrm);
        field_to_json( Obj,"firmwareUpgrade",firmwareUpgrade);
        field_to_json( Obj,"firmwareRCOnly",firmwareRCOnly);
    }

    bool DeviceConfiguration::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"deviceTypes",deviceTypes);
            field_from_json( Obj,"configuration",configuration);
            field_from_json( Obj,"inUse",inUse);
            field_from_json( Obj,"variables",variables);
            field_from_json( Obj,"rrm",rrm);
            field_from_json( Obj,"firmwareUpgrade",firmwareUpgrade);
            field_from_json( Obj,"firmwareRCOnly",firmwareRCOnly);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Report::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "snapshot", snapShot);
        field_to_json(Obj, "devices", tenants);
    };

    void Report::reset() {
        tenants.clear();
    }

    void ExpandedUseEntry::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "uuid", uuid);
        field_to_json(Obj, "name", name);
        field_to_json(Obj, "description", description);
    }

    bool ExpandedUseEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"uuid",uuid);
            field_from_json( Obj,"name",name);
            field_from_json( Obj,"description",description);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ExpandedUseEntryList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "type", type);
        field_to_json(Obj, "entries", entries);
    }

    bool ExpandedUseEntryList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"type",type);
            field_from_json( Obj,"entries",entries);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ExpandedUseEntryMapList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "entries", entries);
    }

    bool ExpandedUseEntryMapList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"entries",entries);
            return true;
        } catch(...) {

        }
        return false;
    }

    void UuidList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "list", list);
    }

    bool UuidList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "list", list);
            return true;
        } catch(...) {

        }
        return false;
    }

    void field_to_json(Poco::JSON::Object &Obj, const char * FieldName, ACLACCESS A) {
        switch(A) {
            case READ: Obj.set(FieldName,"read"); break;
            case MODIFY: Obj.set(FieldName,"modify"); break;
            case CREATE: Obj.set(FieldName,"create"); break;
            case DELETE: Obj.set(FieldName,"delete"); break;
            case NONE:
                default:
                    Obj.set(FieldName,"none");
        }
    }

    void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char * FieldName, ACLACCESS &A) {
        if(Obj->has(FieldName)) {
            auto V = Obj->getValue<std::string>(FieldName);
            if(V=="read")
                A = READ;
            else if(V=="modify")
                A = MODIFY;
            else if(V=="create")
                A = CREATE;
            else if(V=="delete")
                A = DELETE;
            else if(V=="none")
                A = NONE;
            else
                throw Poco::Exception("invalid JSON");
        }
    }

    void ObjectACL::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj, "users", users);
        RESTAPI_utils::field_to_json(Obj, "roles", roles);
        field_to_json(Obj, "access", access);
    }

    bool ObjectACL::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj, "users", users);
            RESTAPI_utils::field_from_json(Obj, "roles", roles);
            field_from_json(Obj, "access", access);
            return true;
        } catch(...) {

        }
        return false;
    }
    
    void ObjectACLList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj, "list", list);
    }

    bool ObjectACLList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj, "list", list);
            return true;
        } catch(...) {

        }
        return false;
    }

    std::string to_string(VISIBILITY A) {
        switch(A) {
            case PUBLIC: return "public";
            case SELECT: return "select";
            case PRIVATE:
            default:
                return "private";
        }
    }

    void field_to_json(Poco::JSON::Object &Obj, const char * FieldName, VISIBILITY A) {
        Obj.set(FieldName,to_string(A));
    }

    VISIBILITY visibility_from_string(const std::string &V) {
        if(V=="public")
            return PUBLIC;
        else if(V=="select")
            return SELECT;
        else if(V=="private")
            return PRIVATE;
        throw Poco::Exception("invalid json");
    }

    void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char * FieldName, VISIBILITY &A) {
        if(Obj->has(FieldName)) {
            auto V = Obj->getValue<std::string>(FieldName);
            A = visibility_from_string(V);
        }
    }

    void Map::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        RESTAPI_utils::field_to_json( Obj,"data",data);
        RESTAPI_utils::field_to_json( Obj,"entity",entity);
        RESTAPI_utils::field_to_json( Obj,"creator",creator);
        field_to_json( Obj,"visibility",visibility);
        RESTAPI_utils::field_to_json( Obj,"access",access);
    }

    bool Map::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"data",data);
            RESTAPI_utils::field_from_json( Obj,"entity",entity);
            RESTAPI_utils::field_from_json( Obj,"creator",creator);
            field_from_json( Obj,"visibility",visibility);
            RESTAPI_utils::field_from_json( Obj,"access",access);
            return true;
        } catch(...) {

        }
        return false;
    }

    void MapList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"list",list);
    }

    bool MapList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"list",list);
            return true;
        } catch(...) {

        }
        return false;
    }

    bool UpdateObjectInfo(const Poco::JSON::Object::Ptr &O, const SecurityObjects::UserInfo &U, ObjectInfo &I) {
        uint64_t Now = std::time(nullptr);
        if(O->has("name"))
            I.name = O->get("name").toString();

        if(I.name.empty())
            return false;

        if(O->has("description"))
            I.description = O->get("description").toString();
        SecurityObjects::MergeNotes(O,U,I.notes);
        SecurityObjects::NoteInfoVec N;
        for(auto &i:I.notes) {
            if(i.note.empty())
                continue;
            N.push_back(SecurityObjects::NoteInfo{.created=Now,.createdBy=U.email,.note=i.note});
        }
        I.modified = Now;
        return true;
    }

    bool CreateObjectInfo(const Poco::JSON::Object::Ptr &O, const SecurityObjects::UserInfo &U, ObjectInfo &I) {
        uint64_t Now = std::time(nullptr);
        if(O->has("name"))
            I.name = O->get("name").toString();

        if(I.name.empty())
            return false;

        if(O->has("description"))
            I.description = O->get("description").toString();

        SecurityObjects::NoteInfoVec N;
        for(auto &i:I.notes) {
            if(i.note.empty())
                continue;
            N.push_back(SecurityObjects::NoteInfo{.created=Now,.createdBy=U.email,.note=i.note});
        }
        I.notes = N;
        I.modified = I.created = Now;
        I.id = MicroService::CreateUUID();

        return true;
    }
}

