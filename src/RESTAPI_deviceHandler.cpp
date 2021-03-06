//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_deviceHandler.h"
#include "uStorageService.h"
#include "uAuthService.h"

void RESTAPI_deviceHandler::handleRequest(HTTPServerRequest& Request, HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
        ParseParameters(Request);

        std::string     SerialNumber = GetBinding("serialNumber","0xdeadbeef");
        uCentralDevice  Device;

        if(uCentral::Storage::Service::instance()->GetDevice(SerialNumber,Device))
        {
            Poco::JSON::Object  Obj = Device.to_json();
            PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_OK);
            std::ostream & Answer = Response.send();
            Poco::JSON::Stringifier::stringify(Obj, Answer);
        }
        else
        {
            PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
            Response.send();
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
        ParseParameters(Request);

        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");

        if (uCentral::Storage::Service::instance()->DeleteDevice(SerialNumber)) {
            PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_OK);
            Response.send();
        } else {
            PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
            Response.send();
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
        ParseParameters(Request);

        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentralDevice  Device;
        Device.from_JSON(Obj);
        if(Device.UUID==0)
            Device.UUID = time(nullptr);

        if (uCentral::Storage::Service::instance()->CreateDevice(Device)) {
            PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_OK);
            Response.send();
        } else {
            PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
            Response.send();
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
        ParseParameters(Request);

        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");
    } else {
        BadRequest(Response);
    }

}