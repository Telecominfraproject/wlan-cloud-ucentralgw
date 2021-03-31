//
// Created by stephane bourque on 2021-03-04.
//

#ifndef UCENTRAL_RESTAPI_HANDLER_H
#define UCENTRAL_RESTAPI_HANDLER_H

#include "Poco/URI.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NetException.h"
#include "Poco/Logger.h"
#include "Poco/JSON/Object.h"

class RESTAPIHandler: public Poco::Net::HTTPRequestHandler
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
    void ParseParameters(Poco::Net::HTTPServerRequest& request);
    static std::string to_RFC3339(uint64_t t);
    static uint64_t from_RFC3339(const std::string &t);

    void ProcessOptions( Poco::Net::HTTPServerResponse & response );
    void PrepareResponse( Poco::Net::HTTPServerResponse & response, Poco::Net::HTTPResponse::HTTPStatus Status=Poco::Net::HTTPResponse::HTTP_OK);
    bool ContinueProcessing( Poco::Net::HTTPServerRequest & Request , Poco::Net::HTTPServerResponse & Response );
    bool IsAuthorized(Poco::Net::HTTPServerRequest & Request, Poco::Net::HTTPServerResponse & Response );
    bool IsAuthorized(Poco::Net::HTTPServerRequest & Request, Poco::Net::HTTPServerResponse & Response , std::string & UserName );
    uint64_t GetParameter(const std::string &Name,uint64_t Default);
    std::string GetParameter(const std::string &Name,const std::string & Default);

    void BadRequest(Poco::Net::HTTPServerResponse & Response);
    void UnAuthorized(Poco::Net::HTTPServerResponse & Response );
    void ReturnObject(Poco::JSON::Object & Object, Poco::Net::HTTPServerResponse & Response);
    void NotFound(Poco::Net::HTTPServerResponse &Response);
    void OK(Poco::Net::HTTPServerResponse &Response);

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
