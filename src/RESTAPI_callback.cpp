//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_callback.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"

#include "RESTAPI_protocol.h"
#include "StorageService.h"

namespace uCentral {
void RESTAPI_callback::handleRequest(Poco::Net::HTTPServerRequest &Request,
									 Poco::Net::HTTPServerResponse &Response) {

	if (!ContinueProcessing(Request, Response))
		return;

	if (!ValidateAPIKey(Request, Response))
		return;

	ParseParameters(Request);

	try {
		if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
			DoPost(Request, Response);
		return;
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_callback::DoPost(Poco::Net::HTTPServerRequest &Request,
							  Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		auto Topic = GetParameter(uCentral::RESTAPI::Protocol::TOPIC, "");
		if (Topic == "ucentralfws") {
			if (ds.contains(uCentral::RESTAPI::Protocol::FIRMWARES) &&
				ds[uCentral::RESTAPI::Protocol::FIRMWARES].isArray()) {
				std::cout << "Proper manifest received..." << std::endl;
				Logger_.information("New manifest...");
				OK(Request, Response);
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
	BadRequest(Request, Response);
}
}