//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>
#include <vector>
#include <map>

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Logger.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/DeflatingStream.h"
#include "Poco/TemporaryFile.h"
#include "Poco/Net/OAuth20Credentials.h"

#include "framework/ow_constants.h"
#include "framework/RESTAPI_GenericServerAccounting.h"
#include "framework/RESTAPI_RateLimiter.h"
#include "framework/utils.h"
#include "framework/RESTAPI_utils.h"
#include "framework/AuthClient.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"

#if defined(TIP_SECURITY_SERVICE)
#include "AuthService.h"
#endif

using namespace std::chrono_literals;

namespace OpenWifi {

	class RESTAPIHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		struct QueryBlock {
			uint64_t StartDate = 0 , EndDate = 0 , Offset = 0 , Limit = 0, LogType = 0 ;
			std::string SerialNumber, Filter;
			std::vector<std::string>    Select;
			bool Lifetime=false, LastOnly=false, Newest=false, CountOnly=false, AdditionalInfo=false;
		};
		typedef std::map<std::string, std::string> BindingMap;

		struct RateLimit {
			int64_t     Interval=1000;
			int64_t     MaxCalls=10;
		};

		RESTAPIHandler( BindingMap map,
					   Poco::Logger &l,
					   std::vector<std::string> Methods,
					   RESTAPI_GenericServerAccounting & Server,
					   uint64_t TransactionId,
					   bool Internal,
					   bool AlwaysAuthorize=true,
					   bool RateLimited=false,
					   const RateLimit & Profile = RateLimit{.Interval=1000,.MaxCalls=100},
					   bool SubscriberOnly=false)
			:   Bindings_(std::move(map)),
			  Logger_(l),
			  Methods_(std::move(Methods)),
			  Internal_(Internal),
			  RateLimited_(RateLimited),
			  SubOnlyService_(SubscriberOnly),
			  AlwaysAuthorize_(AlwaysAuthorize),
			  Server_(Server),
			  MyRates_(Profile),
			  TransactionId_(TransactionId)
		{
		}

		inline bool RoleIsAuthorized([[maybe_unused]] const std::string & Path, [[maybe_unused]] const std::string & Method, [[maybe_unused]] std::string & Reason) {
			return true;
		}

		inline void handleRequest(Poco::Net::HTTPServerRequest &RequestIn,
								  Poco::Net::HTTPServerResponse &ResponseIn) final {
			try {
				Request = &RequestIn;
				Response = &ResponseIn;

				//				std::string th_name = "restsvr_" + std::to_string(TransactionId_);
				//				Utils::SetThreadName(th_name.c_str());

				if(Request->getContentLength()>0) {
					if(Request->getContentType().find("application/json")!=std::string::npos) {
						ParsedBody_ = IncomingParser_.parse(Request->stream()).extract<Poco::JSON::Object::Ptr>();
					}
				}

				if(RateLimited_ && RESTAPI_RateLimiter()->IsRateLimited(RequestIn,MyRates_.Interval, MyRates_.MaxCalls)) {
					return UnAuthorized(RESTAPI::Errors::RATE_LIMIT_EXCEEDED);
				}

				if (!ContinueProcessing())
					return;

				bool Expired=false, Contacted=false;
				if (AlwaysAuthorize_ && !IsAuthorized(Expired, Contacted, SubOnlyService_)) {
					if(Expired)
						return UnAuthorized(RESTAPI::Errors::EXPIRED_TOKEN);
					if(Contacted)
						return UnAuthorized(RESTAPI::Errors::INVALID_TOKEN);
					else
						return UnAuthorized(RESTAPI::Errors::SECURITY_SERVICE_UNREACHABLE);
				}

				std::string Reason;
				if(!RoleIsAuthorized(RequestIn.getURI(), Request->getMethod(), Reason)) {
					return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
				}

				ParseParameters();
				if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
					return DoGet();
				else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
					return DoPost();
				else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
					return DoDelete();
				else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
					return DoPut();
				return BadRequest(RESTAPI::Errors::UnsupportedHTTPMethod);
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				return BadRequest(RESTAPI::Errors::InternalError);
			}
		}

