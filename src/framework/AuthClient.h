//
// Created by stephane bourque on 2021-06-30.
//

#ifndef UCENTRALGW_AUTHCLIENT_H
#define UCENTRALGW_AUTHCLIENT_H

#include "Poco/JSON/Object.h"
#include "Poco/JWT/Signer.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/SHA2Engine.h"
#include "RESTAPI/RESTAPI_SecurityObjects.h"
#include "SubSystemServer.h"

namespace OpenWifi {

class AuthClient : public SubSystemServer {
	  public:
		explicit AuthClient() noexcept:
			SubSystemServer("Authentication", "AUTH-CLNT", "authentication")
		{
		}

		static AuthClient *instance() {
			if (instance_ == nullptr) {
				instance_ = new AuthClient;
			}
			return instance_;
		}

		int Start() override;
		void Stop() override;
		bool IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string &SessionToken, OpenWifi::SecurityObjects::UserInfoAndPolicy & UInfo );
		void RemovedCachedToken(const std::string &Token);
		bool IsTokenAuthorized(const std::string &Token, SecurityObjects::UserInfoAndPolicy & UInfo);
	  private:
		static AuthClient 					*instance_;
		OpenWifi::SecurityObjects::UserInfoCache 		UserCache_;
	};

	inline AuthClient * AuthClient() { return AuthClient::instance(); }
}

#endif // UCENTRALGW_AUTHCLIENT_H
