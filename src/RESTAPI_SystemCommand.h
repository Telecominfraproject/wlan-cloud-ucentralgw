//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_RESTAPI_SYSTEMCOMMAND_H
#define UCENTRALGW_RESTAPI_SYSTEMCOMMAND_H

#include "RESTAPI_handler.h"

class RESTAPI_SystemCommand: public RESTAPIHandler
{
  public:
	RESTAPI_SystemCommand(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
		: RESTAPIHandler(bindings,L,
						 std::vector<std::string>
						 {  Poco::Net::HTTPRequest::HTTP_POST,
							Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
};

#endif // UCENTRALGW_RESTAPI_SYSTEMCOMMAND_H
