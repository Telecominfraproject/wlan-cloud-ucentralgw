//
// Created by stephane bourque on 2021-11-17.
//

#pragma once

#include "framework/MicroService.h"
#include "nlohmann/json.hpp"

namespace OpenWifi {
	static const std::string CapabilitiesCacheFileName{"/capabilities_cache.json"};

	class CapabilitiesCache {
	  public:

		static CapabilitiesCache * instance() {
			static CapabilitiesCache * instance = new CapabilitiesCache;
			return instance;
		}

		inline void Add(const std::string & DeviceType, const std::string & Platform) {
			std::lock_guard	G(Mutex_);
			if(!Loaded_)
				LoadIt();
			auto P = Poco::toUpper(Platform);
			auto Hint = Caps_.find(DeviceType);
			if(Hint==Caps_.end()) {
				Caps_.insert(std::make_pair(DeviceType,P));
				SaveIt();
			}
			if(Hint->second != P) {
				Hint->second = P;
				SaveIt();
			}
		}

		inline std::string Get(const std::string & DeviceType) {
			std::lock_guard	G(Mutex_);

			if(!Loaded_) {
				LoadIt();
			}

			auto Hint = Caps_.find(DeviceType);
			if(Hint==Caps_.end())
				return "AP";
			return Hint->second;
		}

	  private:
		std::mutex			Mutex_;
		std::atomic_bool 	Loaded_=false;
		std::map<std::string,std::string>	Caps_;

		inline void LoadIt() {
			try {
				std::ifstream i(MicroService::instance().DataDir()+CapabilitiesCacheFileName);
				nlohmann::json cache;
				i >> cache;

				for(const auto &[Type,Platform]:cache.items()) {
					Caps_[Type] = Platform;
				}
			} catch(...) {

			}
			Loaded_ = true;
		}

		inline void SaveIt() {
			try {
				std::ofstream i(MicroService::instance().DataDir() + CapabilitiesCacheFileName);
				nlohmann::json cache(Caps_);
				i << cache;
			} catch (...) {

			}
		}
	};
}