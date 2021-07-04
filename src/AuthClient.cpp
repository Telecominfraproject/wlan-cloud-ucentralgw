//
// Created by stephane bourque on 2021-06-30.
//
#include <utility>

#include "AuthClient.h"
#include "RESTAPI_SecurityObjects.h"
#include "Daemon.h"
#include "OpenAPIRequest.h"

namespace uCentral {
	class AuthClient * AuthClient::instance_ = nullptr;

	int AuthClient::Start() {
		return 0;
	}

	void AuthClient::Stop() {

	}

	void AuthClient::RemovedCachedToken(const std::string &Token) {
		SubMutexGuard G(Mutex_);
		UserCache_.erase(Token);
	}

	bool IsTokenExpired(const SecurityObjects::WebToken &T) {
		return ((T.expires_in_+T.created_)<std::time(nullptr));
	}

	bool AuthClient::IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo ) {
		SubMutexGuard G(Mutex_);

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