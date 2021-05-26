//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
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

#include "RESTAPI_objects.h"
#include "uAuthService.h"

namespace uCentral::RESTAPI {

	struct QueryBlock {
		uint64_t StartDate = 0 , EndDate = 0 , Offset = 0 , Limit = 0, LogType = 0 ;
		std::string SerialNumber, Filter, Select;
		bool Lifetime=false, LastOnly=false;
	};

	class RESTAPIHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		typedef std::map<std::string, std::string> BindingMap;

		RESTAPIHandler(BindingMap map, Poco::Logger &l, std::vector<std::string> Methods)
			: Bindings_(std::move(map)), Logger_(l), Methods_(std::move(Methods)) {}

		static bool ParseBindings(const char *path, const char *resource, BindingMap &Keys);
		void PrintBindings();
		void ParseParameters(Poco::Net::HTTPServerRequest &request);

		void AddCORS(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &response);
		void SetCommonHeaders(Poco::Net::HTTPServerResponse &response);
		void ProcessOptions(Poco::Net::HTTPServerRequest &Request,
							Poco::Net::HTTPServerResponse &response);
		void
		PrepareResponse(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &response,
						Poco::Net::HTTPResponse::HTTPStatus Status = Poco::Net::HTTPResponse::HTTP_OK);
		bool ContinueProcessing(Poco::Net::HTTPServerRequest &Request,
								Poco::Net::HTTPServerResponse &Response);
		bool IsAuthorized(Poco::Net::HTTPServerRequest &Request,
						  Poco::Net::HTTPServerResponse &Response);
		bool IsAuthorized(Poco::Net::HTTPServerRequest &Request,
						  Poco::Net::HTTPServerResponse &Response, std::string &UserName);
		uint64_t GetParameter(const std::string &Name, uint64_t Default);
		std::string GetParameter(const std::string &Name, const std::string &Default);
		bool GetBoolParameter(const std::string &Name, bool Default);
		bool ValidateAPIKey(Poco::Net::HTTPServerRequest &Request,
							Poco::Net::HTTPServerResponse &Response);

		void BadRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
		void UnAuthorized(Poco::Net::HTTPServerRequest &Request,
						  Poco::Net::HTTPServerResponse &Response);
		void ReturnObject(Poco::Net::HTTPServerRequest &Request, Poco::JSON::Object &Object,
						  Poco::Net::HTTPServerResponse &Response);
		void NotFound(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
		void OK(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
		bool WaitForRPC(uCentral::Objects::CommandDetails &Cmd, Poco::Net::HTTPServerRequest &Request,
						Poco::Net::HTTPServerResponse &Response, uint64_t Timeout = 5000,
						bool ReturnObject = true);
		void ReturnStatus(Poco::Net::HTTPServerRequest &Request,
						  Poco::Net::HTTPServerResponse &Response,
						  Poco::Net::HTTPResponse::HTTPStatus Status);

		const std::string &GetBinding(const std::string &Name, const std::string &Default);
		void InitQueryBlock();

		[[nodiscard]] inline bool HasReadAccess() const {
			return UserInfo_.acl_template_.Read_ || UserInfo_.acl_template_.ReadWrite_ ||
				   UserInfo_.acl_template_.ReadWriteCreate_;
		}
		[[nodiscard]] inline bool HasWriteAccess() const {
			return UserInfo_.acl_template_.ReadWrite_ || UserInfo_.acl_template_.ReadWriteCreate_;
		}
		[[nodiscard]] inline bool HasCreateAccess() const {
			return UserInfo_.acl_template_.ReadWriteCreate_;
		}
		[[nodiscard]] static uint64_t Get(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, uint64_t Default=0);
		[[nodiscard]] static std::string GetS(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, const std::string & Default="");
		[[nodiscard]] static bool GetB(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, bool Default=false);
		[[nodiscard]] static uint64_t GetWhen(const Poco::JSON::Object::Ptr &Obj);

	  protected:
		BindingMap Bindings_;
		Poco::URI::QueryParameters Parameters_;
		Poco::Logger &Logger_;
		std::string SessionToken_;
		struct uCentral::Objects::WebToken UserInfo_;
		std::vector<std::string> Methods_;
		QueryBlock		QB_;
	};
}

#endif //UCENTRAL_RESTAPI_HANDLER_H
