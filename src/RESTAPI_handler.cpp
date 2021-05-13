//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <cctype>
#include <algorithm>
#include <iostream>
#include <iterator>

#include "Poco/URI.h"
#include "Poco/DateTimeParser.h"

#include "RESTAPI_handler.h"
#include "uAuthService.h"
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
    for(auto &[key,value]:Bindings_)
        std::cout << "Key = " << key << "  Value= " << value << std::endl;
}

void RESTAPIHandler::ParseParameters(Poco::Net::HTTPServerRequest& request) {

    Poco::URI uri(request.getURI());
    Parameters_ = uri.getQueryParameters();
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

    for(const auto & i:Parameters_)
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

	for(const auto & i:Parameters_)
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
    for(const auto & i:Parameters_)
    {
        if(i.first == Name)
            return i.second;
    }
    return Default;
}

const std::string & RESTAPIHandler::GetBinding(const std::string &Name, const std::string &Default) {
    auto E = Bindings_.find(Name);

    if(E==Bindings_.end())
        return Default;

    return E->second;
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
    Response.add("Access-Control-Allow-Method",MakeList(Methods_));
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
    Response.add("Access-Control-Allow-Method",MakeList(Methods_));
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

void RESTAPIHandler::WaitForRPC(uCentral::Objects::CommandDetails & Cmd, Poco::Net::HTTPServerResponse &Response, uint64_t Timeout) {

	if(uCentral::DeviceRegistry::Connected(Cmd.SerialNumber)) {
		uCentral::Objects::CommandDetails ResCmd;
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
    } else if(std::find(Methods_.begin(),Methods_.end(),Request.getMethod()) == Methods_.end())
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

    if(uCentral::Auth::IsAuthorized(Request, SessionToken_, UserInfo_))
    {
        UserName = UserInfo_.username_ ;
        return true;
    }
    else {
        UnAuthorized(Response);
    }
    return false;
}

bool RESTAPIHandler::ValidateAPIKey(Poco::Net::HTTPServerRequest & Request , Poco::Net::HTTPServerResponse & Response) {
	auto Key = Request.get("X-API-KEY","");

	if(Key.empty())
		return false;

	return true;
}


void RESTAPIHandler::ReturnObject(Poco::JSON::Object & Object, Poco::Net::HTTPServerResponse & Response) {
    PrepareResponse(Response);
    std::ostream & Answer = Response.send();
    Poco::JSON::Stringifier::stringify(Object, Answer);
}
