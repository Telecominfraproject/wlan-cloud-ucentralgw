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

#include "RESTAPI_GWobjects.h"
#include "RESTAPI_handler.h"

namespace OpenWifi::RESTAPI_RPC {

	void WaitForCommand( 	GWObjects::CommandDetails &Cmd,
							Poco::JSON::Object  & Params,
							Poco::Net::HTTPServerRequest &Request,
							Poco::Net::HTTPServerResponse &Response,
							int64_t WaitTimeInMs,
							Poco::JSON::Object * ObjectToReturn,
							RESTAPIHandler * Handler,
							Poco::Logger &Logger);

	void SetCommandAsPending(	GWObjects::CommandDetails &Cmd,
								Poco::Net::HTTPServerRequest &Request,
								Poco::Net::HTTPServerResponse &Response, RESTAPIHandler * handler,
								Poco::Logger &Logger);

	void SetCommandAsTimedOut(	GWObjects::CommandDetails &Cmd,
								 Poco::Net::HTTPServerRequest &Request,
								 Poco::Net::HTTPServerResponse &Response, RESTAPIHandler * handler,
								 Poco::Logger &Logger);


}
#endif // UCENTRALGW_RESTAPI_RPC_H
