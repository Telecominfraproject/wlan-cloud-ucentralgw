//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_deviceHandler.h"
#include "uStorageService.h"
#include "Poco/JSON/Parser.h"

void RESTAPI_deviceHandler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    ParseParameters(Request);
    if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
        std::string     SerialNumber = GetBinding("serialNumber","0xdeadbeef");
        uCentral::Objects::Device  Device;

        if(uCentral::Storage::GetDevice(SerialNumber,Device))
        {
            Poco::JSON::Object  Obj;
			Device.to_json(Obj);
            ReturnObject(Obj,Response);
        }
        else
        {
            NotFound(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");

        if (uCentral::Storage::DeleteDevice(SerialNumber)) {
            OK(Response);
        } else {
            NotFound(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentral::Objects::Device  Device;

        if(!Device.from_json(Obj))
        {
            BadRequest(Response);
            return;
        }

		Device.UUID = time(nullptr);
        if (uCentral::Storage::CreateDevice(Device)) {
			Poco::JSON::Object  DevObj;
			Device.to_json(DevObj);
			ReturnObject(DevObj,Response);
        } else {
            BadRequest(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
        std::string SerialNumber = GetBinding("serialNumber", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentral::Objects::Device  Device;

        if(!Device.from_json(Obj))
        {
            BadRequest(Response);
            return;
        }

        if (uCentral::Storage::UpdateDevice(Device)) {
            OK(Response);
        } else {
            BadRequest(Response);
        }
    } else {
        BadRequest(Response);
    }
}