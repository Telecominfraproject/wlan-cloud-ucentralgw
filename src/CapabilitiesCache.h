//
// Created by stephane bourque on 2021-11-17.
//

#pragma once

#include <string>
#include <map>
#include <mutex>
#include <fstream>

#include "framework/MicroServiceFuncs.h"

#include "nlohmann/json.hpp"

namespace OpenWifi {
	const std::string PlatformCacheFileName{"/plat_cache.json"};
	const std::string CapabilitiesCacheFileName{"/caps_cache.json"};

	typedef std::map<std::string,nlohmann::json>	CapabilitiesCache_t;

	class CapabilitiesCache {
	  public:

		static auto instance() {
			static auto instance = new CapabilitiesCache;
			return instance;
		}

		inline void Add(const std::string & DeviceType, const std::string & Platform, const std::string & FullCapabilities) {
			if(DeviceType.empty() || Platform.empty())
				return;

			std::lock_guard	G(Mutex_);
			if(!PlatformsLoaded_)
				LoadPlatforms();
			auto P = Poco::toUpper(Platform);
			auto Hint = Platforms_.find(DeviceType);
			if(Hint==Platforms_.end()) {
				Platforms_.insert(std::make_pair(DeviceType,P));
				SavePlatforms();
			} else if(Hint->second != P) {
				Hint->second = P;
				SavePlatforms();
			}

			if(!CapabilitiesLoaded_)
				LoadCapabilities();

			auto CapHint = Capabilities_.find(DeviceType);
			if(CapHint==Capabilities_.end()) {
				Capabilities_[DeviceType] = nlohmann::json::parse(FullCapabilities);
				SaveCapabilities();
			} else {
				CapHint->second = nlohmann::json::parse(FullCapabilities);
				SaveCapabilities();
			}
		}

		inline std::string GetPlatform(const std::string & DeviceType) {
			std::lock_guard	G(Mutex_);

			if(!PlatformsLoaded_) {
				LoadPlatforms();
			}

			auto Hint = Platforms_.find(DeviceType);
			if(Hint==Platforms_.end())
				return "AP";
			return Hint->second;
		}

		inline nlohmann::json GetCapabilities(const std::string & DeviceType) {
			std::lock_guard	G(Mutex_);

			if(!CapabilitiesLoaded_) {
				LoadCapabilities();
			}

			auto Hint = Capabilities_.find(DeviceType);
			if(Hint==Capabilities_.end())
				return nlohmann::json{};
			return Hint->second;
		}

		inline const CapabilitiesCache_t & AllCapabilities() {
			std::lock_guard	G(Mutex_);
			if(!CapabilitiesLoaded_) {
				LoadCapabilities();
			}
			return Capabilities_;
		}

	  private:
		std::recursive_mutex					Mutex_;
		std::atomic_bool 						PlatformsLoaded_=false;
		std::atomic_bool 						CapabilitiesLoaded_=false;
		std::map<std::string,std::string>		Platforms_;
		CapabilitiesCache_t						Capabilities_;
		std::string 							PlatformCacheFileName_{ MicroServiceDataDirectory()+PlatformCacheFileName };
		std::string 							CapabilitiesCacheFileName_{ MicroServiceDataDirectory()+CapabilitiesCacheFileName };

		inline void LoadPlatforms() {
			try {
				std::ifstream i(PlatformCacheFileName_);
				nlohmann::json cache;
				i >> cache;

				for(const auto &[Type,Platform]:cache.items()) {
					Platforms_[Type] = Platform;
				}
			} catch(...) {

			}
			PlatformsLoaded_ = true;
		}

		inline void SavePlatforms() {
			try {
				std::ofstream i(PlatformCacheFileName_);
				nlohmann::json cache(Platforms_);
				i << cache;
			} catch (...) {

			}
		}

		inline void LoadCapabilities() {
			try {
				std::ifstream i(CapabilitiesCacheFileName_, std::ios_base::binary|std::ios_base::in);
				nlohmann::json cache;
				i >> cache;

				for(const auto &[Type,Caps]:cache.items()) {
					Capabilities_[Type] = Caps;
				}
			} catch(...) {

			}
			CapabilitiesLoaded_ = true;
		}

		inline void SaveCapabilities() {
			try {
				std::ofstream i(CapabilitiesCacheFileName_, std::ios_base::trunc | std::ios_base::out | std::ios_base::binary );
				nlohmann::json cache(Capabilities_);
				i << cache;
			} catch (...) {

			}
		}
	};
}