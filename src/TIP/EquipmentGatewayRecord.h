//
// Created by stephane bourque on 2021-02-17.
//

#ifndef UCENTRAL_EQUIPMENTGATEWAYRECORD_H
#define UCENTRAL_EQUIPMENTGATEWAYRECORD_H

#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/NetSSL.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"
#include "Poco/Net/KeyFileHandler.h"
#include "Poco/Net/InvalidCertificateHandler.h"
#include "Poco/JSON/Object.h"

#include "TIP/Api.h"

namespace TIP::Routing {

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

    bool CreateRoutingGateway(const TIP::Routing::EquipmentGatewayRecord & R);
    bool DeleteRoutingGateway(uint64_t id);
    TIP::Routing::EquipmentGatewayRecord GetRoutingGateway(uint64_t id);
    std::vector<TIP::Routing::EquipmentGatewayRecord> GetRoutingGatewaysByHost(const std::string &host);
    std::vector<TIP::Routing::EquipmentGatewayRecord> GetRoutingGatewaysByType(const std::string & Type = "CEGW");

}

#endif //UCENTRAL_EQUIPMENTGATEWAYRECORD_H
