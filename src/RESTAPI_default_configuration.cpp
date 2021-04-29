//
// Created by stephane bourque on 2021-03-15.
//

#include "RESTAPI_default_configuration.h"
#include "uStorageService.h"

#include "Poco/JSON/Parser.h"

void RESTAPI_default_configuration::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    ParseParameters(Request);
    if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
        std::string                   Name = GetBinding("name","0xdeadbeef");
        uCentralDefaultConfiguration  DefConfig;

        if(uCentral::Storage::GetDefaultConfiguration(Name,DefConfig))
        {
            Poco::JSON::Object  Obj = DefConfig.to_json();
            ReturnObject(Obj,Response);
        }
        else
        {
            NotFound(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
        std::string Name = GetBinding("name", "0xdeadbeef");

        if (uCentral::Storage::DeleteDefaultConfiguration(Name)) {
            OK(Response);
        } else {
            NotFound(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
        std::string Name = GetBinding("name", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentralDefaultConfiguration  DefConfig;

        if(!DefConfig.from_json(Obj))
        {
            BadRequest(Response);
            return;
        }

        if (uCentral::Storage::CreateDefaultConfiguration(Name,DefConfig)) {
            OK(Response);
        } else {
            BadRequest(Response);
        }
    } else if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
        std::string Name = GetBinding("name", "0xdeadbeef");

        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

        uCentralDefaultConfiguration  DefConfig;
        if(!DefConfig.from_json(Obj))
        {
            BadRequest(Response);
            return;
        }

        if (uCentral::Storage::UpdateDefaultConfiguration(Name, DefConfig)) {
            OK(Response);
        } else {
            BadRequest(Response);
        }
    } else {
        BadRequest(Response);
    }
}