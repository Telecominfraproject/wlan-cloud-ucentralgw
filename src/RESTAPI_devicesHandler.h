//
// Created by stephane bourque on 2021-03-03.
//

#ifndef UCENTRAL_RESTAPI_DEVICESHANDLER_H
#define UCENTRAL_RESTAPI_DEVICESHANDLER_H

#include "RESTAPI_Handler.h"


class RESTAPI_devicesHandler: public RESTAPIHandler
{
public:
    RESTAPI_devicesHandler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
        : RESTAPIHandler(bindings,
                         L,
                         std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
                                                  Poco::Net::HTTPRequest::HTTP_OPTIONS}) {};
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;
};

#endif //UCENTRAL_RESTAPI_DEVICESHANDLER_H
