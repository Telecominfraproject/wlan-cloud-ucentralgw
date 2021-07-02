//
// Created by stephane bourque on 2021-06-30.
//

#ifndef UCENTRALGW_AUTHCLIENT_H
#define UCENTRALGW_AUTHCLIENT_H

#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/JWT/Signer.h"
#include "Poco/SHA2Engine.h"
#include "RESTAPI_SecurityObjects.h"
#include "SubSystemServer.h"

namespace uCentral {

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
		bool IsAuthorized(Poco::Net::HTTPServerRequest & Request, std::string &SessionToken, SecurityObjects::WebToken & UserInfo );

	  private:
		static AuthClient 						*instance_;
		std::map<std::string,SecurityObjects::UserInfoAndPolicy>	UserCache_;
	};

	inline AuthClient * AuthClient() { return AuthClient::instance(); }
}

#endif // UCENTRALGW_AUTHCLIENT_H
