//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_commands.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"
#include "Utils.h"

namespace uCentral {
void RESTAPI_commands::handleRequest(Poco::Net::HTTPServerRequest &Request,
									 Poco::Net::HTTPServerResponse &Response) {
	if (!ContinueProcessing(Request, Response))
		return;

	if (!IsAuthorized(Request, Response))
		return;

	try {
		ParseParameters(Request);
		if(!InitQueryBlock()) {
			BadRequest(Request, Response, "Illegal parameter value.");
			return;
		}

		auto SerialNumber = GetParameter(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
			std::vector<GWObjects::CommandDetails> Commands;
			if (QB_.Newest) {
				Storage()->GetNewestCommands(SerialNumber, QB_.Limit, Commands);
			} else {
				Storage()->GetCommands(SerialNumber, QB_.StartDate, QB_.EndDate, QB_.Offset,
											   QB_.Limit, Commands);
			}
			Poco::JSON::Array ArrayObj;
			for (const auto &i : Commands) {
				Poco::JSON::Object Obj;
				i.to_json(Obj);
				ArrayObj.add(Obj);
			}
			Poco::JSON::Object RetObj;
			RetObj.set(uCentral::RESTAPI::Protocol::COMMANDS, ArrayObj);
			ReturnObject(Request, RetObj, Response);
			return;

		} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
			if (Storage()->DeleteCommands(SerialNumber, QB_.StartDate, QB_.EndDate))
				OK(Request, Response);
			else
				BadRequest(Request, Response);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}
}