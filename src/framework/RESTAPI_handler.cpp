//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <cctype>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <future>
#include <chrono>

#include "Poco/URI.h"
#include "Poco/Net/OAuth20Credentials.h"

#include "RESTAPI_errors.h"

#ifdef	TIP_SECURITY_SERVICE
#include "AuthService.h"
#else
#include "framework/AuthClient.h"
#endif

#include "RESTAPI_handler.h"
#include "RESTAPI_protocol.h"
#include "Utils.h"
#include "Daemon.h"

namespace OpenWifi {

    void RESTAPIHandler::handleRequest(Poco::Net::HTTPServerRequest &RequestIn,
                       Poco::Net::HTTPServerResponse &ResponseIn) {
		try {
			Request = &RequestIn;
			Response = &ResponseIn;

			if (!ContinueProcessing())
				return;

			if (AlwaysAuthorize_ && !IsAuthorized())
				return;

			ParseParameters();
			if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
				DoGet();
			else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
				DoPost();
			else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
				DoDelete();
			else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
				DoPut();
			else
				BadRequest(RESTAPI::Errors::UnsupportedHTTPMethod);
			return;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
			BadRequest(RESTAPI::Errors::InternalError);
		}
	}

    const Poco::JSON::Object::Ptr &RESTAPIHandler::ParseStream() {
        return IncomingParser_.parse(Request->stream()).extract<Poco::JSON::Object::Ptr>();
    }

