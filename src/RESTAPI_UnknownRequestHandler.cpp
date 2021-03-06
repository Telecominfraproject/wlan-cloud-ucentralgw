//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_UnknownRequestHandler.h"

void RESTAPI_UnknownRequestHandler::handleRequest(HTTPServerRequest& Request, HTTPServerResponse& Response)
{
    if(!IsAuthorized(Request,Response))
        return;

    BadRequest(Response);
};