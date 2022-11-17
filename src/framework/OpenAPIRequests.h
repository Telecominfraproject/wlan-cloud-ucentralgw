//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerResponse.h"

#include "framework/OpenWifiTypes.h"

namespace OpenWifi {

	class OpenAPIRequestGet {
	  public:
		explicit OpenAPIRequestGet( const std::string & Type,
								   const std::string & EndPoint,
								   const Types::StringPairVec & QueryData,
								   uint64_t msTimeout,
                                   const std::string &LoggingStr=""):
														 Type_(Type),
														 EndPoint_(EndPoint),
														 QueryData_(QueryData),
														 msTimeout_(msTimeout),
                                                         LoggingStr_(LoggingStr){};
		Poco::Net::HTTPServerResponse::HTTPStatus Do(Poco::JSON::Object::Ptr &ResponseObject, const std::string & BearerToken = "");
	  private:
		std::string 			Type_;
		std::string 			EndPoint_;
		Types::StringPairVec 	QueryData_;
		uint64_t 				msTimeout_;
        std::string             LoggingStr_;
	};

	class OpenAPIRequestPut {
	  public:
		explicit OpenAPIRequestPut( const std::string & Type,
								   const std::string & EndPoint,
								   const Types::StringPairVec & QueryData,
								   const Poco::JSON::Object & Body,
								   uint64_t msTimeout,
                                   const std::string &LoggingStr=""):
														 Type_(Type),
														 EndPoint_(EndPoint),
														 QueryData_(QueryData),
														 msTimeout_(msTimeout),
														 Body_(Body),
                                                         LoggingStr_(LoggingStr){};

		Poco::Net::HTTPServerResponse::HTTPStatus Do(Poco::JSON::Object::Ptr &ResponseObject, const std::string & BearerToken = "");

	  private:
		std::string 			Type_;
		std::string 			EndPoint_;
		Types::StringPairVec 	QueryData_;
		uint64_t 				msTimeout_;
		Poco::JSON::Object      Body_;
        std::string             LoggingStr_;
	};

	class OpenAPIRequestPost {
	  public:
		explicit OpenAPIRequestPost( const std::string & Type,
									const std::string & EndPoint,
									const Types::StringPairVec & QueryData,
									const Poco::JSON::Object & Body,
									uint64_t msTimeout,
                                   const std::string &LoggingStr=""):
														  Type_(Type),
														  EndPoint_(EndPoint),
														  QueryData_(QueryData),
														  msTimeout_(msTimeout),
														  Body_(Body),
                                                         LoggingStr_(LoggingStr){};
		Poco::Net::HTTPServerResponse::HTTPStatus Do(Poco::JSON::Object::Ptr &ResponseObject, const std::string & BearerToken = "");
	  private:
		std::string 			Type_;
		std::string 			EndPoint_;
		Types::StringPairVec 	QueryData_;
		uint64_t 				msTimeout_;
		Poco::JSON::Object      Body_;
        std::string             LoggingStr_;
	};

	class OpenAPIRequestDelete {
	  public:
		explicit OpenAPIRequestDelete(  const std::string & Type,
									    const std::string & EndPoint,
									    const Types::StringPairVec & QueryData,
									    uint64_t msTimeout,
                                        const std::string &LoggingStr=""):
                                            Type_(Type),
                                            EndPoint_(EndPoint),
                                            QueryData_(QueryData),
                                            msTimeout_(msTimeout),
                                            LoggingStr_(LoggingStr){};
		Poco::Net::HTTPServerResponse::HTTPStatus Do(const std::string & BearerToken = "");

	  private:
		std::string 			Type_;
		std::string 			EndPoint_;
		Types::StringPairVec 	QueryData_;
		uint64_t 				msTimeout_;
		Poco::JSON::Object      Body_;
        std::string             LoggingStr_;
	};

} // namespace OpenWifi
