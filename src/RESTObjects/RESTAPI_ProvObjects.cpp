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
        field_to_json( Obj,"deviceRules",deviceRules);
        field_to_json( Obj,"sourceIP",sourceIP);
        field_to_json( Obj,"variables", variables);
        field_to_json( Obj,"managementPolicies", managementPolicies);
        field_to_json( Obj,"managementRoles", managementRoles);
        field_to_json( Obj,"maps", maps);
        field_to_json( Obj,"configurations", configurations);
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
            field_from_json( Obj,"deviceRules",deviceRules);
            field_from_json( Obj,"sourceIP",sourceIP);
            field_from_json( Obj,"variables", variables);
            field_from_json( Obj,"managementPolicies", managementPolicies);
            field_from_json( Obj,"managementRoles", managementRoles);
            field_from_json( Obj,"maps", maps);
            field_from_json( Obj,"configurations", configurations);
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
        field_to_json( Obj,"contacts",contacts);
        field_to_json( Obj,"location",location);
        field_to_json( Obj,"deviceRules",deviceRules);
        field_to_json( Obj,"sourceIP",sourceIP);
        field_to_json( Obj,"variables", variables);
        field_to_json( Obj,"managementPolicies", managementPolicies);
        field_to_json( Obj,"managementRoles", managementRoles);
        field_to_json( Obj,"maps", maps);
        field_to_json( Obj,"configurations", configurations);
        field_to_json( Obj,"boards", boards);
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
            field_from_json( Obj,"contacts",contacts);
            field_from_json( Obj,"location",location);
            field_from_json( Obj,"deviceRules",deviceRules);
            field_from_json( Obj,"sourceIP",sourceIP);
            field_from_json( Obj,"variables", variables);
            field_from_json( Obj,"managementPolicies", managementPolicies);
            field_from_json( Obj,"managementRoles", managementRoles);
            field_from_json( Obj,"maps", maps);
            field_from_json( Obj,"configurations", configurations);
            field_from_json( Obj,"boards", boards);
            return true;
        } catch (...) {

        }
        return false;
    }

    void Operator::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"managementPolicy",managementPolicy);
        field_to_json( Obj,"managementRoles",managementRoles);
        field_to_json( Obj,"deviceRules",deviceRules);
        field_to_json( Obj,"variables",variables);
        field_to_json( Obj,"defaultOperator",defaultOperator);
        field_to_json( Obj,"sourceIP",sourceIP);
        field_to_json( Obj,"registrationId",registrationId);
    }

    bool Operator::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"managementRoles",managementRoles);
            field_from_json( Obj,"deviceRules",deviceRules);
            field_from_json( Obj,"variables",variables);
            field_from_json( Obj,"defaultOperator",defaultOperator);
            field_from_json( Obj,"sourceIP",sourceIP);
            field_from_json( Obj,"registrationId",registrationId);
            return true;
        } catch(...) {
        }
        return false;
    }

    void OperatorList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"operators",operators);
    }

    bool OperatorList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"operators",operators);
            return true;
        } catch(...) {
        }
        return false;
    }

    void ServiceClass::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"operatorId",operatorId);
        field_to_json( Obj,"managementPolicy",managementPolicy);
        field_to_json( Obj,"cost",cost);
        field_to_json( Obj,"currency",currency);
        field_to_json( Obj,"period",period);
        field_to_json( Obj,"billingCode",billingCode);
        field_to_json( Obj,"variables",variables);
        field_to_json( Obj,"defaultService",defaultService);
    }

    bool ServiceClass::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"operatorId",operatorId);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"cost",cost);
            field_from_json( Obj,"currency",currency);
            field_from_json( Obj,"period",period);
            field_from_json( Obj,"billingCode",billingCode);
            field_from_json( Obj,"variables",variables);
            field_from_json( Obj,"defaultService",defaultService);
            return true;
        } catch(...) {
        }
        return false;

    }

    void ServiceClassList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"serviceClasses",serviceClasses);
    }

    bool ServiceClassList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"serviceClasses",serviceClasses);
            return true;
        } catch(...) {
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
        field_to_json( Obj,"venue",venue);
    }

    bool ManagementRole::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"users",users);
            field_from_json( Obj,"entity",entity);
            field_from_json( Obj,"venue",venue);
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

    void OperatorLocation::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"type",type);
        field_to_json( Obj,"buildingName",buildingName);
        field_to_json( Obj,"addressLines",addressLines);
        field_to_json( Obj,"city",city);
        field_to_json( Obj,"state",state);
        field_to_json( Obj,"postal",postal);
        field_to_json( Obj,"country",country);
        field_to_json( Obj,"phones",phones);
        field_to_json( Obj,"mobiles",mobiles);
        field_to_json( Obj,"geoCode",geoCode);
        field_to_json( Obj,"operatorId",operatorId);
        field_to_json( Obj,"subscriberDeviceId",subscriberDeviceId);
        field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool OperatorLocation::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"type", type);
            field_from_json( Obj,"buildingName",buildingName);
            field_from_json( Obj,"addressLines",addressLines);
            field_from_json( Obj,"city",city);
            field_from_json( Obj,"state",state);
            field_from_json( Obj,"postal",postal);
            field_from_json( Obj,"country",country);
            field_from_json( Obj,"phones",phones);
            field_from_json( Obj,"mobiles",mobiles);
            field_from_json( Obj,"geoCode",geoCode);
            field_from_json( Obj,"operatorId",operatorId);
            field_from_json( Obj,"subscriberDeviceId",subscriberDeviceId);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch (...) {

        }
        return false;
    }

    void SubLocation::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"type",type);
        field_to_json( Obj,"buildingName",buildingName);
        field_to_json( Obj,"addressLines",addressLines);
        field_to_json( Obj,"city",city);
        field_to_json( Obj,"state",state);
        field_to_json( Obj,"postal",postal);
        field_to_json( Obj,"country",country);
        field_to_json( Obj,"phones",phones);
        field_to_json( Obj,"mobiles",mobiles);
        field_to_json( Obj,"geoCode",geoCode);
    }

    bool SubLocation::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"type", type);
            field_from_json( Obj,"buildingName",buildingName);
            field_from_json( Obj,"addressLines",addressLines);
            field_from_json( Obj,"city",city);
            field_from_json( Obj,"state",state);
            field_from_json( Obj,"postal",postal);
            field_from_json( Obj,"country",country);
            field_from_json( Obj,"phones",phones);
            field_from_json( Obj,"mobiles",mobiles);
            field_from_json( Obj,"geoCode",geoCode);
            return true;
        } catch (...) {

        }
        return false;
    }

    void OperatorLocationList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj, "locations", locations);
    }

    bool OperatorLocationList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj, "locations", locations);
            return true;
        } catch(...) {

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

    void OperatorContact::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"type", type);
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
        field_to_json( Obj,"operatorId",operatorId);
        field_to_json( Obj,"subscriberDeviceId",subscriberDeviceId);
        field_to_json( Obj,"managementPolicy",managementPolicy);
    }

    bool OperatorContact::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"type", type);
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
            field_from_json( Obj,"operatorId",operatorId);
            field_from_json( Obj,"subscriberDeviceId",subscriberDeviceId);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            return true;
        } catch (...) {

        }
        return false;
    }

    void SubContact::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"type", type);
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
    }

    bool SubContact::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"type", type);
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
            return true;
        } catch (...) {

        }
        return false;
    }

    void OperatorContactList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj, "contacts", contacts);
    }

    bool OperatorContactList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj, "contacts", contacts);
            return true;
        } catch(...) {

        }
        return false;
    }

    void InventoryTag::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj, "serialNumber", serialNumber);
        field_to_json( Obj, "venue", venue);
        field_to_json( Obj, "entity", entity);
        field_to_json( Obj, "subscriber", subscriber);
        field_to_json( Obj, "deviceType", deviceType);
        field_to_json( Obj, "qrCode", qrCode);
        field_to_json( Obj, "geoCode", geoCode);
        field_to_json( Obj, "location", location);
        field_to_json( Obj, "contact", contact);
        field_to_json( Obj, "deviceConfiguration",deviceConfiguration);
        field_to_json( Obj,"deviceRules",deviceRules);
        field_to_json( Obj, "managementPolicy",managementPolicy);
        field_to_json( Obj, "state",state);
        field_to_json( Obj, "devClass",devClass);
        field_to_json( Obj, "locale",locale);
        field_to_json( Obj, "realMacAddress",realMacAddress);
    }

    bool InventoryTag::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"serialNumber",serialNumber);
            field_from_json( Obj,"venue",venue);
            field_from_json( Obj,"entity",entity);
            field_from_json( Obj,"subscriber",subscriber);
            field_from_json( Obj,"deviceType",deviceType);
            field_from_json( Obj,"qrCode", qrCode);
            field_from_json( Obj,"geoCode",geoCode);
            field_from_json( Obj,"location",location);
            field_from_json( Obj,"contact",contact);
            field_from_json( Obj,"deviceConfiguration",deviceConfiguration);
            field_from_json( Obj,"deviceRules",deviceRules);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"state",state);
            field_from_json( Obj,"devClass",devClass);
            field_from_json( Obj,"locale",locale);
            field_from_json( Obj,"realMacAddress",realMacAddress);
            return true;
        } catch(...) {

        }
        return false;
    }

    void InventoryConfigApplyResult::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj, "appliedConfiguration", appliedConfiguration);
        field_to_json( Obj, "warnings", warnings);
        field_to_json( Obj, "errors", errors);
        field_to_json( Obj, "errorCode", errorCode);
    }

    bool InventoryConfigApplyResult::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj, "appliedConfiguration", appliedConfiguration);
            field_from_json( Obj, "warnings", warnings);
            field_from_json( Obj, "errors", errors);
            field_from_json( Obj, "errorCode", errorCode);
            return true;
        } catch (...) {

        }
        return false;
    }

    void InventoryTagList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"taglist",taglist);
    }

    bool InventoryTagList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"taglist",taglist);
            return true;
        } catch (...) {

        }
        return false;
    };

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
        field_to_json( Obj,"subscriberOnly",subscriberOnly);
        field_to_json( Obj,"entity", entity);
        field_to_json( Obj,"venue", venue);
        field_to_json( Obj,"subscriber", subscriber);
        field_to_json( Obj,"configuration",configuration);
        field_to_json( Obj,"inUse",inUse);
        field_to_json( Obj,"variables",variables);
        field_to_json( Obj,"deviceRules",deviceRules);
    }

    bool DeviceConfiguration::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"managementPolicy",managementPolicy);
            field_from_json( Obj,"deviceTypes",deviceTypes);
            field_from_json( Obj,"inUse",inUse);
            field_from_json( Obj,"variables",variables);
            field_from_json( Obj,"subscriberOnly",subscriberOnly);
            field_from_json( Obj,"entity", entity);
            field_from_json( Obj,"venue", venue);
            field_from_json( Obj,"subscriber", subscriber);
            field_from_json( Obj,"configuration",configuration);
            field_from_json( Obj,"deviceRules",deviceRules);
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

    void ObjectACL::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "users", users);
        field_to_json(Obj, "roles", roles);
        field_to_json(Obj, "access", access);
    }

    bool ObjectACL::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "users", users);
            field_from_json(Obj, "roles", roles);
            field_from_json(Obj, "access", access);
            return true;
        } catch(...) {

        }
        return false;
    }
    
    void ObjectACLList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj, "list", list);
    }

    bool ObjectACLList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj, "list", list);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Map::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"data",data);
        field_to_json( Obj,"entity",entity);
        field_to_json( Obj,"creator",creator);
        field_to_json( Obj,"visibility",visibility);
        field_to_json( Obj,"access",access);
        field_to_json( Obj,"managementPolicy", managementPolicy);
        field_to_json( Obj,"venue", venue);
    }

    bool Map::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            RESTAPI_utils::field_from_json( Obj,"data",data);
            RESTAPI_utils::field_from_json( Obj,"entity",entity);
            RESTAPI_utils::field_from_json( Obj,"creator",creator);
            RESTAPI_utils::field_from_json( Obj,"visibility",visibility);
            RESTAPI_utils::field_from_json( Obj,"access",access);
            RESTAPI_utils::field_from_json( Obj,"managementPolicy", managementPolicy);
            RESTAPI_utils::field_from_json( Obj,"venue", venue);
            return true;
        } catch(...) {

        }
        return false;
    }

    void SerialNumberList::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json( Obj,"serialNumbers",serialNumbers);
    }

    bool SerialNumberList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json( Obj,"serialNumbers",serialNumbers);
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

    void SignupEntry::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"email", email);
        field_to_json( Obj,"userId", userId);
        field_to_json( Obj,"macAddress", macAddress);
        field_to_json( Obj,"serialNumber", serialNumber);
        field_to_json( Obj,"submitted", submitted);
        field_to_json( Obj,"completed", completed);
        field_to_json( Obj,"status", status);
        field_to_json( Obj,"error", error);
        field_to_json( Obj,"statusCode", statusCode);
        field_to_json( Obj,"deviceID", deviceID);
        field_to_json( Obj,"registrationId",registrationId);
        field_to_json( Obj,"operatorId",operatorId);
    }

    bool SignupEntry::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"email", email);
            field_from_json( Obj,"userId", userId);
            field_from_json( Obj,"macAddress", macAddress);
            field_from_json( Obj,"serialNumber", serialNumber);
            field_from_json( Obj,"submitted", submitted);
            field_from_json( Obj,"completed", completed);
            field_from_json( Obj,"status", status);
            field_from_json( Obj,"error", error);
            field_from_json( Obj,"statusCode", statusCode);
            field_from_json( Obj,"deviceID", deviceID);
            field_from_json( Obj,"registrationId",registrationId);
            field_from_json( Obj,"operatorId",operatorId);
            return true;
        } catch(...) {

        }
        return false;
    }

    void Variable::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"type", type);
        field_to_json( Obj,"weight", weight);
        field_to_json( Obj,"prefix", prefix);
        field_to_json( Obj,"value", value);
    }

    bool Variable::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"type", type);
            field_from_json( Obj,"weight", weight);
            field_from_json( Obj,"prefix", prefix);
            field_from_json( Obj,"value", value);
            return true;
        } catch(...) {

        }
        return false;
    }

    void VariableList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"variables", variables);
    }

    bool VariableList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"variables", variables);
            return true;
        } catch(...) {

        }
        return false;
    }

    void VariableBlock::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"variables", variables);
        field_to_json( Obj,"entity", entity);
        field_to_json( Obj,"venue", venue);
        field_to_json( Obj,"subscriber", subscriber);
        field_to_json( Obj,"inventory", inventory);
        field_to_json( Obj,"configurations", configurations);
        field_to_json( Obj,"managementPolicy", managementPolicy);
    }

    bool VariableBlock::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"variables", variables);
            field_from_json( Obj,"entity", entity);
            field_from_json( Obj,"venue", venue);
            field_from_json( Obj,"subscriber", subscriber);
            field_from_json( Obj,"inventory", inventory);
            field_from_json( Obj,"configurations", configurations);
            field_from_json( Obj,"managementPolicy", managementPolicy);
            return true;
        } catch(...) {
        }
        return false;
    }

    void VariableBlockList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"variableBlocks", variableBlocks);
    }

    bool VariableBlockList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"variableBlocks", variableBlocks);
            return true;
        } catch(...) {

        }
        return false;
    }

    void ConfigurationDetails::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"configuration", configuration);
        field_to_json( Obj,"rrm", rrm);
        field_to_json( Obj,"firmwareRCOnly", firmwareRCOnly);
        field_to_json( Obj,"firmwareUpgrade", firmwareUpgrade);
    }

    bool ConfigurationDetails::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"configuration", configuration);
            field_from_json( Obj,"rrm", rrm);
            field_from_json( Obj,"firmwareRCOnly", firmwareRCOnly);
            field_from_json( Obj,"firmwareUpgrade", firmwareUpgrade);
            return true;
        } catch(...) {

        }
        return false;
    }

    void SubscriberDevice::to_json(Poco::JSON::Object &Obj) const {
        info.to_json(Obj);
        field_to_json( Obj,"serialNumber", serialNumber);
        field_to_json( Obj,"deviceType", deviceType);
        field_to_json( Obj,"operatorId", operatorId);
        field_to_json( Obj,"subscriberId", subscriberId);
        field_to_json( Obj,"location", location);
        field_to_json( Obj,"contact", contact);
        field_to_json( Obj,"managementPolicy", managementPolicy);
        field_to_json( Obj,"serviceClass", serviceClass);
        field_to_json( Obj,"qrCode", qrCode);
        field_to_json( Obj,"geoCode", geoCode);
        field_to_json( Obj,"deviceRules",deviceRules);
        field_to_json( Obj,"state", state);
        field_to_json( Obj,"locale", locale);
        field_to_json( Obj,"billingCode", billingCode);
        field_to_json( Obj,"configuration", configuration);
        field_to_json( Obj,"suspended", suspended);
        field_to_json( Obj,"realMacAddress", realMacAddress);
    }

    bool SubscriberDevice::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            info.from_json(Obj);
            field_from_json( Obj,"serialNumber", serialNumber);
            field_from_json( Obj,"deviceType", deviceType);
            field_from_json( Obj,"operatorId", operatorId);
            field_from_json( Obj,"subscriberId", subscriberId);
            field_from_json( Obj,"location", location);
            field_from_json( Obj,"contact", contact);
            field_from_json( Obj,"managementPolicy", managementPolicy);
            field_from_json( Obj,"serviceClass", serviceClass);
            field_from_json( Obj,"qrCode", qrCode);
            field_from_json( Obj,"geoCode", geoCode);
            field_from_json( Obj,"deviceRules",deviceRules);
            field_from_json( Obj,"state", state);
            field_from_json( Obj,"locale", locale);
            field_from_json( Obj,"billingCode", billingCode);
            field_from_json( Obj,"configuration", configuration);
            field_from_json( Obj,"suspended", suspended);
            field_from_json( Obj,"realMacAddress", realMacAddress);
            return true;
        } catch(...) {

        }
        return false;
    }

    void SubscriberDeviceList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json( Obj,"subscriberDevices", subscriberDevices);
    }

    bool SubscriberDeviceList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json( Obj,"subscriberDevices", subscriberDevices);
            return true;
        } catch(...) {

        }
        return false;
    }

    void VenueDeviceList::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"id",id);
        field_to_json(Obj,"name",name);
        field_to_json(Obj,"description",description);
        field_to_json(Obj,"devices",devices);
    }

    bool VenueDeviceList::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"id",id);
            field_from_json(Obj,"name",name);
            field_from_json(Obj,"description",description);
            field_from_json(Obj,"devices",devices);
            return true;
        } catch(...) {

        }
        return false;
    }

    bool UpdateObjectInfo(const Poco::JSON::Object::Ptr &O, const SecurityObjects::UserInfo &U, ObjectInfo &I) {
        uint64_t Now = OpenWifi::Now();
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
        uint64_t Now = OpenWifi::Now();
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

    bool CreateObjectInfo([[maybe_unused]] const SecurityObjects::UserInfo &U, ObjectInfo &I) {
        I.modified = I.created = OpenWifi::Now();
        I.id = MicroService::CreateUUID();
        return true;
    }

    void DeviceRules::to_json(Poco::JSON::Object &Obj) const {
        field_to_json(Obj,"rcOnly",rcOnly);
        field_to_json(Obj,"rrm",rrm);
        field_to_json(Obj,"firmwareUpgrade",firmwareUpgrade);
    }

    bool DeviceRules::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            field_from_json(Obj,"rcOnly",rcOnly);
            field_from_json(Obj,"rrm",rrm);
            field_from_json(Obj,"firmwareUpgrade",firmwareUpgrade);
            return true;
        } catch(...) {

        }
        return false;
    }

}

