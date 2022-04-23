//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "framework/MicroService.h"

namespace OpenWifi {
	class RESTAPI_device_handler : public RESTAPIHandler {
	  public:
		RESTAPI_device_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, uint64_t TransactionId, bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{
								 Poco::Net::HTTPRequest::HTTP_GET, Poco::Net::HTTPRequest::HTTP_POST,
								 Poco::Net::HTTPRequest::HTTP_PUT, Poco::Net::HTTPRequest::HTTP_DELETE,
								 Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server,
							 TransactionId,
							 Internal) {}
		static auto PathName() { return std::list<std::string>{"/api/v1/device/{serialNumber}"}; };
		void DoGet() final;
		void DoDelete() final;
		void DoPost() final;
		void DoPut() final;
	};
}
