//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_oauth2Handler.h"
#include "uAuthService.h"
#include "Poco/JSON/Parser.h"

void RESTAPI_oauth2Handler::handleRequest(Poco::Net::HTTPServerRequest & Request, Poco::Net::HTTPServerResponse & Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    try {
        if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {

            // Extract the info for login...
            Poco::JSON::Parser parser;
            Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
            Poco::DynamicStruct ds = *Obj;

            auto userId = ds["userId"].toString();
            auto password = ds["password"].toString();

            uCentral::Auth::WebToken Token;

            if (uCentral::Auth::Authorize(userId, password, Token)) {
                auto ReturnObj = Token.to_JSON();
                ReturnObject(ReturnObj, Response);
            } else {
                UnAuthorized(Response);
            }

        } else if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            if (!IsAuthorized(Request, Response))
                return;

            auto Token = GetBinding("token", "...");

            if (Token == SessionToken_)
                uCentral::Auth::Logout(Token);
            OK(Response);
        }
        return;
    }
    catch (const Poco::Exception &E) {
        Logger_.warning(Poco::format( "%s: Failed with: %s" , std::string(__func__), E.displayText()));
    }

    BadRequest(Response);
}