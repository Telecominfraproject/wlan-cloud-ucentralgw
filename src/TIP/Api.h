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

using Poco::Logger;

namespace TIP::API {

    template<class T> std::vector<T> GetJSONArray(std::istream &response) {
        std::vector<T>  R;
        Poco::JSON::Parser parser;
        Poco::JSON::Array::Ptr arr = parser.parse(response).extract<Poco::JSON::Array::Ptr>();

        for(auto i=0; i<arr->size() ; i++ )
        {
            Poco::JSON::Object::Ptr object = arr->getObject(i);
            T   NewObject;

            NewObject.from_object(object);

            R.push_back(NewObject);
        }

        return R;
    }

    class API {
        public:
            API():
                logger_(Logger::get("TIPAPI")) {
            }

            ~API() {
                Logout();
            }

            static API *instance() {
                if(!instance_) {
                    instance_ = new API;
                    instance_->Init();
                }
                return instance_;
            }

            bool Login();
            void Logout();
            void Init();
            const std::string & ssc_host() const { return ssc_host_; }
            uint64_t ssc_port() const { return ssc_port_; }

            const std::string &access_token() const { return token_.access_token(); };

        private:
            static API *instance_;
            std::string api_host_;       //  TIP portal server name: default to wlan-ui.wlan.local
            unsigned int api_port_;
            std::string username_;          //  TIP user name: default to "support@example.com"
            std::string password_;          //  TIP user password: default to "support"
            std::string ssc_host_;
            unsigned int ssc_port_;
            TIP::WebToken::WebTokenResult token_;
            Logger &logger_;
    };

    bool Login();
    void Logout();
    const std::string & SSC_Host();
    uint64_t SSC_Port();
}


#endif //UCENTRAL_API_H
