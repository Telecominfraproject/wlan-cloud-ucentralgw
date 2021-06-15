//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/JSON/Parser.h"

#include "RESTAPI_default_configuration.h"

#include "RESTAPI_objects.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"

namespace uCentral {
void RESTAPI_default_configuration::handleRequest(Poco::Net::HTTPServerRequest &Request,
												  Poco::Net::HTTPServerResponse &Response) {
	if (!ContinueProcessing(Request, Response))
		return;

	if (!IsAuthorized(Request, Response))
		return;

	std::string Name = GetBinding(uCentral::RESTAPI::Protocol::NAME, "");
	ParseParameters(Request);

	if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
		uCentral::Objects::DefaultConfiguration DefConfig;
		if (Storage()->GetDefaultConfiguration(Name, DefConfig)) {
			Poco::JSON::Object Obj;
			DefConfig.to_json(Obj);
			ReturnObject(Request, Obj, Response);
		} else {
			NotFound(Request, Response);
		}
	} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
		if (Storage()->DeleteDefaultConfiguration(Name)) {
			OK(Request, Response);
		} else {
			NotFound(Request, Response);
		}
	} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
		Poco::JSON::Parser IncomingParser;
		Poco::JSON::Object::Ptr Obj =
			IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		uCentral::Objects::DefaultConfiguration DefConfig;

		if (!DefConfig.from_json(Obj)) {
			BadRequest(Request, Response);
			return;
		}

		if (Storage()->CreateDefaultConfiguration(Name, DefConfig)) {
			OK(Request, Response);
		} else {
			BadRequest(Request, Response);
		}
	} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
		Poco::JSON::Parser IncomingParser;
		Poco::JSON::Object::Ptr Obj =
			IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		uCentral::Objects::DefaultConfiguration DefConfig;
		if (!DefConfig.from_json(Obj)) {
			BadRequest(Request, Response);
			return;
		}

		if (Storage()->UpdateDefaultConfiguration(Name, DefConfig)) {
			OK(Request, Response);
		} else {
			BadRequest(Request, Response);
		}
	} else {
		BadRequest(Request, Response);
	}
}
}