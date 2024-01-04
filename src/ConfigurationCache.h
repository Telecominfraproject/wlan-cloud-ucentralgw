//
// Created by stephane bourque on 2021-09-21.
//

#pragma once

#include <map>
#include <mutex>
#include <string>

namespace OpenWifi {
	class ConfigurationCache {
	  public:
		static auto instance() {
			static auto instance = new ConfigurationCache;
			return instance;
		}

		inline uint64_t CurrentConfig(std::uint64_t SerialNumber) {
			std::lock_guard G(Mutex_);
			const auto Hint = Cache_.find(SerialNumber);
			if (Hint == end(Cache_))
				return 0;
			return Hint->second;
		}

		inline void Add(std::uint64_t SerialNumber, uint64_t Id) {
			std::lock_guard G(Mutex_);
			Cache_[SerialNumber] = Id;
		}

	  private:
		std::mutex Mutex_;
		std::map<uint64_t, uint64_t> Cache_;
	};

	inline uint64_t GetCurrentConfigurationID(uint64_t SerialNumber) {
		return ConfigurationCache::instance()->CurrentConfig(SerialNumber);
	}

	inline void SetCurrentConfigurationID(const std::string &SerialNumber, uint64_t ID) {
		return ConfigurationCache::instance()->Add(Utils::SerialNumberToInt(SerialNumber), ID);
	}

	inline void SetCurrentConfigurationID(uint64_t SerialNumber, uint64_t ID) {
		return ConfigurationCache::instance()->Add(SerialNumber, ID);
	}
} // namespace OpenWifi
