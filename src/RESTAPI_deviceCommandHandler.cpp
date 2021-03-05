//
// Created by stephane bourque on 2021-03-04.
//

#include "RESTAPI_deviceCommandHandler.h"

void RESTAPI_deviceCommandHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    std::cout << "Device Command Handler:" << std::endl;
    RESTAPIHandler::print_bindings();
};