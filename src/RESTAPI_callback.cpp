//
// Created by stephane bourque on 2021-05-10.
//

#include "RESTAPI_callback.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"

#include "uStorageService.h"

void RESTAPI_callback::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {

	if(!ContinueProcessing(Request,Response))
		return;

	if(!ValidateAPIKey(Request, Response))
		return;

	ParseParameters(Request);

	try {
		if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_POST)
			DoPost(Request, Response);
		return;
	}
	catch(const Poco::Exception &E)
	{
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Response);
}

void RESTAPI_callback::DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		auto Topic = GetParameter("topic","");
		if(Topic=="ucentralfws") {
			if(ds.contains("firmwares") && ds["firmwares"].isArray()) {
				std::cout << "Proper manifest received..." << std::endl;
				Logger_.information("New manifest...");
				OK(Response);
				return;
			} else {
				std::cout << __LINE__ << std::endl;
				Logger_.information("Bad manifest. JSON does not contain firmwares");
			}
		} else {
			Logger_.information("Missing topic in callback.");
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Response);
}
