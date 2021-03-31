//
// Created by stephane bourque on 2021-03-15.
//

#ifndef UCENTRAL_RESTAPI_DEFAULT_CONFIGURATION_H
#define UCENTRAL_RESTAPI_DEFAULT_CONFIGURATION_H

#include "RESTAPI_handler.h"

class RESTAPI_default_configuration: public RESTAPIHandler
{
public:
    RESTAPI_default_configuration(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
            : RESTAPIHandler(bindings,L,
                             std::vector<std::string>
                                     {  Poco::Net::HTTPRequest::HTTP_GET,
                                        Poco::Net::HTTPRequest::HTTP_POST,
                                        Poco::Net::HTTPRequest::HTTP_PUT,
                                        Poco::Net::HTTPRequest::HTTP_DELETE,
                                        Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
};

#endif //UCENTRAL_RESTAPI_DEFAULT_CONFIGURATION_H
