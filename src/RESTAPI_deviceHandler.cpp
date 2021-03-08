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

    ParseParameters(Request);
    if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
        std::string     SerialNumber = GetBinding("serialNumber","0xdeadbeef");
        uCentralDevice  Device;

        if(uCentral::Storage::Service::instance()->GetDevice(SerialNumber,Device))
        {
            Poco::JSON::Object  Obj = Device.to_json();
            ReturnObject(Obj,Response);
        }
        else
        {
            NotFound(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");

        if (uCentral::Storage::Service::instance()->DeleteDevice(SerialNumber)) {
            OK(Response);
        } else {
            NotFound(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentralDevice  Device;
        Device.from_JSON(Obj);
        if(Device.UUID==0)
            Device.UUID = time(nullptr);

        if (uCentral::Storage::Service::instance()->CreateDevice(Device)) {
            OK(Response);
        } else {
            BadRequest(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentralDevice  Device;
        Device.from_JSON(Obj);
        if (uCentral::Storage::Service::instance()->UpdateDevice(Device)) {
            OK(Response);
        } else {
            BadRequest(Response);
        }
    } else {
        BadRequest(Response);
    }

}