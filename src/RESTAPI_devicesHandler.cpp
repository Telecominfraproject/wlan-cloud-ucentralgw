//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_devicesHandler.h"
#include "uStorageService.h"
#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"

using Poco::Array;

void RESTAPI_devicesHandler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
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

            Logger_.information(Poco::format("DEVICES: from %Lu, limit of %Lu, filter='%s'.", (uint64_t )Offset, (uint64_t )Limit, Filter));
            RESTAPIHandler::PrintBindings();

            std::vector<uCentralDevice> Devices;

            uCentral::Storage::GetDevices(Offset, Limit, Devices);

            Poco::JSON::Array Objects;
            for (const auto & i:Devices)
                Objects.add(i.to_json());

            Poco::JSON::Object RetObj;
            RetObj.set("devices", Objects);
            ReturnObject(RetObj, Response);
        } else
            BadRequest(Response);

        return;
    }
    catch (const Poco::Exception & E)
    {
        Logger_.warning(Poco::format("%s: Failed with: %s",std::string(__func__), E.displayText()));
    }
}