		[[nodiscard]] inline bool NeedAdditionalInfo() const { return QB_.AdditionalInfo; }
		[[nodiscard]] inline const std::vector<std::string> & SelectedRecords() const { return QB_.Select; }

		inline static bool ParseBindings(const std::string & Request, const std::list<std::string> & EndPoints, BindingMap &bindings) {
			bindings.clear();
			auto PathItems = Poco::StringTokenizer(Request, "/");

			for(const auto &EndPoint:EndPoints) {
				auto ParamItems = Poco::StringTokenizer(EndPoint, "/");
				if (PathItems.count() != ParamItems.count())
					continue;

				bool Matched = true;
				for (size_t i = 0; i < PathItems.count(); i++) {
					if (PathItems[i] != ParamItems[i]) {
						if (ParamItems[i][0] == '{') {
							auto ParamName = ParamItems[i].substr(1, ParamItems[i].size() - 2);
							bindings[Poco::toLower(ParamName)] = PathItems[i];
						} else {
							Matched = false;
							break;
						}
					}
				}
				if(Matched)
					return true;
			}
			return false;
		}

		inline void PrintBindings() {
			for (const auto &[key, value] : Bindings_)
				std::cout << "Key = " << key << "  Value= " << value << std::endl;
		}

		inline void ParseParameters() {
			Poco::URI uri(Request->getURI());
			Parameters_ = uri.getQueryParameters();
			InitQueryBlock();
		}

		inline static bool is_number(const std::string &s) {
			return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
		}

		inline static bool is_bool(const std::string &s) {
			if (s == "true" || s == "false")
				return true;
			return false;
		}

		[[nodiscard]] inline uint64_t GetParameter(const std::string &Name, const uint64_t Default) {
			auto Hint = std::find_if(Parameters_.begin(),Parameters_.end(),[&](const std::pair<std::string,std::string> &S){ return S.first==Name; });
			if(Hint==Parameters_.end() || !is_number(Hint->second))
				return Default;
			return std::stoull(Hint->second);
		}

