//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_UAUTHSERVICE_H
#define UCENTRAL_UAUTHSERVICE_H

#include "SubSystemServer.h"

#include "Poco/JSON/Object.h"

namespace uCentral::Auth {

    struct AclTemplate {
        bool Read_;
        bool ReadWrite_;
        bool ReadWriteCreate_;
        bool Delete_;
        bool PortalLogin_;

        Poco::JSON::Object to_JSON();
    };

    struct WebToken {
        std::string access_token_;
        std::string refresh_token_;
        std::string id_token_;
        std::string token_type_;
        unsigned int expires_in_;
        unsigned int idle_timeout_;
        AclTemplate acl_template_;

        Poco::JSON::Object to_JSON();
    };

    class Service : public SubSystemServer {
    public:
        Service() noexcept;

        int start();

        void stop();

        Logger &logger() { return SubSystemServer::logger(); };

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

    private:
        static Service *instance_;
        std::mutex mutex_;
    };

}; // end of namespace

#endif //UCENTRAL_UAUTHSERVICE_H
