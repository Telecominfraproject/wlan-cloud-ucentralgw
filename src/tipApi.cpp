//
// Created by stephane bourque on 2021-02-12.
//

#include <string>

#include "tipApi.h"
#include "app.h"

void TIPAPI::init() {
    initialized_ = true ;
    key_filename_ = App::instance().config().getString("tip.key_filename") ;
    cert_filename_ = App::instance().config().getString("tip.cert_filename");
    cacert_filename_ = App::instance().config().getString("tip.cacert_filename");
    key_filename_password_ = App::instance().config().getString("tip.key_password");

    Poco::Net::SSLManager::instance().initializeClient(new MyPassPhraseHandler, 0, 0);
    ctx_ptr_ = new Poco::Net::Context(Poco::Net::Context::TLS_CLIENT_USE,
                                      key_filename_,
                                      cert_filename_,
                                      cacert_filename_,
                                      Poco::Net::Context::VERIFY_NONE);
    Poco::Net::SSLManager::instance().initializeClient(new MyPassPhraseHandler, new MyInvalidCertificateHandler, ctx_ptr_);
}

bool TIPAPI::login() {

    if(!initialized_)
        init();

    Poco::Net::HTTPSClientSession session( App::instance().config().getString("tipapi.host"),
                                           App::instance().config().getInt("tipapi.port"), ctx_ptr_);

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/management/v1/oauth2/token", Poco::Net::HTTPMessage::HTTP_1_1);
    request.setContentType("application/json");

    Poco::JSON::Object obj;
    obj.set("userId", App::instance().config().getString("tiplogin.username"));
    obj.set("password", App::instance().config().getString("tiplogin.password"));
    std::stringstream ss;
    obj.stringify(ss);
    request.setContentLength(ss.str().size());
    std::ostream& o = session.sendRequest(request);
    obj.stringify(o);

    Poco::Net::HTTPResponse response;

    std::istream& s = session.receiveResponse(response);

    Poco::JSON::Parser parser;
    Poco::JSON::Object::Ptr ret = parser.parse(s).extract<Poco::JSON::Object::Ptr>();

    access_token_ = ret->get("access_token").toString();

    std::cout << "ACCESS_TOKEN: " << access_token_ << std::endl;

    return true;
}
