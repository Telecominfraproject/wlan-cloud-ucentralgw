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
#include "Poco/Net/PartHandler.h"

#include "Poco/Logger.h"
#include "Poco/File.h"
#include "Poco/TemporaryFile.h"
#include "Poco/JSON/Object.h"
#include "Poco/CountingStream.h"
#include "Poco/NullStream.h"

#include "RESTAPI_SecurityObjects.h"

namespace uCentral {

    class RESTAPI_PartHandler: public Poco::Net::PartHandler
    {
    public:
        RESTAPI_PartHandler():
                _length(0)
        {
        }

        void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override
        {
            _type = header.get("Content-Type", "(unspecified)");
            if (header.has("Content-Disposition"))
            {
                std::string disp;
                Poco::Net::NameValueCollection params;
                Poco::Net::MessageHeader::splitParameters(header["Content-Disposition"], disp, params);
                _name = params.get("name", "(unnamed)");
                _fileName = params.get("filename", "(unnamed)");
            }

            Poco::CountingInputStream istr(stream);
            Poco::NullOutputStream ostr;
            Poco::StreamCopier::copyStream(istr, ostr);
            _length = (int)istr.chars();
        }

        [[nodiscard]] int length() const
        {
            return _length;
        }

        [[nodiscard]] const std::string& name() const
        {
            return _name;
        }

        [[nodiscard]] const std::string& fileName() const
        {
            return _fileName;
        }

        [[nodiscard]] const std::string& contentType() const
        {
            return _type;
        }

    private:
        int _length;
        std::string _type;
        std::string _name;
        std::string _fileName;
    };

    class RESTAPIHandler : public Poco::Net::HTTPRequestHandler {
	  public:
		struct QueryBlock {
			uint64_t StartDate = 0 , EndDate = 0 , Offset = 0 , Limit = 0, LogType = 0 ;
			std::string SerialNumber, Filter, Select;
			bool Lifetime=false, LastOnly=false, Newest=false;
		};

		typedef std::map<std::string, std::string> BindingMap;

		RESTAPIHandler(BindingMap map, Poco::Logger &l, std::vector<std::string> Methods, bool Internal=false)
			: Bindings_(std::move(map)), Logger_(l), Methods_(std::move(Methods)), Internal_(Internal) {}

		static bool ParseBindings(const std::string & Request, const std::list<const char *> & EndPoints, BindingMap &Keys);
		void PrintBindings();
		void ParseParameters(Poco::Net::HTTPServerRequest &request);

		void AddCORS(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &response);
	 	void SetCommonHeaders(Poco::Net::HTTPServerResponse &response, bool CloseConnection=false);
		void ProcessOptions(Poco::Net::HTTPServerRequest &Request,
							Poco::Net::HTTPServerResponse &response);
		void
		PrepareResponse(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &response,
						Poco::Net::HTTPResponse::HTTPStatus Status = Poco::Net::HTTPResponse::HTTP_OK,
						bool CloseConnection = false);
		bool ContinueProcessing(Poco::Net::HTTPServerRequest &Request,
								Poco::Net::HTTPServerResponse &Response);

		bool IsAuthorized(Poco::Net::HTTPServerRequest &Request,
						  Poco::Net::HTTPServerResponse &Response);
/*		bool ValidateAPIKey(Poco::Net::HTTPServerRequest &Request,
							Poco::Net::HTTPServerResponse &Response); */

		uint64_t GetParameter(const std::string &Name, uint64_t Default);
		std::string GetParameter(const std::string &Name, const std::string &Default);
		bool GetBoolParameter(const std::string &Name, bool Default);

		void BadRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response, const std::string &Reason = "");
		void UnAuthorized(Poco::Net::HTTPServerRequest &Request,
						  Poco::Net::HTTPServerResponse &Response, const std::string &Reason = "");
		void ReturnObject(Poco::Net::HTTPServerRequest &Request, Poco::JSON::Object &Object,
						  Poco::Net::HTTPServerResponse &Response);
		void NotFound(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
		void OK(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
		void ReturnStatus(Poco::Net::HTTPServerRequest &Request,
						  Poco::Net::HTTPServerResponse &Response,
						  Poco::Net::HTTPResponse::HTTPStatus Status,
						  bool CloseConnection=false);
		void SendFile(Poco::File & File, const std::string & UUID,
					  Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);
		void SendHTMLFileBack(Poco::File & File,
                              Poco::Net::HTTPServerRequest &Request,
                              Poco::Net::HTTPServerResponse &Response ,
                              const Types::StringPairVec & FormVars);
        void SendFile(Poco::TemporaryFile &TempAvatar, const std::string &Type, const std::string & Name, Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);

        void SendFile(Poco::File & File, Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response);

        const std::string &GetBinding(const std::string &Name, const std::string &Default);
		void InitQueryBlock();

		[[nodiscard]] static uint64_t Get(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, uint64_t Default=0);
		[[nodiscard]] static std::string GetS(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, const std::string & Default="");
		[[nodiscard]] static bool GetB(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, bool Default=false);
		[[nodiscard]] static uint64_t GetWhen(const Poco::JSON::Object::Ptr &Obj);

	  protected:
		BindingMap 					Bindings_;
		Poco::URI::QueryParameters 	Parameters_;
		Poco::Logger 				&Logger_;
		std::string 				SessionToken_;
		SecurityObjects::UserInfoAndPolicy 	UserInfo_;
		std::vector<std::string> 	Methods_;
		QueryBlock					QB_;
		bool                        Internal_=false;
	};

	class RESTAPI_UnknownRequestHandler : public RESTAPIHandler {
	  public:
		RESTAPI_UnknownRequestHandler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L)
			: RESTAPIHandler(bindings, L, std::vector<std::string>{}) {}
		void handleRequest(Poco::Net::HTTPServerRequest &Request,
						   Poco::Net::HTTPServerResponse &Response) override {
			if (!IsAuthorized(Request, Response))
				return;
			BadRequest(Request, Response);
		}
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
	RESTAPIHandler * RESTAPI_Router(const std::string & RequestedPath, RESTAPIHandler::BindingMap &Bindings, Poco::Logger & Logger ) {
		static_assert(test_has_PathName_method((T*)nullptr), "Class must have a static PathName() method.");
		if(RESTAPIHandler::ParseBindings(RequestedPath,T::PathName(),Bindings)) {
			return new T(Bindings, Logger, false);
		}

		if constexpr (sizeof...(Args) == 0) {
			return new RESTAPI_UnknownRequestHandler(Bindings,Logger);
		} else {
			return RESTAPI_Router<Args...>(RequestedPath, Bindings, Logger);
		}
	}

    template<typename T, typename... Args>
    RESTAPIHandler * RESTAPI_Router_I(const std::string & RequestedPath, RESTAPIHandler::BindingMap &Bindings, Poco::Logger & Logger) {
        static_assert(test_has_PathName_method((T*)nullptr), "Class must have a static PathName() method.");
        if(RESTAPIHandler::ParseBindings(RequestedPath,T::PathName(),Bindings)) {
            return new T(Bindings, Logger, true);
        }

        if constexpr (sizeof...(Args) == 0) {
            return new RESTAPI_UnknownRequestHandler(Bindings,Logger);
        } else {
            return RESTAPI_Router_I<Args...>(RequestedPath, Bindings, Logger);
        }
    }


}

#endif //UCENTRAL_RESTAPI_HANDLER_H
