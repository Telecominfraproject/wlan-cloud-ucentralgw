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

#include "RESTAPI_protocol.h"

namespace uCentral::RESTAPI {
	bool RESTAPIHandler::ParseBindings(const char *p, const char *r, BindingMap &bindings) {
		char param[256] = {0}, value[256] = {0};

		bindings.clear();
		while (*r) {
			if (*r == '{') {
				r++;
				auto pi = 0;
				while (*r != '}' && pi < sizeof(param))
					param[pi++] = *r++;
				r++;
				param[pi] = 0;
				auto vi = 0;
				while (*p != '/' && *p && vi < sizeof(value))
					value[vi++] = *p++;
				value[vi] = 0;
				bindings[param] = value;
			} else if (*p != *r) {
				return false;
			} else {
				r++;
				p++;
			}
		}

		return (*p == *r);
	}

	void RESTAPIHandler::PrintBindings() {
		for (auto &[key, value] : Bindings_)
			std::cout << "Key = " << key << "  Value= " << value << std::endl;
	}

	void RESTAPIHandler::ParseParameters(Poco::Net::HTTPServerRequest &request) {

		Poco::URI uri(request.getURI());
		Parameters_ = uri.getQueryParameters();
	}

	static bool is_number(const std::string &s) {
		return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
	}

	static bool is_bool(const std::string &s) {
		if (s == "true" || s == "false")
			return true;
		return false;
	}

	uint64_t RESTAPIHandler::GetParameter(const std::string &Name, const uint64_t Default) {

		for (const auto &i : Parameters_) {
			if (i.first == Name) {
				if (!is_number(i.second))
					return Default;
				return std::stoi(i.second);
			}
		}
		return Default;
	}

	bool RESTAPIHandler::GetBoolParameter(const std::string &Name, bool Default) {

		for (const auto &i : Parameters_) {
			if (i.first == Name) {
				if (!is_bool(i.second))
					return Default;
				return i.second == "true";
			}
		}
		return Default;
	}

	std::string RESTAPIHandler::GetParameter(const std::string &Name, const std::string &Default) {
		for (const auto &i : Parameters_) {
			if (i.first == Name)
				return i.second;
		}
		return Default;
	}

	const std::string &RESTAPIHandler::GetBinding(const std::string &Name, const std::string &Default) {
		auto E = Bindings_.find(Name);

		if (E == Bindings_.end())
			return Default;

		return E->second;
	}

	static std::string MakeList(const std::vector<std::string> &L) {
		std::string Return;

		for (const auto &i : L)
			if (Return.empty())
				Return = i;
			else
				Return += ", " + i;

		return Return;
	}

	void RESTAPIHandler::AddCORS(Poco::Net::HTTPServerRequest &Request,
								 Poco::Net::HTTPServerResponse &Response) {
		auto Origin = Request.find("Origin");
		if (Origin != Request.end()) {
			Response.set("Access-Control-Allow-Origin", Origin->second);
			Response.set("Vary", "Origin");
		} else {
			Response.set("Access-Control-Allow-Origin", "*");
		}
		Response.set("Access-Control-Allow-Headers", "*");
		Response.set("Access-Control-Allow-Methods", MakeList(Methods_));
		Response.set("Access-Control-Max-Age", "86400");
	}

	void RESTAPIHandler::SetCommonHeaders(Poco::Net::HTTPServerResponse &Response) {
		Response.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
		Response.setChunkedTransferEncoding(true);
		Response.setKeepAlive(true);
		Response.setContentType("application/json");
		Response.set("Connection", "Keep-Alive");
		Response.set("Keep-Alive", "timeout=5, max=1000");
	}

	void RESTAPIHandler::ProcessOptions(Poco::Net::HTTPServerRequest &Request,
										Poco::Net::HTTPServerResponse &Response) {
		AddCORS(Request, Response);
		SetCommonHeaders(Response);
		Response.setContentLength(0);
		Response.set("Access-Control-Allow-Credentials", "true");
		Response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
		Response.set("Vary", "Origin, Access-Control-Request-Headers, Access-Control-Request-Method");
		/*	std::cout << "RESPONSE:" << std::endl;
			for(const auto &[f,s]:Response)
				std::cout << "First: " << f << " second:" << s << std::endl;
		*/
		Response.send();
	}

	void RESTAPIHandler::PrepareResponse(Poco::Net::HTTPServerRequest &Request,
										 Poco::Net::HTTPServerResponse &Response,
										 Poco::Net::HTTPResponse::HTTPStatus Status) {
		Response.setStatus(Status);
		AddCORS(Request, Response);
		SetCommonHeaders(Response);
	}

