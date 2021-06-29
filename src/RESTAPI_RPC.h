//
// Created by stephane bourque on 2021-06-28.
//

#ifndef UCENTRALGW_RESTAPI_RPC_H
#define UCENTRALGW_RESTAPI_RPC_H

#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NetException.h"
#include "Poco/Logger.h"
#include "Poco/File.h"
#include "Poco/JSON/Object.h"

#include "RESTAPI_objects.h"
#include "RESTAPI_handler.h"

namespace uCentral::RESTAPI_RPC {

	bool WaitForRPC(uCentral::Objects::CommandDetails &Cmd,
					   Poco::Net::HTTPServerRequest &Request,
					   Poco::Net::HTTPServerResponse &Response,
					   uint64_t Timeout,
					   bool ReturnObject,
					   RESTAPIHandler * Handler);

	void WaitForCommand( uCentral::Objects::CommandDetails &Cmd,
							Poco::JSON::Object  & Params,
							Poco::Net::HTTPServerRequest &Request,
							Poco::Net::HTTPServerResponse &Response,
							std::chrono::milliseconds D,
							Poco::JSON::Object * ObjectToReturn,
							RESTAPIHandler * Handler);

	void SetCommandAsPending(uCentral::Objects::CommandDetails &Cmd,
								Poco::Net::HTTPServerRequest &Request,
								Poco::Net::HTTPServerResponse &Response, RESTAPIHandler * handler);

}
#endif // UCENTRALGW_RESTAPI_RPC_H
