//
// Created by stephane bourque on 2021-02-12.
//

//  This implements the various calls to the tipController

#ifndef UCENTRAL_TIPAPI_H
#define UCENTRAL_TIPAPI_H

class TIP {
public:
    TIP() {
    }

    ~TIP() {
    }

    bool login();

private:
    std::string portal_host_;       //  TIP portal server name: default to wlan-ui.wlan.local
    std::string username_;          //  TIP user name: default to "support@example.com"
    std::string password_;          //  TIP user password: default to "support"
    unsigned int tip_port_;         //  TIP portal management port: default to 9051
    std::string access_token_;      //  Token obtained during login
    std::string key_filename_;
    std::string cert_filename_;
    std::string key_filename_password_;
    bool use_ssl_;
};

#endif //UCENTRAL_TIPAPI_H
