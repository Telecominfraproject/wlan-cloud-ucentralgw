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
        const std::string &access_token() const { return access_token_; };

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
