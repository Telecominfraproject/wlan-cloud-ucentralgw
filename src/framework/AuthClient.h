//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "framework/SubSystemServer.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "Poco/ExpireLRUCache.h"
#include "framework/utils.h"

namespace OpenWifi {

	class AuthClient : public SubSystemServer {

	  public:
		explicit AuthClient() noexcept:
			 SubSystemServer("Authentication", "AUTH-CLNT", "authentication")
		{
		}

		static auto instance() {
			static auto instance_ = new AuthClient;
			return instance_;
		}

        struct ApiKeyCacheEntry {
            OpenWifi::SecurityObjects::UserInfoAndPolicy    UserInfo;
            std::uint64_t                                   ExpiresOn;
        };

        inline int Start() override {
			return 0;
		}

		inline void Stop() override {
			poco_information(Logger(),"Stopping...");
			std::lock_guard	G(Mutex_);
			Cache_.clear();
			poco_information(Logger(),"Stopped...");
		}

		inline void RemovedCachedToken(const std::string &Token) {
			Cache_.remove(Token);
            ApiKeyCache_.remove(Token);
		}

		inline static bool IsTokenExpired(const SecurityObjects::WebToken &T) {
			return ((T.expires_in_+T.created_) < Utils::Now());
		}

		bool RetrieveTokenInformation(const std::string & SessionToken,
			SecurityObjects::UserInfoAndPolicy & UInfo,
			std::uint64_t TID,
		bool & Expired, bool & Contacted, bool Sub=false);

        bool RetrieveApiKeyInformation(const std::string & SessionToken,
                                      SecurityObjects::UserInfoAndPolicy & UInfo,
                                      std::uint64_t TID,
                                      bool & Expired, bool & Contacted);

		bool IsAuthorized(const std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo,
								 std::uint64_t TID,
								 bool & Expired, bool & Contacted, bool Sub = false);

        bool IsValidApiKey(const std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo,
                          std::uint64_t TID,
                          bool & Expired, bool & Contacted);

	  private:

		Poco::ExpireLRUCache<std::string,OpenWifi::SecurityObjects::UserInfoAndPolicy>      Cache_{512,1200000 };
        Poco::ExpireLRUCache<std::string,ApiKeyCacheEntry>                                  ApiKeyCache_{512,1200000 };
	};

	inline auto AuthClient() { return AuthClient::instance(); }

} // namespace OpenWifi

