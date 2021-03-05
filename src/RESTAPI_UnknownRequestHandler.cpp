//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_UnknownRequestHandler.h"

void RESTAPI_UnknownRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    std::cout << "Unknown Handler:" << std::endl;
};