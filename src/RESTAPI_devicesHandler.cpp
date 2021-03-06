//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_devicesHandler.h"
#include "uStorageService.h"
#include "uAuthService.h"
#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"


using Poco::Array;

void RESTAPI_devicesHandler::handleRequest(HTTPServerRequest& Request, HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
        ParseParameters(Request);

        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 10000);
        auto Filter = GetParameter("filter", "");

        logger_.information(Poco::format("DEVICES: from %d, limit of %d, filter=%s.", Offset, Limit, Filter));
        RESTAPIHandler::PrintBindings();

        std::vector<uCentralDevice> Devices;

        auto HowMany = uCentral::Storage::Service::instance()->GetDevices(Offset, Limit, Devices);

        std::cout << "Devices Handler:" << std::endl;
        std::cout << "Found " << HowMany << " devices." << std::endl;

        // create the response...
        Poco::JSON::Array Objects;
        for (auto i:Devices)
            Objects.add(i.to_json());

        Poco::JSON::Object ReturnObject;
        ReturnObject.set("devices", Objects);

        Response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);

        PrepareResponse(Response);
        std::ostream &answer = Response.send();
        Poco::JSON::Stringifier::stringify(ReturnObject, answer);
    } else
        BadRequest(Response);
}
