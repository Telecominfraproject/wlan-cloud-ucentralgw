//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"

#include "RESTAPI_default_configurations.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_default_configurations::handleRequest(Poco::Net::HTTPServerRequest &Request,
													   Poco::Net::HTTPServerResponse &Response) {
		if (!ContinueProcessing(Request, Response))
			return;

		if (!IsAuthorized(Request, Response))
			return;

		try {
			if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
				ParseParameters(Request);
				if(!InitQueryBlock()) {
					BadRequest(Request, Response, "Illegal parameter value.");
					return;
				}

				Logger_.information(
					Poco::format("DEFAULT_CONFIGURATIONS: from %Lu, limit of %Lu, filter=%s.",
								 (int64_t)QB_.Offset, (int64_t)QB_.Limit, QB_.Filter));
				RESTAPIHandler::PrintBindings();

				std::vector<GWObjects::DefaultConfiguration> DefConfigs;

				Storage()->GetDefaultConfigurations(QB_.Offset, QB_.Limit, DefConfigs);

				Poco::JSON::Array Objects;
				for (const auto &i : DefConfigs) {
					Poco::JSON::Object Obj;
					i.to_json(Obj);
					Objects.add(Obj);
				}

				Poco::JSON::Object RetObj;
				RetObj.set(RESTAPI::Protocol::CONFIGURATIONS, Objects);
				ReturnObject(Request, RetObj, Response);
			} else
				BadRequest(Request, Response);
			return;
		} catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
		}
		BadRequest(Request, Response);
	}
}
