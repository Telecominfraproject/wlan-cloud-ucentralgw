//
// Created by stephane bourque on 2021-03-29.
//

#ifndef UCENTRAL_RESTAPI_FILE_H
#define UCENTRAL_RESTAPI_FILE_H

#include "RESTAPI_handler.h"

class RESTAPI_file: public RESTAPIHandler
{
public:
    RESTAPI_file(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
            : RESTAPIHandler(bindings,L,
                             std::vector<std::string>
                                     {  Poco::Net::HTTPRequest::HTTP_GET,
                                        Poco::Net::HTTPRequest::HTTP_DELETE,
                                        Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
};


#endif //UCENTRAL_RESTAPI_FILE_H
