//
// Created by stephane bourque on 2021-04-13.
//

#ifndef UCENTRALGW_RESTAPI_BLACKLIST_H
#define UCENTRALGW_RESTAPI_BLACKLIST_H

#include "RESTAPI_handler.h"

class RESTAPI_BlackList: public RESTAPIHandler
{
  public:
	RESTAPI_BlackList(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
		: RESTAPIHandler(bindings,L,
						 std::vector<std::string>
							 {  Poco::Net::HTTPRequest::HTTP_GET,
								Poco::Net::HTTPRequest::HTTP_POST,
								Poco::Net::HTTPRequest::HTTP_DELETE,
								Poco::Net::HTTPRequest::HTTP_OPTIONS}) {}
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;

	void DoGet(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
	void DoDelete(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
	void DoPost(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
};

#endif // UCENTRALGW_RESTAPI_BLACKLIST_H
