//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "Poco/File.h"
#include "Poco/StreamCopier.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"

#include "framework/MicroServiceFuncs.h"

// #include "nlohmann/json.hpp"

namespace OpenWifi {

	class AppServiceRegistry {
	  public:
		AppServiceRegistry() {
			FileName = MicroServiceDataDirectory() + "/registry.json";
			Poco::File F(FileName);

			try {
				if (F.exists()) {
					std::ostringstream OS;
					std::ifstream IF(FileName);
                    Poco::JSON::Parser  P;
					Registry_ = P.parse(IF).extract<Poco::JSON::Object::Ptr>();
				}
			} catch (...) {
				Registry_ = Poco::makeShared<Poco::JSON::Object>();
			}
		}

		static AppServiceRegistry &instance() {
			static auto instance_ = new AppServiceRegistry;
			return *instance_;
		}

		inline ~AppServiceRegistry() { Save(); }

		inline void Save() {
			std::ofstream OF;
			OF.open(FileName, std::ios::binary | std::ios::trunc);
            Registry_->stringify(OF);
		}

        void Set(const char *key, const std::vector<std::string> &V) {
            Poco::JSON::Array   Arr;
            for(const auto &s:V) {
                Arr.add(s);
            }
            Registry_->set(key,Arr);
            Save();
        }

        template<class T> void Set(const char *key, const T &Value) {
            Registry_->set(key,Value);
			Save();
		}

        bool Get(const char *key, std::vector<std::string> &Value) {
            if(Registry_->has(key) && !Registry_->isNull(key) && Registry_->isArray(key)) {
                auto Arr = Registry_->get(key);
                for(const auto &v:Arr) {
                    Value.emplace_back(v);
                }
                return true;
            }
            return false;
        }

        template<class T> bool Get(const char *key, T &Value) {
            if(Registry_->has(key) && !Registry_->isNull(key)) {
                Value = Registry_->getValue<T>(key);
                return true;
            }
            return false;
        }

	  private:
		std::string FileName;
		Poco::JSON::Object::Ptr Registry_;
	};

	inline auto AppServiceRegistry() { return AppServiceRegistry::instance(); }

} // namespace OpenWifi