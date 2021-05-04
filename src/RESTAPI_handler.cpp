//
// Created by stephane bourque on 2021-03-04.
//
#include <cctype>
#include <algorithm>
#include <iostream>
#include <iterator>

#include "RESTAPI_handler.h"
#include "uAuthService.h"

#include "Poco/URI.h"
#include "Poco/DateTimeParser.h"

#include "uDeviceRegistry.h"
#include "uStorageService.h"

bool RESTAPIHandler::ParseBindings(const char *p,const char *r, BindingMap & bindings)
{
    char param[256]={0},
         value[256]={0};

    bindings.clear();
    while(*r)
    {
        if(*r == '{') {
            r++;
            auto pi=0;
            while (*r != '}' && pi<sizeof(param))
                param[pi++] = *r++;
            r++;
            param[pi]=0;
            auto vi=0;
            while (*p != '/' && *p && vi<sizeof(value))
                value[vi++] = *p++;
            value[vi]=0;
            bindings[param] = value;
        } else if( *p != *r ) {
            return false;
        } else {
            r++;
            p++;
        }
    }

    return (*p == *r);
}

void RESTAPIHandler::PrintBindings() {
    for(auto &[key,value]:bindings_)
        std::cout << "Key = " << key << "  Value= " << value << std::endl;
}

void RESTAPIHandler::ParseParameters(Poco::Net::HTTPServerRequest& request) {

    Poco::URI uri(request.getURI());
    parameters_ = uri.getQueryParameters();
}

static bool is_number(const std::string &s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

static bool is_bool(const std::string &s) {
	if(s=="true" || s=="false")
		return true;
	return false;
}

uint64_t RESTAPIHandler::GetParameter(const std::string &Name,const uint64_t Default) {

    for(const auto & i:parameters_)
    {
        if(i.first == Name) {
            if(!is_number(i.second))
                return Default;
            return std::stoi(i.second);
        }
    }
    return Default;
}

bool RESTAPIHandler::GetBoolParameter(const std::string &Name,bool Default) {

	for(const auto & i:parameters_)
	{
		if(i.first == Name) {
			if(!is_bool(i.second))
				return Default;
			return i.second=="true";
		}
	}
	return Default;
}

std::string RESTAPIHandler::GetParameter(const std::string &Name,const std::string & Default){
    for(const auto & i:parameters_)
    {
        if(i.first == Name)
            return i.second;
    }
    return Default;
}

const std::string & RESTAPIHandler::GetBinding(const std::string &Name, const std::string &Default) {
    auto E = bindings_.find(Name);

    if(E==bindings_.end())
        return Default;

    return E->second;
}

std::string RESTAPIHandler::to_RFC3339(uint64_t t)
{
    return Poco::DateTimeFormatter::format(Poco::DateTime(Poco::Timestamp::fromEpochTime(t)), Poco::DateTimeFormat::ISO8601_FORMAT);
}

uint64_t RESTAPIHandler::from_RFC3339(const std::string &TimeString)
{
    if(TimeString.empty())
        return 0;

    try {
        int             TZ;
        Poco::DateTime  DT = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT,TimeString,TZ);
        return DT.timestamp().epochTime();
    }
    catch( const Poco::Exception & E )
    {

    }
    return 0;
}

static std::string MakeList(const std::vector<std::string> & L)
{
    std::string Return;

    for(const auto &i : L)
        if( Return.empty() )
            Return = i;
        else
            Return += ',' + i;

    return Return;
}

void RESTAPIHandler::ProcessOptions(Poco::Net::HTTPServerResponse & Response )
{
    Response.setContentType("application/json");
    Response.set("Access-Control-Allow-Origin", "*");
    Response.add("Access-Control-Allow-Headers", "*");
    Response.add("Access-Control-Allow-Method",MakeList(methods_));
    Response.send();
}

void RESTAPIHandler::PrepareResponse(Poco::Net::HTTPServerResponse &Response,Poco::Net::HTTPResponse::HTTPStatus Status)
{
    Response.setStatus(Status);
    Response.setChunkedTransferEncoding(true);
	Response.setKeepAlive(true);
	Response.setContentType("application/json");
    Response.set("Access-Control-Allow-Origin", "*");
    Response.add("Access-Control-Allow-Headers", "*");
    Response.add("Access-Control-Allow-Method",MakeList(methods_));
}

void RESTAPIHandler::BadRequest(Poco::Net::HTTPServerResponse & Response) {
    PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    Response.send();
}

void RESTAPIHandler::UnAuthorized(Poco::Net::HTTPServerResponse & Response )
{
    PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_FORBIDDEN);
    Response.send();
}

void RESTAPIHandler::NotFound(Poco::Net::HTTPServerResponse &Response) {
    PrepareResponse(Response, Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    Response.send();
}

void RESTAPIHandler::OK(Poco::Net::HTTPServerResponse &Response) {
    PrepareResponse(Response);
    Response.send();
}

void RESTAPIHandler::WaitForRPC(uCentralCommandDetails & Cmd, Poco::Net::HTTPServerResponse &Response, uint64_t Timeout) {

	if(uCentral::DeviceRegistry::Connected(Cmd.SerialNumber)) {
		uCentralCommandDetails ResCmd;
		while (Timeout > 0) {
			Timeout -= 1000;
			Poco::Thread::sleep(1000);

			if (uCentral::Storage::GetCommand(Cmd.UUID, ResCmd)) {
				if (ResCmd.Completed) {
					Poco::JSON::Object RetObj;
					ResCmd.to_json(RetObj);
					ReturnObject(RetObj, Response);
					return;
				}
			}
		}
	}
	Poco::JSON::Object RetObj;
	Cmd.to_json(RetObj);
	ReturnObject(RetObj, Response);
}

bool RESTAPIHandler::ContinueProcessing(Poco::Net::HTTPServerRequest & Request, Poco::Net::HTTPServerResponse & Response )
{
    if(Request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS)
    {
        ProcessOptions(Response);
        return false;
    } else if(std::find(methods_.begin(),methods_.end(),Request.getMethod()) == methods_.end())
    {
        BadRequest(Response);
        return false;
    }

    return true;
}

bool RESTAPIHandler::IsAuthorized(Poco::Net::HTTPServerRequest & Request, Poco::Net::HTTPServerResponse & Response )
{
    if(uCentral::Auth::IsAuthorized(Request,SessionToken_, UserInfo_))
    {
        return true;
    }
    else {
        UnAuthorized(Response);
    }
    return false;
}

bool RESTAPIHandler::IsAuthorized(Poco::Net::HTTPServerRequest & Request, Poco::Net::HTTPServerResponse & Response , std::string & UserName ) {

    if(uCentral::Auth::IsAuthorized(Request,SessionToken_, UserInfo_))
    {
        UserName = UserInfo_.username_ ;
        return true;
    }
    else {
        UnAuthorized(Response);
    }
    return false;
}


void RESTAPIHandler::ReturnObject(Poco::JSON::Object & Object, Poco::Net::HTTPServerResponse & Response) {
    PrepareResponse(Response);
    std::ostream & Answer = Response.send();
    Poco::JSON::Stringifier::stringify(Object, Answer);
}
