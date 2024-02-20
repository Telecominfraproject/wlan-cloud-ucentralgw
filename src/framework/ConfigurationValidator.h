//
// Created by stephane bourque on 2021-09-14.
//

#pragma once

#include "framework/SubSystemServer.h"

#include <valijson/adapters/poco_json_adapter.hpp>
#include <valijson/constraints/constraint.hpp>
#include <valijson/constraints/constraint_visitor.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/utils/poco_json_utils.hpp>
#include <valijson/validator.hpp>

namespace OpenWifi {
	class ConfigurationValidator : public SubSystemServer {
	  public:

		enum class ConfigurationType { AP = 0 , SWITCH = 1};

		static auto instance() {
			static auto instance_ = new ConfigurationValidator;
			return instance_;
		}

		bool Validate(ConfigurationType Type, const std::string &C, std::vector<std::string> &Errors, bool Strict);
		int Start() override;
		void Stop() override;
		void reinitialize(Poco::Util::Application &self) override;

		inline static ConfigurationType GetType(const std::string &type) {
			std::string Type = Poco::toUpper(type);
			if (Type == "AP")
				return ConfigurationType::AP;
			if (Type == "SWITCH")
				return ConfigurationType::SWITCH;
			return ConfigurationType::AP;
		}

	  private:
		bool Initialized_ = false;
		bool Working_ = false;
		void Init();
		std::array<std::unique_ptr<valijson::Schema>,2> 			RootSchema_;
		std::array<std::unique_ptr<valijson::SchemaParser>,2> 	SchemaParser_;
		std::array<std::unique_ptr<valijson::adapters::PocoJsonAdapter>,2> PocoJsonAdapter_;
		std::array<Poco::JSON::Object::Ptr,2> 					SchemaDocPtr_;
		bool SetSchema(ConfigurationType Type, const std::string &SchemaStr);

		ConfigurationValidator()
			: SubSystemServer("ConfigValidator", "CFG-VALIDATOR", "config.validator") {}
	};

	inline auto ConfigurationValidator() { return ConfigurationValidator::instance(); }
	inline bool ValidateUCentralConfiguration(ConfigurationValidator::ConfigurationType Type, const std::string &C, std::vector<std::string> &Error,
											  bool strict) {
		return ConfigurationValidator::instance()->Validate(Type, C, Error, strict);
	}
} // namespace OpenWifi
