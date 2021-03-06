//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_UAUTHSERVICE_H
#define UCENTRAL_UAUTHSERVICE_H

#include "SubSystemServer.h"

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

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

        int Start() override;
        void Stop() override;

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

        bool IsAuthorized(Poco::Net::HTTPServerRequest & Request);
        void CreateToken(const std::string & UserName, WebToken & ResultToken);
        bool Authorize( const std::string & UserName, const std::string & Password, WebToken & ResultToken );
        static std::string GenerateToken();

    private:
        static Service *instance_;
        std::mutex mutex_;
        std::map<std::string,WebToken>   Tokens_;
        bool    Secure_;
        std::string     DefaultUserName_,
                        DefaultPassword_;
        std::string     Mechanism_;
    };

}; // end of namespace

#endif //UCENTRAL_UAUTHSERVICE_H
