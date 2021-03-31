//
// Created by stephane bourque on 2021-03-21.
//

#ifndef UCENTRAL_RESTAPI_COMMAND_H
#define UCENTRAL_RESTAPI_COMMAND_H

#include "RESTAPI_handler.h"

class RESTAPI_command: public RESTAPIHandler
{
public:
    RESTAPI_command(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
            : RESTAPIHandler(bindings,L,
                             std::vector<std::string>
                                     {  Poco::Net::HTTPRequest::HTTP_GET,
                                        Poco::Net::HTTPRequest::HTTP_DELETE,
                                        Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
};


#endif //UCENTRAL_RESTAPI_COMMAND_H
