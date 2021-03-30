//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_RESTAPI_HANDLER_H
#define UCENTRAL_RESTAPI_HANDLER_H

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
#include "Poco/Logger.h"
#include "Poco/URI.h"

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
using Poco::URI;


class RESTAPIHandler: public HTTPRequestHandler
{
public:
    typedef std::map<std::string,std::string>   BindingMap;

    RESTAPIHandler(BindingMap map, Poco::Logger & l, std::vector<std::string> Methods)
            : bindings_(std::move(map)),
            Logger_(l),
            methods_(std::move(Methods))
    {

    }

    static bool ParseBindings(const char *path,const char *resource, BindingMap & Keys);
    void PrintBindings();
    void ParseParameters(HTTPServerRequest& request);
    static std::string to_RFC3339(uint64_t t);
    static uint64_t from_RFC3339(const std::string &t);

    void ProcessOptions( HTTPServerResponse & response );
    void PrepareResponse( HTTPServerResponse & response, Poco::Net::HTTPResponse::HTTPStatus Status=Poco::Net::HTTPResponse::HTTP_OK);
    bool ContinueProcessing( HTTPServerRequest & Request , HTTPServerResponse & Response );
    bool IsAuthorized(Poco::Net::HTTPServerRequest & Request, HTTPServerResponse & Response );
    bool IsAuthorized(Poco::Net::HTTPServerRequest & Request, HTTPServerResponse & Response , std::string & UserName );
    uint64_t GetParameter(const std::string &Name,uint64_t Default);
    std::string GetParameter(const std::string &Name,const std::string & Default);

    void BadRequest(HTTPServerResponse & Response);
    void UnAuthorized(HTTPServerResponse & Response );
    void ReturnObject(Poco::JSON::Object & Object, HTTPServerResponse & Response);
    void NotFound(HTTPServerResponse &Response);
    void OK(HTTPServerResponse &Response);

    const std::string & GetBinding(const std::string &Name, const std::string &Default);

protected:
    BindingMap                  bindings_;
    Poco::URI::QueryParameters  parameters_;
    Poco::Logger                & Logger_;
    std::string                 SessionToken_;
    std::string                 UserName_;
    std::vector<std::string>    methods_;
};


#endif //UCENTRAL_RESTAPI_HANDLER_H
