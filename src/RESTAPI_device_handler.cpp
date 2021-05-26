//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_device_handler.h"
#include "Poco/JSON/Parser.h"
#include "uStorageService.h"
#include "RESTAPI_protocol.h"

void RESTAPI_device_handler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    ParseParameters(Request);
    if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
        std::string     SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER,"");
        uCentral::Objects::Device  Device;

        if(uCentral::Storage::GetDevice(SerialNumber,Device))
        {
            Poco::JSON::Object  Obj;
			Device.to_json(Obj);
            ReturnObject(Request, Obj,Response);
        }
        else
        {
            NotFound(Request, Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
        std::string SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        if (uCentral::Storage::DeleteDevice(SerialNumber)) {
            OK(Request, Response);
        } else {
            NotFound(Request, Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
        std::string SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentral::Objects::Device  Device;

        if(!Device.from_json(Obj))
        {
            BadRequest(Request, Response);
            return;
        }

		Device.UUID = time(nullptr);
        if (uCentral::Storage::CreateDevice(Device)) {
			Poco::JSON::Object  DevObj;
			Device.to_json(DevObj);
			ReturnObject(Request, DevObj,Response);
        } else {
            BadRequest(Request, Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
        std::string SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentral::Objects::Device  Device;

        if(!Device.from_json(Obj))
        {
            BadRequest(Request, Response);
            return;
        }

        if (uCentral::Storage::UpdateDevice(Device)) {
            OK(Request, Response);
        } else {
            BadRequest(Request, Response);
        }
    } else {
        BadRequest(Request, Response);
    }
}