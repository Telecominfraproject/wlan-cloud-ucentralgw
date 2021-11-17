//
// Created by stephane bourque on 2021-09-14.
//

#pragma once

#include <nlohmann/json-schema.hpp>
#include "framework/MicroService.h"

using nlohmann::json;
using nlohmann::json_schema::json_validator;

namespace OpenWifi {
    class ConfigurationValidator : public  SubSystemServer {
    public:

        static ConfigurationValidator *instance() {
            if(instance_== nullptr)
                instance_ = new ConfigurationValidator;
            return instance_;
        }

        bool Validate(const std::string &C, std::string &Error);
        static void my_format_checker(const std::string &format, const std::string &value);
        int Start() override;
        void Stop() override;
        void reinitialize(Poco::Util::Application &self) override;

    private:
        static  ConfigurationValidator * instance_;
        bool            Initialized_=false;
        bool            Working_=false;
        void            Init();
        std::unique_ptr<json_validator>  Validator_=std::make_unique<json_validator>(nullptr, my_format_checker);

        ConfigurationValidator():
            SubSystemServer("configvalidator", "CFG-VALIDATOR", "config.validator") {
        }
    };

    inline ConfigurationValidator * ConfigurationValidator() { return ConfigurationValidator::instance(); }
    inline bool ValidateUCentralConfiguration(const std::string &C, std::string &Error) { return ConfigurationValidator::instance()->Validate(C, Error); }
}

