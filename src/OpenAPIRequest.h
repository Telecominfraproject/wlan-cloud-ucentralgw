//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_OPENAPIREQUEST_H
#define UCENTRALGW_OPENAPIREQUEST_H

#include "Poco/JSON/Object.h"

#include "OpenWifiTypes.h"

namespace OpenWifi {

	class OpenAPIRequestGet {
	  public:
		explicit OpenAPIRequestGet( std::string Type,
								   	std::string EndPoint,
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
