//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H
#define UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H

#include "RESTAPI_handler.h"

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
    void handleRequest(HTTPServerRequest& Request, HTTPServerResponse& Response) override;

    void GetCapabilities(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void DeleteCapabilities(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void GetLogs(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void DeleteLogs(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void GetStatistics(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void DeleteStatistics(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void GetStatus(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void ExecuteCommand(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void Configure(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void GetChecks(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void DeleteChecks(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void Upgrade(HTTPServerRequest& Request, HTTPServerResponse& Response);
    void Reboot(HTTPServerRequest& Request, HTTPServerResponse& Response);
};

#endif //UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H
