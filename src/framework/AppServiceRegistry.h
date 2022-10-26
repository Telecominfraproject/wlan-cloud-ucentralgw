//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "Poco/StreamCopier.h"
#include "Poco/File.h"

#include "framework/MicroServiceFuncs.h"

#include "nlohmann/json.hpp"

namespace OpenWifi {


	class AppServiceRegistry {
	  public:
		AppServiceRegistry() {
			FileName = MicroServiceDataDirectory() + "/registry.json";
			Poco::File F(FileName);

			try {
				if(F.exists()) {
					std::ostringstream  OS;
					std::ifstream       IF(FileName);
					Poco::StreamCopier::copyStream(IF, OS);
					Registry_ = nlohmann::json::parse(OS.str());
				}
			} catch (...) {
				Registry_ = nlohmann::json::parse("{}");
			}
		}

		static AppServiceRegistry & instance() {
			static auto instance_= new AppServiceRegistry;
			return *instance_;
		}

		inline ~AppServiceRegistry() {
			Save();
		}

		inline void Save() {
			std::istringstream  IS( to_string(Registry_));
			std::ofstream       OF;
			OF.open(FileName,std::ios::binary | std::ios::trunc);
			Poco::StreamCopier::copyStream(IS, OF);
		}

		inline void Set(const char *Key, uint64_t Value ) {
			Registry_[Key] = Value;
			Save();
		}

		inline void Set(const char *Key, const std::string &Value ) {
			Registry_[Key] = Value;
			Save();
		}

		inline void Set(const char *Key, bool Value ) {
			Registry_[Key] = Value;
			Save();
		}

		inline bool Get(const char *Key, bool & Value ) {
			if(Registry_[Key].is_boolean()) {
				Value = Registry_[Key].get<bool>();
				return true;
			}
			return false;
		}

		inline bool Get(const char *Key, uint64_t & Value ) {
			if(Registry_[Key].is_number_unsigned()) {
				Value = Registry_[Key].get<uint64_t>();
				return true;
			}
			return false;
		}

		inline bool Get(const char *Key, std::string & Value ) {
			if(Registry_[Key].is_string()) {
				Value = Registry_[Key].get<std::string>();
				return true;
			}
			return false;
		}

	  private:
		std::string         FileName;
		nlohmann::json      Registry_;
	};

	inline auto AppServiceRegistry() { return AppServiceRegistry::instance(); }

}