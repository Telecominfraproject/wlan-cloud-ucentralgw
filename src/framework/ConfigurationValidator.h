//
// Created by stephane bourque on 2021-09-14.
//

#pragma once

#include "framework/SubSystemServer.h"

#include <valijson/adapters/poco_json_adapter.hpp>
#include <valijson/utils/poco_json_utils.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>
#include <valijson/constraints/constraint.hpp>
#include <valijson/constraints/constraint_visitor.hpp>

namespace OpenWifi {
    class ConfigurationValidator : public  SubSystemServer {
    public:

        static auto instance() {
            static auto instance_ = new ConfigurationValidator;
            return instance_;
        }

        bool Validate(const std::string &C, std::vector<std::string> &Errors, bool Strict);
        int Start() override;
        void Stop() override;
        void reinitialize(Poco::Util::Application &self) override;

    private:
        bool            					Initialized_=false;
        bool            					Working_=false;
        void            					Init();
		std::unique_ptr<valijson::Schema>						RootSchema_;
		std::unique_ptr<valijson::SchemaParser>  				SchemaParser_;
		std::unique_ptr<valijson::adapters::PocoJsonAdapter> 	PocoJsonAdapter_;
		Poco::JSON::Object::Ptr 			SchemaDocPtr_;
		bool 								SetSchema(const std::string &SchemaStr);

        ConfigurationValidator():
            SubSystemServer("ConfigValidator", "CFG-VALIDATOR", "config.validator") {
        }
    };

    inline auto ConfigurationValidator() { return ConfigurationValidator::instance(); }
    inline bool ValidateUCentralConfiguration(const std::string &C, std::vector<std::string> &Error, bool strict) { return ConfigurationValidator::instance()->Validate(C, Error, strict); }
}

