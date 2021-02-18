//
// Created by stephane bourque on 2021-02-17.
//

#include "EquipmentGatewayRecord.h"
#include "Poco/JSON/Parser.h"

namespace TIP::EquipmentGateway {

    bool EquipmentGatewayRecord::to_JSON(Poco::JSON::Object &obj) const {
        obj.set("id", id_);
        obj.set("hostname", hostname_);
        obj.set("ipAddr", ipAddr_);
        obj.set("port", port_);
        obj.set("gatewayType", gatewayType_);
        obj.set("createdTimestamp", createdTimestamp_);
        obj.set("lastModifiedTimestamp", lastModifiedTimestamp_);
        return true;
    }

    bool EquipmentGatewayRecord::from_object(Poco::JSON::Object::Ptr Obj) {
        Poco::DynamicStruct ds = *Obj;
        id_ = ds["id"];
        hostname_ = ds["hostname"].toString();
        ipAddr_ = ds["ipAddr"].toString();
        port_ = ds["port"];
        createdTimestamp_ = ds["createdTimestamp"];
        lastModifiedTimestamp_ = ds["lastModifiedTimestamp"];
        return true;
    }


    bool EquipmentGatewayRecord::from_stream(std::istream &response) {

        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(response).extract<Poco::JSON::Object::Ptr>();

        return EquipmentGatewayRecord::from_object(Obj);
    }
}

