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

		inline uint64_t GetCurrentConfig(std::uint64_t SerialNumber) {
			std::lock_guard G(Mutex_);
			const auto Hint = Cache_.find(SerialNumber);
			if (Hint == end(Cache_))
				return 0;
			return Hint->second;
		}

		inline void SetCurrentConfig(std::uint64_t SerialNumber, uint64_t Id) {
			std::lock_guard G(Mutex_);
			Cache_[SerialNumber] = Id;
		}

	  private:
		std::mutex Mutex_;
		std::map<uint64_t, uint64_t> Cache_;
	};

	inline auto GetCurrentConfigurationID(std::uint64_t SerialNumber) {
		return ConfigurationCache::instance()->GetCurrentConfig(SerialNumber);
	}

	inline void SetCurrentConfigurationID(const std::string &SerialNumber, std::uint64_t ID) {
		return ConfigurationCache::instance()->SetCurrentConfig(Utils::SerialNumberToInt(SerialNumber), ID);
	}

	inline void SetCurrentConfigurationID(uint64_t SerialNumber, std::uint64_t ID) {
		return ConfigurationCache::instance()->SetCurrentConfig(SerialNumber, ID);
	}
} // namespace OpenWifi
