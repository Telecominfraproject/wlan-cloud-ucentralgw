//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_command.h"

#include "RESTAPI_protocol.h"
#include "StorageService.h"

namespace uCentral {
void RESTAPI_command::handleRequest(Poco::Net::HTTPServerRequest &Request,
									Poco::Net::HTTPServerResponse &Response) {
	if (!ContinueProcessing(Request, Response))
		return;

	if (!IsAuthorized(Request, Response))
		return;

	try {
		ParseParameters(Request);

		if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
			auto CommandUUID = GetBinding(uCentral::RESTAPI::Protocol::COMMANDUUID, "");
			GWObjects::CommandDetails Command;
			if (Storage()->GetCommand(CommandUUID, Command)) {
				Poco::JSON::Object RetObj;
				Command.to_json(RetObj);
				ReturnObject(Request, RetObj, Response);
			} else
				NotFound(Request, Response);
		} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
			auto CommandUUID = GetBinding(uCentral::RESTAPI::Protocol::COMMANDUUID, "");
			if (Storage()->DeleteCommand(CommandUUID)) {
				OK(Request, Response);
			} else {
				NotFound(Request, Response);
			}
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}
}