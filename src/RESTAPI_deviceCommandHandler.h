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
    void handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) override;

    void GetCapabilities(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void DeleteCapabilities(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void GetLogs(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void DeleteLogs(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void GetStatistics(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void DeleteStatistics(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void GetStatus(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void ExecuteCommand(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void Configure(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void GetChecks(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void DeleteChecks(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void Upgrade(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void Reboot(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response);
    void Factory(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
    void LEDs(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
    void Trace(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void MakeRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
};

#endif //UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H