	bool RESTAPIHandler::ParseBindings(const std::string & Request, const std::list<const char *> & EndPoints, BindingMap &bindings) {
		bindings.clear();
		std::vector<std::string> PathItems = Utils::Split(Request, '/');

		for(const auto &EndPoint:EndPoints) {
			std::vector<std::string> ParamItems = Utils::Split(EndPoint, '/');
			if (PathItems.size() != ParamItems.size())
				continue;

			bool Matched = true;
			for (auto i = 0; i != PathItems.size() && Matched; i++) {
				if (PathItems[i] != ParamItems[i]) {
					if (ParamItems[i][0] == '{') {
						auto ParamName = ParamItems[i].substr(1, ParamItems[i].size() - 2);
						bindings[Poco::toLower(ParamName)] = PathItems[i];
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
		for (const auto &[key, value] : Bindings_)
			std::cout << "Key = " << key << "  Value= " << value << std::endl;
	}

	void RESTAPIHandler::ParseParameters() {
		Poco::URI uri(Request->getURI());
		Parameters_ = uri.getQueryParameters();
		InitQueryBlock();
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
        auto Hint = std::find_if(Parameters_.begin(),Parameters_.end(),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
        if(Hint==Parameters_.end() || !is_number(Hint->second))
            return Default;
        return std::stoull(Hint->second);
	}

	bool RESTAPIHandler::GetBoolParameter(const std::string &Name, bool Default) {
        auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
        if(Hint==end(Parameters_) || !is_bool(Hint->second))
            return Default;
		return Hint->second=="true";
	}

	std::string RESTAPIHandler::GetParameter(const std::string &Name, const std::string &Default) {
        auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
        if(Hint==end(Parameters_))
            return Default;
        return Hint->second;
	}

	bool RESTAPIHandler::HasParameter(const std::string &Name, std::string &Value) {
        auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
        if(Hint==end(Parameters_))
            return false;
        Value = Hint->second;
        return true;
	}

	bool RESTAPIHandler::HasParameter(const std::string &Name, uint64_t & Value) {
        auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
        if(Hint==end(Parameters_))
            return false;
        Value = std::stoull(Hint->second);
        return true;
	}

	const std::string &RESTAPIHandler::GetBinding(const std::string &Name, const std::string &Default) {
		auto E = Bindings_.find(Poco::toLower(Name));
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

	bool RESTAPIHandler::AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, std::string &Value) {
	    if(O->has(Field)) {
	        Value = O->get(Field).toString();
	        return true;
	    }
	    return false;
	}

	bool RESTAPIHandler::AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, uint64_t &Value) {
	    if(O->has(Field)) {
	        Value = O->get(Field);
	        return true;
	    }
	    return false;
	}

	bool RESTAPIHandler::AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, bool &Value) {
        if(O->has(Field)) {
            Value = O->get(Field).toString()=="true";
            return true;
        }
        return false;
    }

	void RESTAPIHandler::AddCORS() {
		auto Origin = Request->find("Origin");
		if (Origin != Request->end()) {
			Response->set("Access-Control-Allow-Origin", Origin->second);
			Response->set("Vary", "Origin");
		} else {
			Response->set("Access-Control-Allow-Origin", "*");
		}
		Response->set("Access-Control-Allow-Headers", "*");
		Response->set("Access-Control-Allow-Methods", MakeList(Methods_));
		Response->set("Access-Control-Max-Age", "86400");
	}

	void RESTAPIHandler::SetCommonHeaders(bool CloseConnection) {
		Response->setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
		Response->setChunkedTransferEncoding(true);
		Response->setContentType("application/json");
		if(CloseConnection) {
			Response->set("Connection", "close");
			Response->setKeepAlive(false);
		} else {
			Response->setKeepAlive(true);
			Response->set("Connection", "Keep-Alive");
			Response->set("Keep-Alive", "timeout=5, max=1000");
		}
	}

	void RESTAPIHandler::ProcessOptions() {
		AddCORS();
		SetCommonHeaders();
		Response->setContentLength(0);
		Response->set("Access-Control-Allow-Credentials", "true");
		Response->setStatus(Poco::Net::HTTPResponse::HTTP_OK);
		Response->set("Vary", "Origin, Access-Control-Request-Headers, Access-Control-Request-Method");
		Response->send();
	}

	void RESTAPIHandler::PrepareResponse( Poco::Net::HTTPResponse::HTTPStatus Status,
										 bool CloseConnection) {
		Response->setStatus(Status);
		AddCORS();
		SetCommonHeaders(CloseConnection);
	}

	void RESTAPIHandler::BadRequest(const std::string & Reason) {
		PrepareResponse(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		Poco::JSON::Object	ErrorObject;
		ErrorObject.set("ErrorCode",400);
		ErrorObject.set("ErrorDetails",Request->getMethod());
		ErrorObject.set("ErrorDescription",Reason.empty() ? "Command is missing parameters or wrong values." : Reason) ;
		std::ostream &Answer = Response->send();
		Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
	}

	void RESTAPIHandler::InternalError(const std::string & Reason) {
        PrepareResponse(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        Poco::JSON::Object	ErrorObject;
        ErrorObject.set("ErrorCode",500);
        ErrorObject.set("ErrorDetails",Request->getMethod());
        ErrorObject.set("ErrorDescription",Reason.empty() ? "Please try later or review the data submitted." : Reason) ;
        std::ostream &Answer = Response->send();
        Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
    }

	void RESTAPIHandler::UnAuthorized(const std::string & Reason) {
		PrepareResponse(Poco::Net::HTTPResponse::HTTP_FORBIDDEN);
		Poco::JSON::Object	ErrorObject;
		ErrorObject.set("ErrorCode",403);
		ErrorObject.set("ErrorDetails",Request->getMethod());
		ErrorObject.set("ErrorDescription",Reason.empty() ? "No access allowed." : Reason) ;
		std::ostream &Answer = Response->send();
		Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
	}

	void RESTAPIHandler::NotFound() {
		PrepareResponse(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
		Poco::JSON::Object	ErrorObject;
		ErrorObject.set("ErrorCode",404);
		ErrorObject.set("ErrorDetails",Request->getMethod());
		ErrorObject.set("ErrorDescription","This resource does not exist.");
		std::ostream &Answer = Response->send();
		Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
		Logger_.debug(Poco::format("RES-NOTFOUND: User='%s@%s' Method='%s' Path='%s",
                                   UserInfo_.userinfo.email,
                                   Utils::FormatIPv6(Request->clientAddress().toString()),
                                   Request->getMethod(),
                                   Request->getURI()));
	}

	void RESTAPIHandler::OK() {
		PrepareResponse();
		if(	Request->getMethod()==Poco::Net::HTTPRequest::HTTP_DELETE ||
			Request->getMethod()==Poco::Net::HTTPRequest::HTTP_OPTIONS) {
			Response->send();
		} else {
			Poco::JSON::Object ErrorObject;
			ErrorObject.set("Code", 0);
			ErrorObject.set("Operation", Request->getMethod());
			ErrorObject.set("Details", "Command completed.");
			std::ostream &Answer = Response->send();
			Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
		}
	}

	void RESTAPIHandler::SendFile(Poco::File & File, const std::string & UUID) {
		Response->set("Content-Type","application/octet-stream");
		Response->set("Content-Disposition", "attachment; filename=" + UUID );
		Response->set("Content-Transfer-Encoding","binary");
		Response->set("Accept-Ranges", "bytes");
		Response->set("Cache-Control", "private");
		Response->set("Pragma", "private");
		Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
		Response->set("Content-Length", std::to_string(File.getSize()));
		AddCORS();
		Response->sendFile(File.path(),"application/octet-stream");
	}

    void RESTAPIHandler::SendFile(Poco::File & File) {
        Poco::Path  P(File.path());
        auto MT = Utils::FindMediaType(File);
        if(MT.Encoding==Utils::BINARY) {
            Response->set("Content-Transfer-Encoding","binary");
            Response->set("Accept-Ranges", "bytes");
        }
        Response->set("Cache-Control", "private");
        Response->set("Pragma", "private");
        Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
        AddCORS();
        Response->sendFile(File.path(),MT.ContentType);
    }

    void RESTAPIHandler::SendFile(Poco::TemporaryFile &TempAvatar, const std::string &Type, const std::string & Name) {
        auto MT = Utils::FindMediaType(Name);
        if(MT.Encoding==Utils::BINARY) {
            Response->set("Content-Transfer-Encoding","binary");
            Response->set("Accept-Ranges", "bytes");
        }
        Response->set("Content-Disposition", "attachment; filename=" + Name );
        Response->set("Accept-Ranges", "bytes");
        Response->set("Cache-Control", "private");
        Response->set("Pragma", "private");
        Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
        AddCORS();
        Response->sendFile(TempAvatar.path(),MT.ContentType);
	}

    void RESTAPIHandler::SendHTMLFileBack(Poco::File & File,
                          const Types::StringPairVec & FormVars) {
        Response->set("Pragma", "private");
        Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
        Response->set("Content-Length", std::to_string(File.getSize()));
        AddCORS();
        auto FormContent = Utils::LoadFile(File.path());
        Utils::ReplaceVariables(FormContent, FormVars);
        Response->setChunkedTransferEncoding(true);
        Response->setContentType("text/html");
        std::ostream& ostr = Response->send();
        ostr << FormContent;
	}

    void RESTAPIHandler::ReturnStatus(Poco::Net::HTTPResponse::HTTPStatus Status, bool CloseConnection) {
		PrepareResponse(Status, CloseConnection);
		if(Status == Poco::Net::HTTPResponse::HTTP_NO_CONTENT) {
			Response->setContentLength(0);
			Response->erase("Content-Type");
			Response->setChunkedTransferEncoding(false);
		}
		Response->send();
	}

	bool RESTAPIHandler::ContinueProcessing() {
		if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
			ProcessOptions();
			return false;
		} else if (std::find(Methods_.begin(), Methods_.end(), Request->getMethod()) == Methods_.end()) {
			BadRequest(RESTAPI::Errors::UnsupportedHTTPMethod);
			return false;
		}

		return true;
	}

	bool RESTAPIHandler::IsAuthorized() {
	    if(Internal_) {
	        auto Allowed = Daemon()->IsValidAPIKEY(*Request);
	        if(!Allowed) {
	            if(Server_.LogBadTokens(false)) {
	                Logger_.debug(Poco::format("I-REQ-DENIED(%s): Method='%s' Path='%s",
                                               Utils::FormatIPv6(Request->clientAddress().toString()),
                                               Request->getMethod(), Request->getURI()));
	            }
	        } else {
	            auto Id = Request->get("X-INTERNAL-NAME", "unknown");
	            if(Server_.LogIt(Request->getMethod(),true)) {
	                Logger_.debug(Poco::format("I-REQ-ALLOWED(%s): User='%s' Method='%s' Path='%s",
                                               Utils::FormatIPv6(Request->clientAddress().toString()), Id,
                                               Request->getMethod(), Request->getURI()));
	            }
	        }
            return Allowed;
	    } else {
            if (SessionToken_.empty()) {
                try {
                    Poco::Net::OAuth20Credentials Auth(*Request);
                    if (Auth.getScheme() == "Bearer") {
                        SessionToken_ = Auth.getBearerToken();
                    }
                } catch (const Poco::Exception &E) {
                    Logger_.log(E);
                }
            }
#ifdef    TIP_SECURITY_SERVICE
            if (AuthService()->IsAuthorized(*Request, SessionToken_, UserInfo_)) {
#else
            if (AuthClient()->IsAuthorized(*Request, SessionToken_, UserInfo_)) {
#endif
                if(Server_.LogIt(Request->getMethod(),true)) {
                    Logger_.debug(Poco::format("X-REQ-ALLOWED(%s): User='%s@%s' Method='%s' Path='%s",
                         UserInfo_.userinfo.email,
                         Utils::FormatIPv6(Request->clientAddress().toString()),
                         Request->clientAddress().toString(),
                         Request->getMethod(),
                         Request->getURI()));
                }
                return true;
            } else {
                if(Server_.LogBadTokens(true)) {
                    Logger_.debug(Poco::format("X-REQ-DENIED(%s): Method='%s' Path='%s",
                         Utils::FormatIPv6(Request->clientAddress().toString()),
                         Request->getMethod(), Request->getURI()));
                }
                UnAuthorized();
            }
            return false;
        }
	}

	void RESTAPIHandler::ReturnObject(Poco::JSON::Object &Object) {
		PrepareResponse();
		std::ostream &Answer = Response->send();
		Poco::JSON::Stringifier::stringify(Object, Answer);
	}

	void RESTAPIHandler::ReturnCountOnly(uint64_t Count) {
	    Poco::JSON::Object  Answer;
	    Answer.set("count", Count);
        ReturnObject(Answer);
	}

	bool RESTAPIHandler::InitQueryBlock() {
	    if(QueryBlockInitialized_)
	        return true;
	    QueryBlockInitialized_=true;
		QB_.SerialNumber = GetParameter(RESTAPI::Protocol::SERIALNUMBER, "");
		QB_.StartDate = GetParameter(RESTAPI::Protocol::STARTDATE, 0);
		QB_.EndDate = GetParameter(RESTAPI::Protocol::ENDDATE, 0);
		QB_.Offset = GetParameter(RESTAPI::Protocol::OFFSET, 1);
		QB_.Limit = GetParameter(RESTAPI::Protocol::LIMIT, 100);
		QB_.Filter = GetParameter(RESTAPI::Protocol::FILTER, "");
		QB_.Select = GetParameter(RESTAPI::Protocol::SELECT, "");
		QB_.Lifetime = GetBoolParameter(RESTAPI::Protocol::LIFETIME,false);
		QB_.LogType = GetParameter(RESTAPI::Protocol::LOGTYPE,0);
		QB_.LastOnly = GetBoolParameter(RESTAPI::Protocol::LASTONLY,false);
		QB_.Newest = GetBoolParameter(RESTAPI::Protocol::NEWEST,false);
		QB_.CountOnly = GetBoolParameter(RESTAPI::Protocol::COUNTONLY,false);
		QB_.AdditionalInfo = GetBoolParameter(RESTAPI::Protocol::WITHEXTENDEDINFO,false);

		if(QB_.Offset<1)
		    QB_.Offset=1;
		return true;
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
		return RESTAPIHandler::Get(RESTAPI::Protocol::WHEN, Obj);
	}
}