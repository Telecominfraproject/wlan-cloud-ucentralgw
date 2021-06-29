//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H
#define UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H

#include "RESTAPI_handler.h"

namespace uCentral {
class RESTAPI_device_commandHandler : public RESTAPIHandler {
  public:
	RESTAPI_device_commandHandler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
		: RESTAPIHandler(bindings, L,
						 std::vector<std::string>{
							 Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
							 Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
							 Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
	void handleRequest(Poco::Net::HTTPServerRequest &Request,
					   Poco::Net::HTTPServerResponse &Response) override;

	void GetCapabilities(Poco::Net::HTTPServerRequest &Request,
						 Poco::Net::HTTPServerResponse &Response);
	void DeleteCapabilities(Poco::Net::HTTPServerRequest &Request,
							Poco::Net::HTTPServerResponse &Response);
	void GetLogs(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void DeleteLogs(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void GetStatistics(Poco::Net::HTTPServerRequest &Request,
					   Poco::Net::HTTPServerResponse &Response);
	void DeleteStatistics(Poco::Net::HTTPServerRequest &Request,
						  Poco::Net::HTTPServerResponse &Response);
	void GetStatus(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void ExecuteCommand(Poco::Net::HTTPServerRequest &Request,
						Poco::Net::HTTPServerResponse &Response);
	void Configure(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void GetChecks(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void DeleteChecks(Poco::Net::HTTPServerRequest &Request,
					  Poco::Net::HTTPServerResponse &Response);
	void Upgrade(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void Reboot(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void Factory(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void LEDs(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void Trace(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void MakeRequest(Poco::Net::HTTPServerRequest &Request,
					 Poco::Net::HTTPServerResponse &Response);
	void WifiScan(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void EventQueue(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
	void Rtty(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);

	static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/device/{serialNumber}/{command}"}; };

  private:
	std::string SerialNumber_;
};
}
#endif //UCENTRAL_RESTAPI_DEVICECOMMANDHANDLER_H
