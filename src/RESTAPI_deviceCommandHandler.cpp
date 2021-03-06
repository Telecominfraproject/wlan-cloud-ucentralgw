//
// Created by stephane bourque on 2021-03-04.
//

#include "RESTAPI_deviceCommandHandler.h"

void RESTAPI_deviceCommandHandler::handleRequest(HTTPServerRequest& Request, HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    std::string Command = get_binding("command","-");
    if(Command == "-")
    {
        BadRequest(Response);
        return;
    }

    if(Command=="capabilities" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_GET) {

    } else if(Command=="capabilities" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_DELETE) {

    } else if(Command=="logs" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_GET) {

    } else if(Command=="statistics" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_GET) {

    } else if(Command=="statistics" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_DELETE) {

    } else if(Command=="statistics" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_DELETE) {

    } else if(Command=="status" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_GET) {

    } else if(Command=="command" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_POST) {

    } else if(Command=="configure" && Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_POST) {

    } else
    {
        BadRequest(Response);
    }
};