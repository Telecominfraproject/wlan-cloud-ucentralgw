//
// Created by stephane bourque on 2021-09-21.
//

#ifndef OWGW_CONFIGURATIONCACHE_H
#define OWGW_CONFIGURATIONCACHE_H

#include <map>
#include <string>
#include <mutex>

namespace OpenWifi {
	class ConfigurationCache {
	  public:

		static ConfigurationCache &instance() {
			if(instance_== nullptr)
				instance_ = new ConfigurationCache;
			return *instance_;
		}

		inline uint64_t CurrentConfig(const std::string &SerialNumber) {
			std::lock_guard G(Mutex_);
			const auto Hint = Cache_.find(SerialNumber);
			if(Hint==end(Cache_))
				return 0;
			return Hint->second;
		}

		void Add(const std::string &SerialNumber, uint64_t Id) {
			std::lock_guard	G(Mutex_);
			Cache_[SerialNumber]=Id;
		}

	  private:
		static ConfigurationCache		*instance_;
		std::mutex						Mutex_;
		std::map<std::string,uint64_t>	Cache_;
	};

	inline uint64_t GetCurrentConfigurationID(const std::string &S) { return ConfigurationCache::instance().CurrentConfig(S); }
	inline void SetCurrentConfigurationID(const std::string &S, uint64_t ID) { ConfigurationCache::instance().Add(S,ID); }
}
#endif // OWGW_CONFIGURATIONCACHE_H
