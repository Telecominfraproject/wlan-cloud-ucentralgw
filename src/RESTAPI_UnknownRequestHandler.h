//
// Created by stephane bourque on 2021-03-03.
//

#ifndef UCENTRAL_RESTAPI_UNKNOWNREQUESTHANDLER_H
#define UCENTRAL_RESTAPI_UNKNOWNREQUESTHANDLER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/Context.h"
#include "Poco/JSON/Parser.h"
#include "Poco/DynamicAny.h"

using Poco::Net::ServerSocket;
using Poco::Net::SecureServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::Context;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::JSON::Parser;

#include "RESTAPIHandler.h"

class RESTAPI_UnknownRequestHandler: public RESTAPIHandler
{
public:
    RESTAPI_UnknownRequestHandler(const RESTAPIHandler::BindingMap & bindings,Poco::Logger & L)
    : RESTAPIHandler(bindings,L,
            std::vector<std::string>{}) {}
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;
};



#endif //UCENTRAL_RESTAPI_UNKNOWNREQUESTHANDLER_H
