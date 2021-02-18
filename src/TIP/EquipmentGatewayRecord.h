//
// Created by stephane bourque on 2021-02-17.
//

#ifndef UCENTRAL_EQUIPMENTGATEWAYRECORD_H
#define UCENTRAL_EQUIPMENTGATEWAYRECORD_H

#include "Poco/JSON/Object.h"

namespace TIP::EquipmentGateway {
    class EquipmentGatewayRecord {
        public:

            EquipmentGatewayRecord() : gatewayType_("CEGW") {

            }

            bool to_JSON(Poco::JSON::Object &obj) const;

            bool from_stream(std::istream &response);
            bool from_object(Poco::JSON::Object::Ptr Obj);

            //  setters
            void id(uint64_t &v) { id_ = v; }
            void ipAddr(const std::string &v) { ipAddr_ = v; }
            void hostname(const std::string &v) { hostname_ = v; }
            void port(uint64_t v) { port_ = v; }
            void createdTimestamp(uint64_t v) { createdTimestamp_ = v; }
            void lastModifiedTimestamp(uint64_t v) { lastModifiedTimestamp_ = v; }
            void gatetwayType(const std::string &v) { gatewayType_ = v; }

            //  getters
            uint64_t id() const { return id_; }
            uint64_t port() const { return port_; }
            const std::string &ipAddr() const { return ipAddr_; }
            const std::string &hostname() const { return hostname_; }
            uint64_t createdTimestamp() const { return createdTimestamp_; }

        private:
            uint64_t id_;
            std::string ipAddr_;
            std::string hostname_;
            uint64_t port_;
            uint64_t createdTimestamp_;
            uint64_t lastModifiedTimestamp_;
            std::string gatewayType_;
    };
}

#endif //UCENTRAL_EQUIPMENTGATEWAYRECORD_H
