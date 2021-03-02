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

            [[nodiscard]] const std::string & ssc_host() const { return ssc_host_; }
            [[nodiscard]] uint64_t ssc_port() const { return ssc_port_; }
            [[nodiscard]] const std::string &access_token() const { return token_.access_token(); };

            [[nodiscard]] const std::string &refresh_token() const { return token_.refresh_token(); };
            [[nodiscard]] const std::string &id_token() const { return token_.id_token(); };
            [[nodiscard]] const std::string &token_type() const { return token_.token_type(); };
            [[nodiscard]] unsigned int expires_in() const { return token_.expires_in(); };
            [[nodiscard]] unsigned int idle_timeout() const { return token_.idle_timeout(); };

            [[nodiscard]] bool read_access() const { return token_.read_access(); };
            [[nodiscard]] bool readWrite_access() const { return token_.readWrite_access(); };
            [[nodiscard]] bool readWriteCreate_access() const { return token_.readWriteCreate_access(); };
            [[nodiscard]] bool delete_access() const { return token_.delete_access(); };
            [[nodiscard]] bool portalLogin_access() const { return token_.portalLogin_access(); };


        private:
            void Init();

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
    const std::string & AccessToken();
    const std::string & SSC_Host();
    uint64_t SSC_Port();
}


#endif //UCENTRAL_API_H
