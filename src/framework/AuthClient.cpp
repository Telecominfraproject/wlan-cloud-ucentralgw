//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <utility>

#include "AuthClient.h"
#include "RESTAPI/RESTAPI_SecurityObjects.h"
#include "Daemon.h"
#include "OpenAPIRequest.h"

namespace OpenWifi {
	class AuthClient * AuthClient::instance_ = nullptr;

	int AuthClient::Start() {
		return 0;
	}

	void AuthClient::Stop() {

	}

	void AuthClient::RemovedCachedToken(const std::string &Token) {
		std::lock_guard	G(Mutex_);
		UserCache_.erase(Token);
	}

	bool IsTokenExpired(const SecurityObjects::WebToken &T) {
		return ((T.expires_in_+T.created_)<std::time(nullptr));
	}

	bool AuthClient::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo ) {
		std::lock_guard G(Mutex_);

		auto User = UserCache_.find(SessionToken);
		if(User != UserCache_.end() && !IsTokenExpired(User->second.webtoken)) {
			UInfo = User->second;
			return true;
		} else {
			Types::StringPairVec QueryData;
			QueryData.push_back(std::make_pair("token",SessionToken));
			OpenAPIRequestGet	Req(    uSERVICE_SECURITY,
								  	"/api/v1/validateToken",
									 QueryData,
								  5000);
			Poco::JSON::Object::Ptr Response;
			if(Req.Do(Response)==Poco::Net::HTTPResponse::HTTP_OK) {
				if(Response->has("tokenInfo") && Response->has("userInfo")) {
					SecurityObjects::UserInfoAndPolicy	P;
					P.from_json(Response);
					UserCache_[SessionToken] = P;
					UInfo = P;
				}
				return true;
			}

		}
		return false;
	}

	bool AuthClient::IsTokenAuthorized(const std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo) {
		std::lock_guard G(Mutex_);

		auto User = UserCache_.find(SessionToken);
		if(User != UserCache_.end() && !IsTokenExpired(User->second.webtoken)) {
			UInfo = User->second;
			return true;
		} else {
			Types::StringPairVec QueryData;
			QueryData.push_back(std::make_pair("token",SessionToken));
			OpenAPIRequestGet	Req(uSERVICE_SECURITY,
									 "/api/v1/validateToken",
									 QueryData,
									 5000);
			Poco::JSON::Object::Ptr Response;
			if(Req.Do(Response)==Poco::Net::HTTPResponse::HTTP_OK) {
				if(Response->has("tokenInfo") && Response->has("userInfo")) {
					SecurityObjects::UserInfoAndPolicy	P;
					P.from_json(Response);
					UserCache_[SessionToken] = P;
					UInfo = P;
				}
				return true;
			}

		}
		return false;
	}
}