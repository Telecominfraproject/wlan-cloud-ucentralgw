//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "framework/SubSystemServer.h"

#include "Poco/URI.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/ExpireLRUCache.h"

#include "fmt/format.h"

namespace OpenWifi {

	class RESTAPI_RateLimiter : public SubSystemServer {
	  public:

		struct ClientCacheEntry {
			int64_t  Start=0;
			int      Count=0;
		};

		static auto instance() {
			static auto instance_ = new RESTAPI_RateLimiter;
			return instance_;
		}

		inline int Start() final { return 0;};
		inline void Stop() final { };

		inline bool IsRateLimited(const Poco::Net::HTTPServerRequest &R, int64_t Period, int64_t MaxCalls) {
			Poco::URI   uri(R.getURI());
			auto H = str_hash(uri.getPath() + R.clientAddress().host().toString());
			auto E = Cache_.get(H);
			auto Now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			if(E.isNull()) {
				Cache_.add(H,ClientCacheEntry{.Start=Now, .Count=1});
				return false;
			}
			if((Now-E->Start)<Period) {
				E->Count++;
				Cache_.update(H,E);
				if(E->Count > MaxCalls) {
					poco_warning(Logger(),fmt::format("RATE-LIMIT-EXCEEDED: from '{}'", R.clientAddress().toString()));
					return true;
				}
				return false;
			}
			E->Start = Now;
			E->Count = 1;
			Cache_.update(H,E);
			return false;
		}

		inline void Clear() {
			Cache_.clear();
		}

	  private:
		Poco::ExpireLRUCache<uint64_t,ClientCacheEntry>      Cache_{2048};
		std::hash<std::string>          str_hash;

		RESTAPI_RateLimiter() noexcept:
										 SubSystemServer("RateLimiter", "RATE-LIMITER", "rate.limiter")
		{
		}

	};

	inline auto RESTAPI_RateLimiter() { return RESTAPI_RateLimiter::instance(); }


}