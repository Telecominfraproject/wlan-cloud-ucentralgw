//
// Created by stephane bourque on 2021-02-12.
//

#include <string>

#include "Api.h"
#include "../app.h"

#include "Poco//Net/DNS.h"

namespace TIP::API {

    TIP::API::API *TIP::API::API::instance_ = 0;

    void API::Init() {
        username_ = App::instance().config().getString("tip.api.login.username");
        password_ = App::instance().config().getString("tip.api.login.password");
        api_host_ = App::instance().config().getString("tip.api.host");
        api_port_ = App::instance().config().getInt("tip.api.port");
        ssc_host_ = App::instance().config().getString("tip.ssc.host");
        ssc_port_ = App::instance().config().getInt("tip.ssc.port");
    }

    void API::Logout() {
    }

    bool API::Login() {
        Init();
        Poco::Net::HTTPSClientSession session(api_host_, api_port_);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/management/v1/oauth2/token",
                                       Poco::Net::HTTPMessage::HTTP_1_1);

        Poco::JSON::Object obj;
        obj.set("userId", username_);
        obj.set("password", password_);
        std::stringstream ss;
        obj.stringify(ss);
        request.setContentType("application/json");
        request.setContentLength(ss.str().size());
        std::ostream &o = session.sendRequest(request);
        obj.stringify(o);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        token_.from_JSON(s);
        logger_.information("ACCESS_TOKEN: %s", token_.access_token());

        return true;
    }

    bool Login() {
        return TIP::API::API::instance()->Login();
    }

    void Logout() {
        TIP::API::API::instance()->Logout();
    }

    const std::string & SSC_Host() { return TIP::API::API::instance()->ssc_host(); }
    uint64_t SSC_Port() { return TIP::API::API::instance()->ssc_port(); }

}

