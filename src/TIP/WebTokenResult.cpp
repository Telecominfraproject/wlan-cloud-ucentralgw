//
// Created by stephane bourque on 2021-02-17.
//

#include "WebTokenResult.h"
#include "Poco/JSON/Parser.h"

namespace TIP::WebToken {
    bool WebTokenResult::from_JSON(std::istream &response) {
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr ret = parser.parse(response).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *ret;

        access_token_ = ds["access_token"].toString();
        refresh_token_ = ds["refresh_token"].toString();
        token_type_ = ds["token_type"].toString();
        expires_in_ = ds["expires_in"];
        idle_timeout_ = ds["idle_timeout"];
        Poco::DynamicStruct collection = ds["aclTemplate"].extract<Poco::DynamicStruct>();
        Poco::DynamicStruct c2 = collection["aclTemplate"].extract<Poco::DynamicStruct>();
        acl_template_.ReadWriteCreate_ = c2["ReadWriteCreate"].convert<bool>();
        acl_template_.Read_ = c2["Read"].convert<bool>();
        acl_template_.Delete_ = c2["Delete"].convert<bool>();
        acl_template_.ReadWrite_ = c2["ReadWrite"].convert<bool>();
        acl_template_.PortalLogin_ = c2["PortalLogin"].convert<bool>();

        return true;
    }
}
