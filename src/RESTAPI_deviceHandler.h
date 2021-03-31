//
// Created by stephane bourque on 2021-03-03.
//

#ifndef UCENTRAL_RESTAPI_DEVICEHANDLER_H
#define UCENTRAL_RESTAPI_DEVICEHANDLER_H

#include "RESTAPI_handler.h"

#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

class RESTAPI_deviceHandler: public RESTAPIHandler
{
public:
    RESTAPI_deviceHandler(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
        : RESTAPIHandler(bindings,L,
                         std::vector<std::string>
                                 {  Poco::Net::HTTPRequest::HTTP_GET,
                                    Poco::Net::HTTPRequest::HTTP_POST,
                                    Poco::Net::HTTPRequest::HTTP_PUT,
                                    Poco::Net::HTTPRequest::HTTP_DELETE,
                                    Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
};



#endif //UCENTRAL_RESTAPI_DEVICEHANDLER_H
