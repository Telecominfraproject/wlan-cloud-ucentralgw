//
// Created by stephane bourque on 2021-03-15.
//

#include "RESTAPI_default_configurations.h"
#include "uStorageService.h"

#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"

void RESTAPI_default_configurations::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    try {
        if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            ParseParameters(Request);

            auto Offset = GetParameter("offset", 0);
            auto Limit = GetParameter("limit", 100);
            auto Filter = GetParameter("filter", "");

            Logger_.information(Poco::format("DEFAULT_CONFIGURATIONS: from %Lu, limit of %Lu, filter=%s.", (int64_t )Offset, (int64_t ) Limit, Filter));
            RESTAPIHandler::PrintBindings();

            std::vector<uCentralDefaultConfiguration> DefConfigs;

            uCentral::Storage::GetDefaultConfigurations(Offset, Limit, DefConfigs);

            Poco::JSON::Array Objects;
            for (const auto & i:DefConfigs) {
				Poco::JSON::Object	Obj;
				i.to_json(Obj);
				Objects.add(Obj);
			}

            Poco::JSON::Object RetObj;
            RetObj.set("configurations", Objects);
            ReturnObject(RetObj, Response);
        } else
            BadRequest(Response);

        return;
    }
    catch (const Poco::Exception & E)
    {
        Logger_.warning(Poco::format("%s: Failed with: %s",std::string(__func__),E.displayText() ));
    }
}
