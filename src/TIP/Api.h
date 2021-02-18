//
// Created by stephane bourque on 2021-02-12.
//

//  This implements the various calls to the tipController

#ifndef UCENTRAL_API_H
#define UCENTRAL_API_H

#include <string>

#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Object.h"
#include "Poco/URI.h"
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
#include "Poco/Logger.h"

#include "../common.h"

#include "WebTokenResult.h"
#include "EquipmentGatewayRecord.h"

using Poco::Logger;

namespace TIP::API {
    class API {
        public:
            API() :
                    initialized_(false),
                    logger_(Logger::get("TIPAPI")) {
            }

            ~API() {
                if (initialized_)
                    Logout();
            }

            bool Login();
            void Logout();
            void Init();
            bool CreateRoutingGateway(const TIP::EquipmentGateway::EquipmentGatewayRecord & R);
            TIP::EquipmentGateway::EquipmentGatewayRecord GetRoutingGateway(uint64_t id);
            std::vector<TIP::EquipmentGateway::EquipmentGatewayRecord> GetRoutingGatewaysByHost(const std::string &host);
            std::vector<TIP::EquipmentGateway::EquipmentGatewayRecord> GetRoutingGatewaysByType(const std::string & Type = "CEGW");

            const std::string &access_token() const { return token_.access_token(); };

        private:
            bool initialized_;
            std::string api_host_;       //  TIP portal server name: default to wlan-ui.wlan.local
            unsigned int api_port_;
            std::string username_;          //  TIP user name: default to "support@example.com"
            std::string password_;          //  TIP user password: default to "support"
            std::string ssc_host_;
            unsigned int ssc_port_;
            TIP::WebToken::WebTokenResult token_;
            Logger &logger_;

        private:

    };
}


#endif //UCENTRAL_API_H
