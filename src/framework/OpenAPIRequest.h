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
									uint64_t msTimeout):
            Type_(std::move(Type)),
            EndPoint_(std::move(EndPoint)),
            QueryData_(QueryData),
            msTimeout_(msTimeout) {};
		int Do(Poco::JSON::Object::Ptr &ResponseObject);
	  private:
		std::string 			Type_;
		std::string 			EndPoint_;
		Types::StringPairVec 	QueryData_;
		uint64_t 				msTimeout_;
	};

	class OpenAPIRequestPut {
        public:
            explicit OpenAPIRequestPut( std::string Type,
                                        std::string EndPoint,
                                        Types::StringPairVec & QueryData,
                                        Poco::JSON::Object Body,
                                        uint64_t msTimeout):
            Type_(std::move(Type)),
            EndPoint_(std::move(EndPoint)),
            QueryData_(QueryData),
            msTimeout_(msTimeout),
            Body_(std::move(Body)){};
        int Do(Poco::JSON::Object::Ptr &ResponseObject);
        private:
            std::string 			Type_;
            std::string 			EndPoint_;
            Types::StringPairVec 	QueryData_;
            uint64_t 				msTimeout_;
            Poco::JSON::Object      Body_;
	};

	class OpenAPIRequestPost {
	public:
	    explicit OpenAPIRequestPost( std::string Type,
                                    std::string EndPoint,
                                    Types::StringPairVec & QueryData,
                                    Poco::JSON::Object Body,
                                    uint64_t msTimeout):
            Type_(std::move(Type)),
            EndPoint_(std::move(EndPoint)),
            QueryData_(QueryData),
            msTimeout_(msTimeout),
            Body_(std::move(Body)){};
	    int Do(Poco::JSON::Object::Ptr &ResponseObject);
	private:
	    std::string 			Type_;
	    std::string 			EndPoint_;
	    Types::StringPairVec 	QueryData_;
	    uint64_t 				msTimeout_;
	    Poco::JSON::Object      Body_;
	};
}

#endif // UCENTRALGW_OPENAPIREQUEST_H
