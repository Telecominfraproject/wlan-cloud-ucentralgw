//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H
#define UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H

#include "RESTAPI_Handler.h"

class RESTAPI_deviceCommandHandler : public RESTAPIHandler {
public:
    RESTAPI_deviceCommandHandler(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
            : RESTAPIHandler(bindings,L,
                             std::vector<std::string>
                                     {  Poco::Net::HTTPRequest::HTTP_GET,
                                        Poco::Net::HTTPRequest::HTTP_POST,
                                        Poco::Net::HTTPRequest::HTTP_PUT,
                                        Poco::Net::HTTPRequest::HTTP_DELETE,
                                        Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;
};

#endif //UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H
