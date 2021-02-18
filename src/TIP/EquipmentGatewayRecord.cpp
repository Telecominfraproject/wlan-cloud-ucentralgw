//
// Created by stephane bourque on 2021-02-17.
//

#include "EquipmentGatewayRecord.h"
#include "Poco/JSON/Parser.h"

namespace TIP::Routing {

    bool EquipmentGatewayRecord::to_JSON(Poco::JSON::Object &obj) const {
        obj.set("id", id_);
        obj.set("hostname", hostname_);
        obj.set("ipAddr", ipAddr_);
        obj.set("port", port_);
        obj.set("gatewayType", gatewayType_);
        obj.set("createdTimestamp", createdTimestamp_);
        obj.set("lastModifiedTimestamp", lastModifiedTimestamp_);
        return true;
    }

    bool EquipmentGatewayRecord::from_object(Poco::JSON::Object::Ptr Obj) {
        Poco::DynamicStruct ds = *Obj;
        id_ = ds["id"];
        hostname_ = ds["hostname"].toString();
        ipAddr_ = ds["ipAddr"].toString();
        port_ = ds["port"];
        createdTimestamp_ = ds["createdTimestamp"];
        lastModifiedTimestamp_ = ds["lastModifiedTimestamp"];
        return true;
    }

    bool EquipmentGatewayRecord::from_stream(std::istream &response) {

        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(response).extract<Poco::JSON::Object::Ptr>();

        return EquipmentGatewayRecord::from_object(Obj);
    }

    bool CreateRoutingGateway(const TIP::Routing::EquipmentGatewayRecord & R) {
        Poco::Net::HTTPSClientSession session(TIP::API::SSC_Host(),TIP::API::SSC_Port());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST,
                                       std::string("/api/routing/gateway"),
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

        TIP::Routing::EquipmentGatewayRecord NR;
        NR.from_stream(s);

        return (response.getStatus() == 200);
    }

    bool DeleteRoutingGateway(uint64_t Id)
    {
        TIP::Routing::EquipmentGatewayRecord  R;

        Poco::Net::HTTPSClientSession session(TIP::API::SSC_Host(),TIP::API::SSC_Port());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_DELETE,
                                       std::string("/api/routing/gateway?gatewayId=") + std::to_string(Id),
                Poco::Net::HTTPMessage::HTTP_1_1);
        request.setContentType("application/json");
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        std::cout << "Delete result: " << response.getStatus() << std::endl;

        return (response.getStatus() == 200);
    }

    EquipmentGatewayRecord GetRoutingGateway(uint64_t Id) {
        EquipmentGatewayRecord  R;

        Poco::Net::HTTPSClientSession session(TIP::API::SSC_Host(),TIP::API::SSC_Port());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET,
                                       std::string("/api/routing/gateway?gatewayId=") + std::to_string(Id),
                Poco::Net::HTTPMessage::HTTP_1_1);
        request.setContentType("application/json");
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        R.from_stream(s);

        return R;
    }

    std::vector<EquipmentGatewayRecord> GetRoutingGatewaysByType(const std::string & Type) {
        std::vector<TIP::Routing::EquipmentGatewayRecord>  R;

        Poco::Net::HTTPSClientSession session(TIP::API::SSC_Host(),TIP::API::SSC_Port());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET,
                                       std::string("/api/routing/gateway/byType?gatewayType=") + Type,
                Poco::Net::HTTPMessage::HTTP_1_1);
        request.setContentType("application/json");
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        R = TIP::API::GetJSONArray<TIP::Routing::EquipmentGatewayRecord>(s);

        return R;
    }

    std::vector<TIP::Routing::EquipmentGatewayRecord> GetRoutingGatewaysByHost(const std::string & host) {
        std::vector<TIP::Routing::EquipmentGatewayRecord>  R;

        Poco::Net::HTTPSClientSession session(TIP::API::SSC_Host(),TIP::API::SSC_Port());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET,
                                       std::string("/api/routing/gateway/byHostname?hostname=") + host,
                Poco::Net::HTTPMessage::HTTP_1_1);
        request.setContentType("application/json");
        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &s = session.receiveResponse(response);

        R = TIP::API::GetJSONArray<TIP::Routing::EquipmentGatewayRecord>(s);

        return R;
    }

}

