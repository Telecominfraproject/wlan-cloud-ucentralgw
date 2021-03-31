//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_unknownRequestHandler.h"

void RESTAPI_UnknownRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!IsAuthorized(Request,Response))
        return;

    BadRequest(Response);
};