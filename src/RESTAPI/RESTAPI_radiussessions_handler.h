//
// Created by stephane bourque on 2023-04-02.
//

#pragma once

#include "framework/RESTAPI_Handler.h"

namespace OpenWifi {

	class RESTAPI_radiussessions_handler : public RESTAPIHandler {
	  public:
		RESTAPI_radiussessions_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L,
						   RESTAPI_GenericServerAccounting &Server, uint64_t TransactionId,
						   bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server, TransactionId, Internal){};
		static auto PathName() { return std::list<std::string>{"/api/v1/radiusSessions/{serialNumber}"}; };
		void DoGet() final;
		void DoDelete() final{};
		void DoPost() final{};
		void DoPut() final{};
	};

} // namespace OpenWifi
