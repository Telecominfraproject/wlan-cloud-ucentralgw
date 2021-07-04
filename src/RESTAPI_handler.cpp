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
#include <future>
#include <chrono>

#include "Poco/URI.h"
#include "Poco/Net/OAuth20Credentials.h"

#ifdef	TIP_SECURITY_SERVICE
#include "AuthService.h"
#else
#include "AuthClient.h"
#endif

#include "RESTAPI_handler.h"
#include "RESTAPI_protocol.h"
#include "Utils.h"

namespace uCentral {

	bool RESTAPIHandler::ParseBindings(const std::string & Request, const std::list<const char *> & EndPoints, BindingMap &bindings) {
		std::string Param, Value;

		bindings.clear();
		std::vector<std::string> PathItems = uCentral::Utils::Split(Request, '/');

		for(const auto &EndPoint:EndPoints) {
			std::vector<std::string> ParamItems = uCentral::Utils::Split(EndPoint, '/');
			if (PathItems.size() != ParamItems.size())
				continue;

			bool Matched = true;
			for (auto i = 0; i != PathItems.size() && Matched; i++) {
				// std::cout << "PATH:" << PathItems[i] << "  ENDPOINT:" << ParamItems[i] << std::endl;
				if (PathItems[i] != ParamItems[i]) {
					if (ParamItems[i][0] == '{') {
						auto ParamName = ParamItems[i].substr(1, ParamItems[i].size() - 2);
						bindings[ParamName] = PathItems[i];
					} else {
						Matched = false;
					}
				}
			}
			if(Matched)
				return true;
		}
		return false;
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

	void RESTAPIHandler::SetCommonHeaders(Poco::Net::HTTPServerResponse &Response, bool CloseConnection) {
		Response.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
		Response.setChunkedTransferEncoding(true);
		Response.setContentType("application/json");
		if(CloseConnection) {
			Response.set("Connection", "close");
			Response.setKeepAlive(false);
		} else {
			Response.setKeepAlive(true);
			Response.set("Connection", "Keep-Alive");
			Response.set("Keep-Alive", "timeout=5, max=1000");
		}
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
										 Poco::Net::HTTPResponse::HTTPStatus Status,
										 bool CloseConnection) {
		Response.setStatus(Status);
		AddCORS(Request, Response);
		SetCommonHeaders(Response, CloseConnection);
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

	void RESTAPIHandler::SendFile(Poco::File & File, const std::string & UUID, Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
		Response.set("Content-Type","application/octet-stream");
		Response.set("Content-Disposition", "attachment; filename=" + UUID );
		Response.set("Content-Transfer-Encoding","binary");
		Response.set("Accept-Ranges", "bytes");
		Response.set("Cache-Control", "private");
		Response.set("Pragma", "private");
		Response.set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
		Response.set("Content-Length", std::to_string(File.getSize()));
		AddCORS(Request, Response);
		Response.sendFile(File.path(),"application/octet-stream");
	}

	void RESTAPIHandler::ReturnStatus(Poco::Net::HTTPServerRequest &Request,
									  Poco::Net::HTTPServerResponse &Response,
									  Poco::Net::HTTPResponse::HTTPStatus Status,
									  bool CloseConnection) {
		PrepareResponse(Request, Response, Status, CloseConnection);
		if(Status == Poco::Net::HTTPResponse::HTTP_NO_CONTENT) {
			Response.setContentLength(0);
			Response.erase("Content-Type");
			Response.setChunkedTransferEncoding(false);
		}
		Response.send();
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
		if(SessionToken_.empty()) {
			try {
				Poco::Net::OAuth20Credentials Auth(Request);

				if (Auth.getScheme() == "Bearer") {
					SessionToken_ = Auth.getBearerToken();
				}
			} catch(const Poco::Exception &E) {
				Logger_.log(E);
			}
		}
#ifdef	TIP_SECURITY_SERVICE
		if (AuthService()->IsAuthorized(Request, SessionToken_, UserInfo_)) {
#else
		if (AuthClient()->IsAuthorized(Request, SessionToken_, UserInfo_)) {
#endif
			return true;
		} else {
			UnAuthorized(Request, Response);
		}
		return false;
	}

	bool RESTAPIHandler::IsAuthorized(Poco::Net::HTTPServerRequest &Request,
									  Poco::Net::HTTPServerResponse &Response, std::string &UserName) {

#ifdef	TIP_SECURITY_SERVICE
		if (AuthService()->IsAuthorized(Request, SessionToken_, UserInfo_)) {
#else
		if (AuthClient()->IsAuthorized(Request, SessionToken_, UserInfo_)) {
#endif
			UserName = UserInfo_.webtoken.username_;
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
		QB_.Newest = GetBoolParameter(uCentral::RESTAPI::Protocol::NEWEST,false);
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