//
// Created by stephane bourque on 2022-05-05.
//

#pragma once

#include "framework/RESTAPI_utils.h"
#include "framework/utils.h"

namespace OpenWifi {

    template<typename ContentStruct>
    struct WebSocketNotification {
        inline static uint64_t xid = 1;
        uint64_t notification_id = ++xid;
        std::string type;
        ContentStruct content;

        void to_json(Poco::JSON::Object &Obj) const;

        bool from_json(const Poco::JSON::Object::Ptr &Obj);
    };

    template<typename ContentStruct>
    void WebSocketNotification<ContentStruct>::to_json(Poco::JSON::Object &Obj) const {
        RESTAPI_utils::field_to_json(Obj, "notification_id", notification_id);
        RESTAPI_utils::field_to_json(Obj, "type", type);
        RESTAPI_utils::field_to_json(Obj, "content", content);
    }

    template<typename ContentStruct>
    bool WebSocketNotification<ContentStruct>::from_json(const Poco::JSON::Object::Ptr &Obj) {
        try {
            RESTAPI_utils::field_from_json(Obj, "notification_id", notification_id);
            RESTAPI_utils::field_from_json(Obj, "content", content);
            RESTAPI_utils::field_from_json(Obj, "type", type);
            return true;
        } catch (...) {

        }
        return false;
    }
}