		[[nodiscard]] inline bool GetBoolParameter(const std::string &Name, bool Default=false) {
			auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[&](const std::pair<std::string,std::string> &S){ return S.first==Name; });
			if(Hint==end(Parameters_) || !is_bool(Hint->second))
				return Default;
			return Hint->second=="true";
		}

		[[nodiscard]] inline std::string GetParameter(const std::string &Name, const std::string &Default="") {
			auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[&](const std::pair<std::string,std::string> &S){ return S.first==Name; });
			if(Hint==end(Parameters_))
				return Default;
			return Hint->second;
		}

		[[nodiscard]] inline bool HasParameter(const std::string &Name, std::string &Value) {
			auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[&](const std::pair<std::string,std::string> &S){ return S.first==Name; });
			if(Hint==end(Parameters_))
				return false;
			Value = Hint->second;
			return true;
		}

		[[nodiscard]] inline bool HasParameter(const std::string &Name, uint64_t & Value) {
			auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[&](const std::pair<std::string,std::string> &S){ return S.first==Name; });
			if(Hint==end(Parameters_))
				return false;
			Value = std::stoull(Hint->second);
			return true;
		}

		[[nodiscard]] inline const std::string & GetBinding(const std::string &Name, const std::string &Default="") {
			auto E = Bindings_.find(Poco::toLower(Name));
			if (E == Bindings_.end())
				return Default;

			return E->second;
		}

		[[nodiscard]] inline static std::string MakeList(const std::vector<std::string> &L) {
			std::string Return;
			for (const auto &i : L) {
				if (Return.empty())
					Return = i;
				else
					Return += ", " + i;
			}
			return Return;
		}

		static inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, Types::UUIDvec_t & Value) {
			if(O->has(Field) && O->isArray(Field)) {
				auto Arr = O->getArray(Field);
				for(const auto &i:*Arr)
					Value.emplace_back(i.toString());
				return true;
			}
			return false;
		}

		static inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, std::string &Value) {
			if(O->has(Field)) {
				Value = O->get(Field).toString();
				return true;
			}
			return false;
		}

		static inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, uint64_t &Value) {
			if(O->has(Field)) {
				Value = O->get(Field);
				return true;
			}
			return false;
		}

		static inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, bool &Value) {
			if(O->has(Field)) {
				Value = O->get(Field).toString()=="true";
				return true;
			}
			return false;
		}

		static inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, double &Value) {
			if(O->has(Field)) {
				Value = (double) O->get(Field);
				return true;
			}
			return false;
		}

		static inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, Poco::Data::BLOB &Value) {
			if(O->has(Field)) {
				std::string Content = O->get(Field).toString();
				auto DecodedBlob = Utils::base64decode(Content);
				Value.assignRaw((const unsigned char *)&DecodedBlob[0],DecodedBlob.size());
				return true;
			}
			return false;
		}


		template <typename T> bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, const T &value, T & assignee) {
			if(O->has(Field)) {
				assignee = value;
				return true;
			}
			return false;
		}

		inline void SetCommonHeaders(bool CloseConnection=false) {
			Response->setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
			Response->setChunkedTransferEncoding(true);
			Response->setContentType("application/json");
			auto Origin = Request->find("Origin");
			if (Origin != Request->end() && !AllowExternalMicroServices()) {
				Response->set("Access-Control-Allow-Origin", Origin->second);
			} else {
				Response->set("Access-Control-Allow-Origin", "*");
			}
			Response->set("Vary", "Origin, Accept-Encoding");
			if(CloseConnection) {
				Response->set("Connection", "close");
				Response->setKeepAlive(false);
			} else {
				Response->setKeepAlive(true);
				Response->set("Connection", "Keep-Alive");
				Response->set("Keep-Alive", "timeout=30, max=1000");
			}
		}

		inline void ProcessOptions() {
			Response->setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
			Response->setChunkedTransferEncoding(true);
			auto Origin = Request->find("Origin");
			if (Origin != Request->end() && !AllowExternalMicroServices()) {
				Response->set("Access-Control-Allow-Origin", Origin->second);
			} else {
				Response->set("Access-Control-Allow-Origin", "*");
			}
			Response->set("Access-Control-Allow-Methods", MakeList(Methods_));
			auto RequestHeaders = Request->find("Access-Control-Request-Headers");
			if(RequestHeaders!=Request->end())
				Response->set("Access-Control-Allow-Headers", RequestHeaders->second);
			Response->set("Vary", "Origin, Accept-Encoding");
			Response->set("Access-Control-Allow-Credentials", "true");
			Response->set("Access-Control-Max-Age", "86400");
			Response->set("Connection", "Keep-Alive");
			Response->set("Keep-Alive", "timeout=30, max=1000");

			Response->setContentLength(0);
			Response->setStatus(Poco::Net::HTTPResponse::HTTP_OK);
			Response->send();
		}

		inline void PrepareResponse(Poco::Net::HTTPResponse::HTTPStatus Status = Poco::Net::HTTPResponse::HTTP_OK,
									bool CloseConnection = false) {
			Response->setStatus(Status);
			SetCommonHeaders(CloseConnection);
		}

		inline void BadRequest(const OpenWifi::RESTAPI::Errors::msg &E, const std::string & Extra="") {
			PrepareResponse(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			Poco::JSON::Object	ErrorObject;
			ErrorObject.set("ErrorCode",400);
			ErrorObject.set("ErrorDetails",Request->getMethod());
			if(Extra.empty())
				ErrorObject.set("ErrorDescription",fmt::format("{}: {}",E.err_num,E.err_txt)) ;
			else
				ErrorObject.set("ErrorDescription",fmt::format("{}: {} ({})",E.err_num,E.err_txt, Extra)) ;

			std::ostream &Answer = Response->send();
			Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
		}

		inline void InternalError(const OpenWifi::RESTAPI::Errors::msg &E) {
			PrepareResponse(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
			Poco::JSON::Object	ErrorObject;
			ErrorObject.set("ErrorCode",500);
			ErrorObject.set("ErrorDetails",Request->getMethod());
			ErrorObject.set("ErrorDescription",fmt::format("{}: {}",E.err_num,E.err_txt)) ;
			std::ostream &Answer = Response->send();
			Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
		}

		inline void UnAuthorized(const OpenWifi::RESTAPI::Errors::msg &E) {
			PrepareResponse(Poco::Net::HTTPResponse::HTTP_FORBIDDEN);
			Poco::JSON::Object	ErrorObject;
			ErrorObject.set("ErrorCode",E.err_num);
			ErrorObject.set("ErrorDetails",Request->getMethod());
			ErrorObject.set("ErrorDescription",fmt::format("{}: {}",E.err_num,E.err_txt)) ;
			std::ostream &Answer = Response->send();
			Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
		}

		inline void NotFound() {
			PrepareResponse(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
			Poco::JSON::Object	ErrorObject;
			ErrorObject.set("ErrorCode",404);
			ErrorObject.set("ErrorDetails",Request->getMethod());
			const auto & E = OpenWifi::RESTAPI::Errors::Error404;
			ErrorObject.set("ErrorDescription",fmt::format("{}: {}",E.err_num,E.err_txt)) ;
			std::ostream &Answer = Response->send();
			Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
			poco_debug(Logger_,fmt::format("RES-NOTFOUND: User='{}@{}' Method='{}' Path='{}",
                                            Requester(),
											Utils::FormatIPv6(Request->clientAddress().toString()),
											Request->getMethod(),
											Request->getURI()));
		}

		inline void OK() {
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

		inline void SendCompressedTarFile(const std::string & FileName, const std::string & Content) {
			Response->setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
			SetCommonHeaders();
			Response->set("Content-Type","application/gzip");
			Response->set("Content-Disposition", "attachment; filename=" + FileName );
			Response->set("Content-Transfer-Encoding","binary");
			Response->set("Accept-Ranges", "bytes");
			Response->set("Cache-Control", "no-store");
			Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
			Response->setStatus(Poco::Net::HTTPResponse::HTTP_OK);
			Response->setContentLength(Content.size());
			Response->setChunkedTransferEncoding(true);
			std::ostream& OutputStream = Response->send();
			OutputStream << Content;
		}

		inline void SendFile(Poco::File & File, const std::string & UUID) {
			Response->setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
			SetCommonHeaders();
			Response->set("Content-Type","application/octet-stream");
			Response->set("Content-Disposition", "attachment; filename=" + UUID );
			Response->set("Content-Transfer-Encoding","binary");
			Response->set("Accept-Ranges", "bytes");
			Response->set("Cache-Control", "no-store");
			Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
			Response->setContentLength(File.getSize());
			Response->sendFile(File.path(),"application/octet-stream");
		}

		inline void SendFile(Poco::File & File) {
			Response->setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
			SetCommonHeaders();
			Poco::Path  P(File.path());
			auto MT = Utils::FindMediaType(File);
			if(MT.Encoding==Utils::BINARY) {
				Response->set("Content-Transfer-Encoding","binary");
				Response->set("Accept-Ranges", "bytes");
			}
			Response->set("Cache-Control", "no-store");
			Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
			Response->sendFile(File.path(),MT.ContentType);
		}

		inline void SendFile(Poco::TemporaryFile &TempAvatar, [[maybe_unused]] const std::string &Type, const std::string & Name) {
			Response->setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
			SetCommonHeaders();
			auto MT = Utils::FindMediaType(Name);
			if(MT.Encoding==Utils::BINARY) {
				Response->set("Content-Transfer-Encoding","binary");
				Response->set("Accept-Ranges", "bytes");
			}
			Response->set("Content-Disposition", "attachment; filename=" + Name );
			Response->set("Accept-Ranges", "bytes");
			Response->set("Cache-Control", "no-store");
			Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
			Response->setContentLength(TempAvatar.getSize());
			Response->sendFile(TempAvatar.path(),MT.ContentType);
		}

		inline void SendFileContent(const std::string &Content, const std::string &Type, const std::string & Name) {
			Response->setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
			SetCommonHeaders();
			auto MT = Utils::FindMediaType(Name);
			if(MT.Encoding==Utils::BINARY) {
				Response->set("Content-Transfer-Encoding","binary");
				Response->set("Accept-Ranges", "bytes");
			}
			Response->set("Content-Disposition", "attachment; filename=" + Name );
			Response->set("Accept-Ranges", "bytes");
			Response->set("Cache-Control", "no-store");
			Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
			Response->setContentLength(Content.size());
			Response->setContentType(Type );
			auto & OutputStream = Response->send();
			OutputStream << Content ;
		}

		inline void SendHTMLFileBack(Poco::File & File,
									 const Types::StringPairVec & FormVars) {
			Response->setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
			SetCommonHeaders();
			Response->set("Pragma", "private");
			Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
			std::string FormContent = Utils::LoadFile(File.path());
			Utils::ReplaceVariables(FormContent, FormVars);
			Response->setContentLength(FormContent.size());
			Response->setChunkedTransferEncoding(true);
			Response->setContentType("text/html");
			std::ostream& ostr = Response->send();
			ostr << FormContent;
		}

		inline void ReturnStatus(Poco::Net::HTTPResponse::HTTPStatus Status, bool CloseConnection=false) {
			PrepareResponse(Status, CloseConnection);
			if(Status == Poco::Net::HTTPResponse::HTTP_NO_CONTENT) {
				Response->setContentLength(0);
				Response->erase("Content-Type");
				Response->setChunkedTransferEncoding(false);
			}
			Response->send();
		}

		inline bool ContinueProcessing() {
			if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
				ProcessOptions();
				return false;
			} else if (std::find(Methods_.begin(), Methods_.end(), Request->getMethod()) == Methods_.end()) {
				BadRequest(RESTAPI::Errors::UnsupportedHTTPMethod);
				return false;
			}

			return true;
		}

		inline bool IsAuthorized(bool & Expired, bool & Contacted, bool SubOnly = false );

		inline void ReturnObject(Poco::JSON::Object &Object) {
			PrepareResponse();
			if(Request!= nullptr) {
				//   can we compress ???
				auto AcceptedEncoding = Request->find("Accept-Encoding");
				if(AcceptedEncoding!=Request->end()) {
					if( AcceptedEncoding->second.find("gzip")!=std::string::npos ||
						AcceptedEncoding->second.find("compress")!=std::string::npos) {
						Response->set("Content-Encoding", "gzip");
						std::ostream &Answer = Response->send();
						Poco::DeflatingOutputStream deflater(Answer, Poco::DeflatingStreamBuf::STREAM_GZIP);
						Poco::JSON::Stringifier::stringify(Object, deflater);
						deflater.close();
						return;
					}
				}
			}
			std::ostream &Answer = Response->send();
			Poco::JSON::Stringifier::stringify(Object, Answer);
		}

		inline void ReturnRawJSON(const std::string &json_doc) {
			PrepareResponse();
			if(Request!= nullptr) {
				//   can we compress ???
				auto AcceptedEncoding = Request->find("Accept-Encoding");
				if(AcceptedEncoding!=Request->end()) {
					if( AcceptedEncoding->second.find("gzip")!=std::string::npos ||
						AcceptedEncoding->second.find("compress")!=std::string::npos) {
						Response->set("Content-Encoding", "gzip");
						std::ostream &Answer = Response->send();
						Poco::DeflatingOutputStream deflater(Answer, Poco::DeflatingStreamBuf::STREAM_GZIP);
                        deflater << json_doc;
						deflater.close();
						return;
					}
				}
			}
			std::ostream &Answer = Response->send();
            Answer << json_doc;
		}

		inline void ReturnCountOnly(uint64_t Count) {
			Poco::JSON::Object  Answer;
			Answer.set("count", Count);
			ReturnObject(Answer);
		}

		inline bool InitQueryBlock() {
			if(QueryBlockInitialized_)
				return true;
			QueryBlockInitialized_=true;
			QB_.SerialNumber = GetParameter(RESTAPI::Protocol::SERIALNUMBER, "");
			QB_.StartDate = GetParameter(RESTAPI::Protocol::STARTDATE, 0);
			QB_.EndDate = GetParameter(RESTAPI::Protocol::ENDDATE, 0);
			QB_.Offset = GetParameter(RESTAPI::Protocol::OFFSET, 0);
			QB_.Limit = GetParameter(RESTAPI::Protocol::LIMIT, 100);
			QB_.Filter = GetParameter(RESTAPI::Protocol::FILTER, "");
			QB_.Lifetime = GetBoolParameter(RESTAPI::Protocol::LIFETIME,false);
			QB_.LogType = GetParameter(RESTAPI::Protocol::LOGTYPE,0);
			QB_.LastOnly = GetBoolParameter(RESTAPI::Protocol::LASTONLY,false);
			QB_.Newest = GetBoolParameter(RESTAPI::Protocol::NEWEST,false);
			QB_.CountOnly = GetBoolParameter(RESTAPI::Protocol::COUNTONLY,false);
			QB_.AdditionalInfo = GetBoolParameter(RESTAPI::Protocol::WITHEXTENDEDINFO,false);

			auto RawSelect = GetParameter(RESTAPI::Protocol::SELECT, "");

			auto Entries = Poco::StringTokenizer(RawSelect,",");
			for(const auto &i:Entries) {
				QB_.Select.emplace_back(i);
			}
			if(QB_.Offset<1)
				QB_.Offset=0;
			return true;
		}

		[[nodiscard]] inline uint64_t Get(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, uint64_t Default=0){
			if(Obj->has(Parameter))
				return Obj->get(Parameter);
			return Default;
		}

		[[nodiscard]] inline std::string GetS(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, const std::string & Default=""){
			if(Obj->has(Parameter))
				return Obj->get(Parameter).toString();
			return Default;
		}

		[[nodiscard]] inline bool GetB(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, bool Default=false){
			if(Obj->has(Parameter))
				return Obj->get(Parameter).toString()=="true";
			return Default;
		}

		[[nodiscard]] inline uint64_t GetWhen(const Poco::JSON::Object::Ptr &Obj) {
			return RESTAPIHandler::Get(RESTAPI::Protocol::WHEN, Obj);
		}

		template<typename T> void ReturnObject(const char *Name, const std::vector<T> & Objects) {
			Poco::JSON::Object  Answer;
			RESTAPI_utils::field_to_json(Answer,Name,Objects);
			ReturnObject(Answer);
		}

		template<typename T> void Object(const char *Name, const std::vector<T> & Objects) {
			Poco::JSON::Object  Answer;
			RESTAPI_utils::field_to_json(Answer,Name,Objects);
			ReturnObject(Answer);
		}

		template <typename T> void Object(const T &O) {
			Poco::JSON::Object  Answer;
			O.to_json(Answer);
			ReturnObject(Answer);
		}

		Poco::Logger & Logger() { return Logger_; }

		virtual void DoGet() = 0 ;
		virtual void DoDelete() = 0 ;
		virtual void DoPost() = 0 ;
		virtual void DoPut() = 0 ;

		Poco::Net::HTTPServerRequest        *Request= nullptr;
		Poco::Net::HTTPServerResponse       *Response= nullptr;
		SecurityObjects::UserInfoAndPolicy 	UserInfo_;
		QueryBlock					QB_;
		const std::string & Requester() const { return REST_Requester_; }
	  protected:
		BindingMap 					Bindings_;
		Poco::URI::QueryParameters 	Parameters_;
		Poco::Logger 				&Logger_;
		std::string 				SessionToken_;
		std::vector<std::string> 	Methods_;
		bool                        Internal_=false;
		bool                        RateLimited_=false;
		bool                        QueryBlockInitialized_=false;
		bool                        SubOnlyService_=false;
		bool                        AlwaysAuthorize_=true;
		Poco::JSON::Parser          IncomingParser_;
		RESTAPI_GenericServerAccounting       & Server_;
		RateLimit                   MyRates_;
		uint64_t                    TransactionId_;
		Poco::JSON::Object::Ptr     ParsedBody_;
		std::string					REST_Requester_;
	};

#ifdef    TIP_SECURITY_SERVICE
	[[nodiscard]] bool AuthServiceIsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken,
                                               SecurityObjects::UserInfoAndPolicy & UInfo, std::uint64_t TID, bool & Expired , bool Sub );
#endif
	inline bool RESTAPIHandler::IsAuthorized( bool & Expired , [[maybe_unused]] bool & Contacted , bool Sub ) {
		if(Internal_ && Request->has("X-INTERNAL-NAME")) {
			auto Allowed = MicroServiceIsValidAPIKEY(*Request);
			Contacted = true;
			if(!Allowed) {
				if(Server_.LogBadTokens(false)) {
					poco_debug(Logger_,fmt::format("I-REQ-DENIED({}): TID={} Method={} Path={}",
													Utils::FormatIPv6(Request->clientAddress().toString()),
													TransactionId_,
													Request->getMethod(), Request->getURI()));
				}
			} else {
				auto Id = Request->get("X-INTERNAL-NAME", "unknown");
				REST_Requester_ = Id;
				if(Server_.LogIt(Request->getMethod(),true)) {
					poco_debug(Logger_,fmt::format("I-REQ-ALLOWED({}): TID={} User='{}' Method={} Path={}",
													Utils::FormatIPv6(Request->clientAddress().toString()),
													TransactionId_,
													Id,
													Request->getMethod(), Request->getURI()));
				}
			}
			return Allowed;
		} else if(!Internal_ && Request->has("X-API-KEY")) {
            SessionToken_ = Request->get("X-API-KEY", "");
#ifdef    TIP_SECURITY_SERVICE
            std::uint64_t expiresOn;
            if (AuthService()->IsValidApiKey(SessionToken_, UserInfo_.webtoken, UserInfo_.userinfo, Expired, expiresOn)) {
#else
            if (AuthClient()->IsValidApiKey( SessionToken_, UserInfo_, TransactionId_, Expired, Contacted)) {
#endif
                REST_Requester_ = UserInfo_.userinfo.email;
                if(Server_.LogIt(Request->getMethod(),true)) {
                    poco_debug(Logger_,fmt::format("X-REQ-ALLOWED({}): APIKEY-ACCESS TID={} User='{}@{}' Method={} Path={}",
                                                   UserInfo_.userinfo.email,
                                                   TransactionId_,
                                                   Utils::FormatIPv6(Request->clientAddress().toString()),
                                                   Request->clientAddress().toString(),
                                                   Request->getMethod(),
                                                   Request->getURI()));
                }
                return true;
            } else {
                if(Server_.LogBadTokens(true)) {
                    poco_debug(Logger_,fmt::format("X-REQ-DENIED({}): TID={} Method={} Path={}",
                                                   Utils::FormatIPv6(Request->clientAddress().toString()),
                                                   TransactionId_,
                                                   Request->getMethod(),
                                                   Request->getURI()));
                }
            }
            return false;
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
			if (AuthServiceIsAuthorized(*Request, SessionToken_, UserInfo_, TransactionId_, Expired, Sub)) {
#else
			if (AuthClient()->IsAuthorized( SessionToken_, UserInfo_, TransactionId_, Expired, Contacted, Sub)) {
#endif
				REST_Requester_ = UserInfo_.userinfo.email;
				if(Server_.LogIt(Request->getMethod(),true)) {
					poco_debug(Logger_,fmt::format("X-REQ-ALLOWED({}): TID={} User='{}@{}' Method={} Path={}",
													UserInfo_.userinfo.email,
													TransactionId_,
													Utils::FormatIPv6(Request->clientAddress().toString()),
													Request->clientAddress().toString(),
													Request->getMethod(),
													Request->getURI()));
				}
				return true;
			} else {
				if(Server_.LogBadTokens(true)) {
					poco_debug(Logger_,fmt::format("X-REQ-DENIED({}): TID={} Method={} Path={}",
													Utils::FormatIPv6(Request->clientAddress().toString()),
													TransactionId_,
													Request->getMethod(),
													Request->getURI()));
				}
			}
			return false;
		}
	}

	class RESTAPI_UnknownRequestHandler : public RESTAPIHandler {
	  public:
		RESTAPI_UnknownRequestHandler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServerAccounting & Server, uint64_t TransactionId, bool Internal)
			: RESTAPIHandler(bindings, L, std::vector<std::string>{}, Server, TransactionId, Internal) {}
		inline void DoGet() override {};
		inline void DoPost() override {};
		inline void DoPut() override {};
		inline void DoDelete() override {};
	};

	template<class T>
		constexpr auto test_has_PathName_method(T*)
			-> decltype(  T::PathName() , std::true_type{} )
		{
			return std::true_type{};
		}
		constexpr auto test_has_PathName_method(...) -> std::false_type
		{
			return std::false_type{};
		}

	template<typename T, typename... Args>
		RESTAPIHandler * RESTAPI_Router(const std::string & RequestedPath, RESTAPIHandler::BindingMap &Bindings,
									   Poco::Logger & Logger, RESTAPI_GenericServerAccounting & Server, uint64_t TransactionId) {
			static_assert(test_has_PathName_method((T*)nullptr), "Class must have a static PathName() method.");
			if(RESTAPIHandler::ParseBindings(RequestedPath,T::PathName(),Bindings)) {
				return new T(Bindings, Logger, Server, TransactionId, false);
			}

			if constexpr (sizeof...(Args) == 0) {
				return new RESTAPI_UnknownRequestHandler(Bindings,Logger, Server, TransactionId, false);
			} else {
				return RESTAPI_Router<Args...>(RequestedPath, Bindings, Logger, Server, TransactionId);
			}
		}

	template<typename T, typename... Args>
		RESTAPIHandler * RESTAPI_Router_I(const std::string & RequestedPath, RESTAPIHandler::BindingMap &Bindings,
										 Poco::Logger & Logger, RESTAPI_GenericServerAccounting & Server, uint64_t TransactionId) {
			static_assert(test_has_PathName_method((T*)nullptr), "Class must have a static PathName() method.");
			if(RESTAPIHandler::ParseBindings(RequestedPath,T::PathName(),Bindings)) {
				return new T(Bindings, Logger, Server, TransactionId, true );
			}

			if constexpr (sizeof...(Args) == 0) {
				return new RESTAPI_UnknownRequestHandler(Bindings,Logger, Server, TransactionId, true);
			} else {
				return RESTAPI_Router_I<Args...>(RequestedPath, Bindings, Logger, Server, TransactionId);
			}
		}

} // namespace OpenWifi
