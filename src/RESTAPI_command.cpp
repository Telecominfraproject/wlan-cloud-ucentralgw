//
// Created by stephane bourque on 2021-03-21.
//

#include "RESTAPI_command.h"

#include "uStorageService.h"

void RESTAPI_command::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    try {
        ParseParameters(Request);

        if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            auto CommandUUID = GetBinding("commandUUID", "");

            uCentralCommandDetails Command;

            if(uCentral::Storage::GetCommand(CommandUUID,  Command)) {
                Poco::JSON::Object RetObj = Command.to_json();
                ReturnObject(RetObj, Response);
            } else
                NotFound(Response);
            return;

        } else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
            auto CommandUUID = GetBinding("commandUUID", "");

            if(uCentral::Storage::DeleteCommand(CommandUUID)) {
                OK(Response);
            } else {
                NotFound(Response);
            }
            return;
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}