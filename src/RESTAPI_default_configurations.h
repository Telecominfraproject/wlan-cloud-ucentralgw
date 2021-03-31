//
// Created by stephane bourque on 2021-03-15.
//

#ifndef UCENTRAL_RESTAPI_DEFAULT_CONFIGURATIONS_H
#define UCENTRAL_RESTAPI_DEFAULT_CONFIGURATIONS_H

#include "RESTAPI_handler.h"

class RESTAPI_default_configurations: public RESTAPIHandler {
public:
    RESTAPI_default_configurations( const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
    : RESTAPIHandler(bindings,
            L,
            std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
                                     Poco::Net::HTTPRequest::HTTP_OPTIONS}) {};
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
};

#endif //UCENTRAL_RESTAPI_DEFAULT_CONFIGURATIONS_H
