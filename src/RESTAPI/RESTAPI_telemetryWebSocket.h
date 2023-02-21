//
// Created by stephane bourque on 2021-09-16.
//

#pragma once

#include "framework/RESTAPI_Handler.h"

namespace OpenWifi {
	class RESTAPI_telemetryWebSocket : public RESTAPIHandler {
	  public:
		RESTAPI_telemetryWebSocket(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L,
								   RESTAPI_GenericServerAccounting &Server, uint64_t TransactionId,
								   bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server, TransactionId, Internal, false) {}
		static auto PathName() { return std::list<std::string>{"/api/v1/ws_telemetry"}; }
		void DoGet() final;
		void DoDelete() final{};
		void DoPost() final{};
		void DoPut() final{};

	  private:
		void Process(const Poco::JSON::Object::Ptr &O, std::string &Answer);
	};
} // namespace OpenWifi
