//
// Created by stephane bourque on 2021-05-10.
//

#ifndef UCENTRALGW_RESTAPI_CALLBACK_H
#define UCENTRALGW_RESTAPI_CALLBACK_H

#include "RESTAPI_handler.h"

class RESTAPI_callback: public RESTAPIHandler
{
  public:
	RESTAPI_callback(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
		: RESTAPIHandler(bindings,L,
						 std::vector<std::string>
							 {  Poco::Net::HTTPRequest::HTTP_POST,
								Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;
	void DoPost(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
};


#endif // UCENTRALGW_RESTAPI_CALLBACK_H
