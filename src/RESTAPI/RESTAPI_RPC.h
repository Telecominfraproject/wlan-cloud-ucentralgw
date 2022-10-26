//
// Created by stephane bourque on 2021-06-28.
//

#pragma once

#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NetException.h"
#include "Poco/Logger.h"
#include "Poco/File.h"
#include "Poco/JSON/Object.h"

#include "RESTObjects/RESTAPI_GWobjects.h"
#include "StorageService.h"
#include "framework/RESTAPI_Handler.h"

namespace OpenWifi::RESTAPI_RPC {

	void WaitForCommand(
					uint64_t RPCID,
					bool RetryLater,
					GWObjects::CommandDetails &Cmd,
					Poco::JSON::Object  & Params,
					Poco::Net::HTTPServerRequest &Request,
					Poco::Net::HTTPServerResponse &Response,
					std::chrono::milliseconds WaitTimeInMs,
					Poco::JSON::Object * ObjectToReturn,
					RESTAPIHandler * Handler,
					Poco::Logger &Logger);

	void SetCommandStatus(	GWObjects::CommandDetails &Cmd,
		Poco::Net::HTTPServerRequest &Request,
		Poco::Net::HTTPServerResponse &Response, RESTAPIHandler * handler,
		OpenWifi::Storage::CommandExecutionType Status,
		Poco::Logger &Logger);

}
