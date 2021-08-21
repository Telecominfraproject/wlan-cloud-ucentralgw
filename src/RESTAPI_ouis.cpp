//
// Created by stephane bourque on 2021-06-17.
//

#include "RESTAPI_ouis.h"
#include "RESTAPI_protocol.h"
#include "Utils.h"
#include "OUIServer.h"

namespace OpenWifi {

	void RESTAPI_ouis::handleRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
		if (!ContinueProcessing(Request, Response))
			return;

		if (!IsAuthorized(Request, Response))
			return;

		try {
			ParseParameters(Request);
			if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
				Poco::JSON::Array Objects;
				auto Select = GetParameter("macList","");
				std::vector<std::string> Macs = Utils::Split(Select);
				for (auto &i : Macs) {
					Poco::JSON::Object O;
					auto Manufacturer = OUIServer()->GetManufacturer(i);
					O.set("tag", i);
					O.set("value", Manufacturer);
					Objects.add(O);
				}
				Poco::JSON::Object	RetObj;
				RetObj.set("tagList",Objects);
				ReturnObject(Request, RetObj, Response);
				return;
			}
		} catch (const Poco::Exception &E) {
			Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
		}
		BadRequest(Request, Response);
	}
}