	void RESTAPIHandler::BadRequest(Poco::Net::HTTPServerRequest &Request,
									Poco::Net::HTTPServerResponse &Response) {
		PrepareResponse(Request, Response, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		Response.send();
	}

	void RESTAPIHandler::UnAuthorized(Poco::Net::HTTPServerRequest &Request,
									  Poco::Net::HTTPServerResponse &Response) {
		PrepareResponse(Request, Response, Poco::Net::HTTPResponse::HTTP_FORBIDDEN);
		Response.send();
	}

	void RESTAPIHandler::NotFound(Poco::Net::HTTPServerRequest &Request,
								  Poco::Net::HTTPServerResponse &Response) {
		PrepareResponse(Request, Response, Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
		Response.send();
	}

	void RESTAPIHandler::OK(Poco::Net::HTTPServerRequest &Request,
							Poco::Net::HTTPServerResponse &Response) {
		PrepareResponse(Request, Response);
		Response.send();
	}

	void RESTAPIHandler::ReturnStatus(Poco::Net::HTTPServerRequest &Request,
									  Poco::Net::HTTPServerResponse &Response,
									  Poco::Net::HTTPResponse::HTTPStatus Status) {
		PrepareResponse(Request, Response, Status);
		Response.send();
	}

	bool RESTAPIHandler::WaitForRPC(uCentral::Objects::CommandDetails &Cmd,
									Poco::Net::HTTPServerRequest &Request,
									Poco::Net::HTTPServerResponse &Response, uint64_t Timeout,
									bool ReturnValue) {

		if (uCentral::DeviceRegistry::Connected(Cmd.SerialNumber)) {
			uCentral::Objects::CommandDetails ResCmd;
			while (Timeout > 0) {
				Timeout -= 1000;
				Poco::Thread::sleep(1000);
				if (uCentral::Storage::GetCommand(Cmd.UUID, ResCmd)) {
					if (ResCmd.Completed) {
						if (ReturnValue) {
							Poco::JSON::Object RetObj;
							ResCmd.to_json(RetObj);
							ReturnObject(Request, RetObj, Response);
						}
						return true;
					}
				}
			}
		}
		if (ReturnValue) {
			Poco::JSON::Object RetObj;
			Cmd.to_json(RetObj);
			ReturnObject(Request, RetObj, Response);
		}
		return false;
	}

	bool RESTAPIHandler::ContinueProcessing(Poco::Net::HTTPServerRequest &Request,
											Poco::Net::HTTPServerResponse &Response) {
		if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
			/*		std::cout << "REQUEST:" << std::endl;
					for(const auto &[f,s]:Request)
						std::cout << "First: " << f << " second:" << s << std::endl;
			*/
			ProcessOptions(Request, Response);
			return false;
		} else if (std::find(Methods_.begin(), Methods_.end(), Request.getMethod()) == Methods_.end()) {
			BadRequest(Request, Response);
			return false;
		}

		return true;
	}

	bool RESTAPIHandler::IsAuthorized(Poco::Net::HTTPServerRequest &Request,
									  Poco::Net::HTTPServerResponse &Response) {
		if (uCentral::Auth::IsAuthorized(Request, SessionToken_, UserInfo_)) {
			return true;
		} else {
			UnAuthorized(Request, Response);
		}
		return false;
	}

	bool RESTAPIHandler::IsAuthorized(Poco::Net::HTTPServerRequest &Request,
									  Poco::Net::HTTPServerResponse &Response, std::string &UserName) {

		if (uCentral::Auth::IsAuthorized(Request, SessionToken_, UserInfo_)) {
			UserName = UserInfo_.username_;
			return true;
		} else {
			UnAuthorized(Request, Response);
		}
		return false;
	}

	bool RESTAPIHandler::ValidateAPIKey(Poco::Net::HTTPServerRequest &Request,
										Poco::Net::HTTPServerResponse &Response) {
		auto Key = Request.get("X-API-KEY", "");

		if (Key.empty())
			return false;

		return true;
	}

	void RESTAPIHandler::ReturnObject(Poco::Net::HTTPServerRequest &Request, Poco::JSON::Object &Object,
									  Poco::Net::HTTPServerResponse &Response) {
		PrepareResponse(Request, Response);
		std::ostream &Answer = Response.send();
		Poco::JSON::Stringifier::stringify(Object, Answer);
	}

	void RESTAPIHandler::InitQueryBlock() {
		QB_.SerialNumber = GetParameter(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
		QB_.StartDate = GetParameter(uCentral::RESTAPI::Protocol::STARTDATE, 0);
		QB_.EndDate = GetParameter(uCentral::RESTAPI::Protocol::ENDDATE, 0);
		QB_.Offset = GetParameter(uCentral::RESTAPI::Protocol::OFFSET, 0);
		QB_.Limit = GetParameter(uCentral::RESTAPI::Protocol::LIMIT, 100);
		QB_.Filter = GetParameter(uCentral::RESTAPI::Protocol::FILTER, "");
		QB_.Select = GetParameter(uCentral::RESTAPI::Protocol::SELECT, "");
		QB_.Lifetime = GetBoolParameter(uCentral::RESTAPI::Protocol::LIFETIME,false);
		QB_.LogType = GetParameter(uCentral::RESTAPI::Protocol::LOGTYPE,0);
		QB_.LastOnly = GetBoolParameter(uCentral::RESTAPI::Protocol::LASTONLY,false);
	}

	[[nodiscard]] uint64_t RESTAPIHandler::Get(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, uint64_t Default){
		if(Obj->has(Parameter))
			return Obj->get(Parameter);
		return Default;
	}

	[[nodiscard]] std::string RESTAPIHandler::GetS(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, const std::string & Default){
		if(Obj->has(Parameter))
			return Obj->get(Parameter).toString();
		return Default;
	}

	[[nodiscard]] bool RESTAPIHandler::GetB(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, bool Default){
		if(Obj->has(Parameter))
			return Obj->get(Parameter).toString()=="true";
		return Default;
	}

	[[nodiscard]] uint64_t RESTAPIHandler::GetWhen(const Poco::JSON::Object::Ptr &Obj) {
		return RESTAPIHandler::Get(uCentral::RESTAPI::Protocol::WHEN, Obj);
	}


}