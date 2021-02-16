//
// Created by stephane bourque on 2021-02-12.
//

//  This implements the various calls to the tipController

#ifndef UCENTRAL_TIPAPI_H
#define UCENTRAL_TIPAPI_H

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

#include "common.h"

class MyPassPhraseHandler : public Poco::Net::KeyFileHandler {
public:
    MyPassPhraseHandler()
    :Poco::Net::KeyFileHandler(false){

    };

    ~MyPassPhraseHandler() {};

    void onPrivateKeyRequested( const void * pSender, std::string & privateKey ) {
        std::cout << "Private key: " << privateKey << std::endl;
        privateKey = "mypassword";
    }
private:

};

class MyInvalidCertificateHandler : public Poco::Net::InvalidCertificateHandler
{
public:
    MyInvalidCertificateHandler() : Poco::Net::InvalidCertificateHandler(false) {};
    ~MyInvalidCertificateHandler() {};
    void onInvalidCertificate(const void * pSender,Poco::Net::VerificationErrorArgs & errorCert)
    {
        std::cout << "CERT: ignoring cert." << std::endl;
        errorCert.setIgnoreError(true);
    }
private:
};

class TIPAPI {
public:
    TIPAPI():initialized_(false) {
    }

    ~TIPAPI() {
        if(initialized_)
            Poco::Net::SSLManager::instance().shutdown();
    }

    bool login();
    void init();

private:
    bool initialized_;
    std::string portal_host_;       //  TIP portal server name: default to wlan-ui.wlan.local
    std::string username_;          //  TIP user name: default to "support@example.com"
    std::string password_;          //  TIP user password: default to "support"
    unsigned int tip_port_;         //  TIP portal management port: default to 9051
    std::string access_token_;      //  Token obtained during login
    std::string key_filename_;
    std::string cert_filename_;
    std::string cacert_filename_;
    std::string key_filename_password_;
    bool use_ssl_;
    Poco::Net::Context * ctx_ptr_;
};

#endif //UCENTRAL_TIPAPI_H
