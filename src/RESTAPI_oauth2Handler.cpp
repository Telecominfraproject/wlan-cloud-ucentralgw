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
	std::cout << __LINE__ << std::endl;

    if(!ContinueProcessing(Request,Response))
        return;

	std::cout << __LINE__ << std::endl;

    try {
        if (Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			std::cout << __LINE__ << std::endl;

            // Extract the info for login...
            Poco::JSON::Parser parser;
            Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

            auto userId = GetS(uCentral::RESTAPI::Protocol::USERID, Obj);
            auto password = GetS(uCentral::RESTAPI::Protocol::PASSWORD, Obj);

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
			std::cout << __LINE__ << std::endl;
            if (!IsAuthorized(Request, Response)) {
				std::cout << __LINE__ << std::endl;
				return;
			}
            auto Token = GetBinding(uCentral::RESTAPI::Protocol::TOKEN, "...");
			std::cout << "Token: " << Token << std::endl;
			std::cout << "Session: " << SessionToken_ << std::endl;
            if (Token == SessionToken_) {
				uCentral::Auth::Logout(Token);
				OK(Request, Response);
			} else {
				NotFound(Request,Response);
			}
        } else {
			std::cout << __LINE__ << std::endl;
			BadRequest(Request, Response);
		}
		std::cout << __LINE__ << std::endl;
		return;
    }
    catch (const Poco::Exception &E) {
        Logger_.warning(Poco::format( "%s: Failed with: %s" , std::string(__func__), E.displayText()));
    }
	std::cout << __LINE__ << std::endl;
    BadRequest(Request, Response);
}