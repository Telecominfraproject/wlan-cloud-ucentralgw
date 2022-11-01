//
// Created by stephane bourque on 2022-10-31.
//

#pragma once

#include "framework/RESTAPI_Handler.h"
#include "framework/MicroServiceFuncs.h"

using namespace std::chrono_literals;

namespace OpenWifi {

	class RESTAPI_system_configuration : public RESTAPIHandler {
	  public:
		RESTAPI_system_configuration(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L,
									 RESTAPI_GenericServerAccounting &Server, uint64_t TransactionId,
									 bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_PUT,
													  Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_DELETE,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server, TransactionId, Internal) {}

		static auto PathName() { return std::list<std::string>{"/api/v1/systemConfiguration"}; }

		inline void DoPost() final {}

		inline void DoGet() final {

			return OK();
		}

		inline void DoPut() final{
			if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
				return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
			}

			return OK();
		};

		inline void DoDelete() final{
			if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
				return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
			}
			MicroServiceDeleteOverrideConfiguration();
			return OK();
		};
	};

}
