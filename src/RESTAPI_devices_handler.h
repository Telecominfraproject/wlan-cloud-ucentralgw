//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_RESTAPI_DEVICESHANDLER_H
#define UCENTRAL_RESTAPI_DEVICESHANDLER_H

#include "RESTAPI_handler.h"

namespace OpenWifi {
	class RESTAPI_devices_handler : public RESTAPIHandler {
	  public:
		RESTAPI_devices_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server,
							 Internal){};
		static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/devices"}; };
		void DoGet() final;
		void DoDelete() final {};
		void DoPost() final {};
		void DoPut() final {};
	};
}

#endif //UCENTRAL_RESTAPI_DEVICESHANDLER_H
