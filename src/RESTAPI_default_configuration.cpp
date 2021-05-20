//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"

#include "RESTAPI_default_configuration.h"

#include "uStorageService.h"
#include "RESTAPI_objects.h"

void RESTAPI_default_configuration::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    ParseParameters(Request);
    if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
        std::string                   Name = GetBinding("name","0xdeadbeef");
        uCentral::Objects::DefaultConfiguration  DefConfig;

        if(uCentral::Storage::GetDefaultConfiguration(Name,DefConfig))
        {
            Poco::JSON::Object  Obj;
			DefConfig.to_json(Obj);
            ReturnObject(Request,Obj,Response);
        }
        else
        {
            NotFound(Request, Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
        std::string Name = GetBinding("name", "0xdeadbeef");

        if (uCentral::Storage::DeleteDefaultConfiguration(Name)) {
            OK(Request, Response);
        } else {
            NotFound(Request, Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
        std::string Name = GetBinding("name", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentral::Objects::DefaultConfiguration  DefConfig;

        if(!DefConfig.from_json(Obj))
        {
            BadRequest(Request, Response);
            return;
        }

        if (uCentral::Storage::CreateDefaultConfiguration(Name,DefConfig)) {
            OK(Request, Response);
        } else {
            BadRequest(Request, Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
        std::string Name = GetBinding("name", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentral::Objects::DefaultConfiguration  DefConfig;
        if(!DefConfig.from_json(Obj))
        {
            BadRequest(Request, Response);
            return;
        }

        if (uCentral::Storage::UpdateDefaultConfiguration(Name, DefConfig)) {
            OK(Request, Response);
        } else {
            BadRequest(Request, Response);
        }
    } else {
        BadRequest(Request, Response);
    }
}