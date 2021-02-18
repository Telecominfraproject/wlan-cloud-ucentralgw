//
// Created by stephane bourque on 2021-02-12.
//

#include <string>

#include "Api.h"
#include "../app.h"

#include "Poco//Net/DNS.h"

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

    void API::Init() {
        if (!initialized_) {
            initialized_ = true;
            username_ = App::instance().config().getString("tip.api.login.username");
            password_ = App::instance().config().getString("tip.api.login.password");
            api_host_ = App::instance().config().getString("tip.api.host");
            api_port_ = App::instance().config().getInt("tip.api.port");
            ssc_host_ = App::instance().config().getString("tip.ssc.host");
            ssc_port_ = App::instance().config().getInt("tip.ssc.port");
        }
    }

    void API::Logout() {
        initialized_ = false;
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

    bool API::CreateRoutingGateway(const TIP::EquipmentGateway::EquipmentGatewayRecord & R) {
        Init();
        Poco::Net::HTTPSClientSession session(ssc_host_, ssc_port_);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, std::string("/api/routing/gateway"),
                                       Poco::Net::HTTPMessage::HTTP_1_1);

        Poco::JSON::Object obj;
        R.to_JSON(obj);
        std::stringstream ss;
        obj.stringify(ss);
        request.setContentType("application/json");
        request.setContentLength(ss.str().size());
        std::ostream &o = session.sendRequest(request);
        obj.stringify(o);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        TIP::EquipmentGateway::EquipmentGatewayRecord NR;
        NR.from_stream(s);

        return (response.getStatus() == 200);
    }

    TIP::EquipmentGateway::EquipmentGatewayRecord API::GetRoutingGateway(uint64_t Id) {
        Init();
        TIP::EquipmentGateway::EquipmentGatewayRecord  R;

        Poco::Net::HTTPSClientSession session(ssc_host_, ssc_port_);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, std::string("/api/routing/gateway?gatewayId=") + std::to_string(Id),
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setContentType("application/json");
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        R.from_stream(s);

        return R;
    }

    std::vector<TIP::EquipmentGateway::EquipmentGatewayRecord>  API::GetRoutingGatewaysByType(const std::string & Type) {
        Init();
        std::vector<TIP::EquipmentGateway::EquipmentGatewayRecord>  R;

        Poco::Net::HTTPSClientSession session(ssc_host_, ssc_port_);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, std::string("/api/routing/gateway/byType?gatewayType=") + Type,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setContentType("application/json");
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        R = GetJSONArray<TIP::EquipmentGateway::EquipmentGatewayRecord>(s);

        return R;
    }

    std::vector<TIP::EquipmentGateway::EquipmentGatewayRecord>  API::GetRoutingGatewaysByHost(const std::string & host) {
        Init();
        std::vector<TIP::EquipmentGateway::EquipmentGatewayRecord>  R;

        Poco::Net::HTTPSClientSession session(ssc_host_, ssc_port_);
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, std::string("/api/routing/gateway/byHostname?hostname=") + host,
                                       Poco::Net::HTTPMessage::HTTP_1_1);
        request.setContentType("application/json");
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        R = GetJSONArray<TIP::EquipmentGateway::EquipmentGatewayRecord>(s);

        return R;
    }


}

