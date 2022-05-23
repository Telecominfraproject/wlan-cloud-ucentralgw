//
// Created by stephane bourque on 2022-05-20.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
	class RESTAPI_radiusProxyConfig_handler : public RESTAPIHandler {
	  public:
		RESTAPI_radiusProxyConfig_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L,
										  RESTAPI_GenericServer &Server, uint64_t TransactionId,
										  bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_DELETE,
													  Poco::Net::HTTPRequest::HTTP_PUT,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server, TransactionId, Internal) {}
		static auto PathName() { return std::list<std::string>{"/api/v1/radiusProxyConfig"}; }
		void DoGet() final;
		void DoDelete() final;
		void DoPost() final{};
		void DoPut() final;
	};
}
