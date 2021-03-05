//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_oauth2Handler.h"

void RESTAPI_oauth2Handler::handleRequest(HTTPServerRequest & Request, HTTPServerResponse & Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_POST) {

    } else if(Request.getMethod()==Poco::Net::HTTPServerRequest::HTTP_DELETE) {

    }

}