//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"

#include "RESTAPI_oauth2Handler.h"
#include "uAuthService.h"
#include "RESTAPI_protocol.h"

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

            auto userId = ds[uCentral::RESTAPI::Protocol::USERID].toString();
            auto password = ds[uCentral::RESTAPI::Protocol::PASSWORD].toString();

			Poco::toLowerInPlace(userId);
            uCentral::Objects::WebToken Token;

            if (uCentral::Auth::Authorize(userId, password, Token)) {
                Poco::JSON::Object ReturnObj;
				Token.to_json(ReturnObj);
                ReturnObject(Request, ReturnObj, Response);
            } else {
                UnAuthorized(Request, Response);
            }

        } else if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            if (!IsAuthorized(Request, Response))
                return;
            auto Token = GetBinding(uCentral::RESTAPI::Protocol::TOKEN, "...");
            if (Token == SessionToken_)
                uCentral::Auth::Logout(Token);
            OK(Request, Response);
        }
        return;
    }
    catch (const Poco::Exception &E) {
        Logger_.warning(Poco::format( "%s: Failed with: %s" , std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}