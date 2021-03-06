//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_oauth2Handler.h"
#include "uAuthService.h"

void RESTAPI_oauth2Handler::handleRequest(HTTPServerRequest & Request, HTTPServerResponse & Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_POST) {
        // Extract the info for login...
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        auto userId = ds["userId"].toString();
        auto password = ds["password"].toString();

        uCentral::Auth::WebToken    Token;

        if(uCentral::Auth::Service::instance()->Authorize(userId,password,Token))
        {
            PrepareResponse(Response);

            auto ReturnObj = Token.to_JSON();

            std::ostream & Answer = Response.send();
            Poco::JSON::Stringifier::stringify(ReturnObj, Answer);
        }
        else
        {
            UnAuthorized(Response);
        }

    } else if(Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_DELETE) {
        if(!IsAuthorized(Request,Response))
            return;
    }

}