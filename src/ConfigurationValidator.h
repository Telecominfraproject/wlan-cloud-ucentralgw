//
// Created by stephane bourque on 2021-09-14.
//

#ifndef OWPROV_CONFIGURATIONVALIDATOR_H
#define OWPROV_CONFIGURATIONVALIDATOR_H

#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

namespace OpenWifi {
    class ConfigurationValidator {
    public:
        ConfigurationValidator() {
            Validator_ = std::make_unique<json_validator>(nullptr, my_format_checker);
        }

        static ConfigurationValidator *instance() {
            if(instance_== nullptr)
                instance_ = new ConfigurationValidator;
            return instance_;
        }

        bool Validate(const std::string &C);
        static void my_format_checker(const std::string &format, const std::string &value)
        {
            /*
                "format": "uc-mac"
                "format": "uc-timeout",
                "format": "uc-cidr4",
                "format": "uc-cidr6",
                "uc-format": "cidr",
                "format": "fqdn",
                "format": "uc-host",
                "format": "uri"
                "format": "hostname"
                "format": "uc-base64"


                if (format == "something") {
                    return;
                    if (!check_value_for_something(value))
                                throw std::invalid_argument("value is not a good something");
                        } else
                        throw std::logic_error("Don't know how to validate " + format);
            */
        }

    private:
        static  ConfigurationValidator * instance_;
        bool    Initialized_=false;
        bool    Working_=false;
        void    Init();
        std::unique_ptr<json_validator>  Validator_;
    };

    inline bool ValidateUCentralConfiguration(const std::string &C) { return ConfigurationValidator::instance()->Validate(C); }
}

#endif //OWPROV_CONFIGURATIONVALIDATOR_H
