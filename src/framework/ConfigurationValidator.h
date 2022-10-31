//
// Created by stephane bourque on 2021-09-14.
//

#pragma once

#include <nlohmann/json-schema.hpp>
#include "framework/SubSystemServer.h"

using nlohmann::json;
using nlohmann::json_schema::json_validator;

namespace OpenWifi {
    class ConfigurationValidator : public  SubSystemServer {
    public:

        static auto instance() {
            static auto instance_ = new ConfigurationValidator;
            return instance_;
        }

        bool Validate(const std::string &C, std::string &Error);
        static void my_format_checker(const std::string &format, const std::string &value);
        int Start() override;
        void Stop() override;
        void reinitialize(Poco::Util::Application &self) override;

    private:
        bool            Initialized_=false;
        bool            Working_=false;
        void            Init();
        nlohmann::json  RootSchema_;

        ConfigurationValidator():
            SubSystemServer("ConfigValidator", "CFG-VALIDATOR", "config.validator") {
        }
    };

    inline auto ConfigurationValidator() { return ConfigurationValidator::instance(); }
    inline bool ValidateUCentralConfiguration(const std::string &C, std::string &Error) { return ConfigurationValidator::instance()->Validate(C, Error); }
}

