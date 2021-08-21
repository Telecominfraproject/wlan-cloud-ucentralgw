//
// Created by stephane bourque on 2021-07-01.
//

#ifndef UCENTRALGW_OPENAPIREQUEST_H
#define UCENTRALGW_OPENAPIREQUEST_H

#include "Poco/JSON/Object.h"

#include "OpenWifiTypes.h"

namespace OpenWifi {

	class OpenAPIRequestGet {
	  public:
		explicit OpenAPIRequestGet( const std::string & Type,
								   	const std::string & EndPoint,
									Types::StringPairVec & QueryData,
									uint64_t msTimeout);
		int Do(Poco::JSON::Object::Ptr &ResponseObject);
	  private:
		std::string 			Type_;
		std::string 			EndPoint_;
		Types::StringPairVec 	QueryData_;
		uint64_t 				msTimeout_;
	};
}

#endif // UCENTRALGW_OPENAPIREQUEST_H
