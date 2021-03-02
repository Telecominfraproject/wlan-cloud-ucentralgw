//
// Created by stephane bourque on 2021-02-17.
//

#ifndef UCENTRAL_WEBTOKENRESULT_H
#define UCENTRAL_WEBTOKENRESULT_H

#include "Poco/JSON/Object.h"

namespace TIP::WebToken {
    class AclTemplate {
        friend class WebTokenResult;
    public:
        AclTemplate() :
                Read_(false),
                ReadWrite_(false),
                ReadWriteCreate_(false),
                Delete_(false),
                PortalLogin_(false) {
        }
    private:
        bool Read_;
        bool ReadWrite_;
        bool ReadWriteCreate_;
        bool Delete_;
        bool PortalLogin_;
    };

    class WebTokenResult {

    public:
        bool from_JSON(std::istream &response);
        [[nodiscard]] const std::string &access_token() const { return access_token_; };
        [[nodiscard]] const std::string &refresh_token() const { return refresh_token_; };
        [[nodiscard]] const std::string &id_token() const { return id_token_; };
        [[nodiscard]] const std::string &token_type() const { return token_type_; };
        [[nodiscard]] unsigned int expires_in() const { return expires_in_; };
        [[nodiscard]] unsigned int idle_timeout() const { return idle_timeout_; };

        [[nodiscard]] bool read_access() const { return acl_template_.Read_; };
        [[nodiscard]] bool readWrite_access() const { return acl_template_.ReadWrite_; };
        [[nodiscard]] bool readWriteCreate_access() const { return acl_template_.ReadWriteCreate_; };
        [[nodiscard]] bool delete_access() const { return acl_template_.Delete_; };
        [[nodiscard]] bool portalLogin_access() const { return acl_template_.PortalLogin_; };

    private:
        std::string access_token_;
        std::string refresh_token_;
        std::string id_token_;
        std::string token_type_;
        unsigned int expires_in_;
        unsigned int idle_timeout_;
        AclTemplate acl_template_;
    };
}


#endif //UCENTRAL_WEBTOKENRESULT_H
