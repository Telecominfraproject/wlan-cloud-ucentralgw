//
// Created by stephane bourque on 2022-05-20.
//

#include "RESTAPI_radiusProxyConfig_handler.h"
#include "RESTObjects/RESTAPI_GWobjects.h"
#include "RADIUS_proxy_server.h"

namespace OpenWifi {

	void RESTAPI_radiusProxyConfig_handler::DoGet() {
		GWObjects::RadiusProxyPoolList	C;
		RADIUS_proxy_server()->GetConfig(C);
		Poco::JSON::Object	Answer;
		C.to_json(Answer);
		return ReturnObject(Answer);
	}

	void RESTAPI_radiusProxyConfig_handler::DoDelete() {
		if(!Internal_ && (UserInfo_.userinfo.userRole!=SecurityObjects::ROOT && UserInfo_.userinfo.userRole!=SecurityObjects::ADMIN)) {
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}
		RADIUS_proxy_server()->DeleteConfig();
		return OK();
	}

	void RESTAPI_radiusProxyConfig_handler::DoPut() {

		if(!Internal_ && (UserInfo_.userinfo.userRole!=SecurityObjects::ROOT && UserInfo_.userinfo.userRole!=SecurityObjects::ADMIN)) {
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		GWObjects::RadiusProxyPoolList	C;
		if(!C.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		//	Logically validate the config.
		for(const auto &pool:C.pools) {
			if(pool.name.empty()) {
				return BadRequest(RESTAPI::Errors::PoolNameInvalid);
			}
			for(const auto &config:{pool.acctConfig,pool.authConfig}) {
				if(config.strategy!="random" && config.strategy!="round_robin" && config.strategy!="weighted") {
					return BadRequest(RESTAPI::Errors::InvalidRadiusProxyStrategy);
				}
				if(config.monitorMethod!="none" && config.monitorMethod!="https" && config.monitorMethod!="radius") {
					return BadRequest(RESTAPI::Errors::InvalidRadiusProxyMonitorMethod);
				}
				if(config.servers.empty()) {
					return BadRequest(RESTAPI::Errors::MustHaveAtLeastOneRadiusServer);
				}

				for(auto &server:config.servers) {
					Poco::Net::IPAddress	Addr;
					if(!Poco::Net::IPAddress::tryParse(server.ip,Addr) || server.port==0) {
						return BadRequest(RESTAPI::Errors::InvalidRadiusServerEntry);
					}
					if(config.strategy=="weighted" && server.weight==0) {
						return BadRequest(RESTAPI::Errors::InvalidRadiusServerWeigth);
					}
				}
			}
		}

		RADIUS_proxy_server()->SetConfig(C);
		return ReturnObject(*ParsedBody_);
	}

}