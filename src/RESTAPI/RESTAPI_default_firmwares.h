//
// Created by stephane bourque on 2023-07-11.
//

#pragma once

#include "framework/RESTAPI_Handler.h"

namespace OpenWifi {
	class RESTAPI_default_firmwares : public RESTAPIHandler {
	  public:
		RESTAPI_default_firmwares(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L,
									   RESTAPI_GenericServerAccounting &Server,
									   uint64_t TransactionId, bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server, TransactionId, Internal){};
		static auto PathName() { return std::list<std::string>{"/api/v1/default_firmwares"}; }
		void DoGet() final;
		void DoDelete() final{};
		void DoPost() final{};
		void DoPut() final{};
	};
} // namespace OpenWifi