//
// Created by stephane bourque on 2021-03-03.
//

#ifndef UCENTRAL_RESTAPI_OAUTH2HANDLER_H
#define UCENTRAL_RESTAPI_OAUTH2HANDLER_H

#include "RESTAPI_handler.h"

class RESTAPI_oauth2Handler: public RESTAPIHandler
{
public:
    RESTAPI_oauth2Handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
        : RESTAPIHandler(bindings,L,
                         std::vector<std::string>
                                 {  Poco::Net::HTTPRequest::HTTP_POST,
                                    Poco::Net::HTTPRequest::HTTP_DELETE,
                                    Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;
};


#endif //UCENTRAL_RESTAPI_OAUTH2HANDLER_H
