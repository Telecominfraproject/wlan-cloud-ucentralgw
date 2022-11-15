//
// Created by stephane bourque on 2022-11-14.
//

#pragma once

#include <string>
#include <set>

#include "Poco/JSON/Object.h"
#include "Poco/Logger.h"
#include "fmt/format.h"

/*
{
	“country”: [
		“US”, “CA”
	],
	“dfs”: true,
	“ssh”: true,
	“rtty”: true,
	“tty”: true,
	“developer”: true,
	“sysupgrade”: true,
	“commands”: true
}
 */

namespace OpenWifi {
    class AP_Restrictions {
    public:
        inline bool initialize(Poco::Logger & Logger, const std::string & serialNumber, const Poco::JSON::Object::Ptr &O) {
            try {
                dfs_ = O->optValue("dfs",false);
                ssh_ = O->optValue("ssh",false);
                rtty_ = O->optValue("rtty",false);
                tty_ = O->optValue("tty",false);
                developer_ = O->optValue("developer",false);
                sysupgrade_ = O->optValue("sysupgrade",false);
                commands_ = O->optValue("commands",false);
                if(O->has("country") && O->isArray("country")) {
                    auto Countries = O->getArray("country");
                    for(const auto &country:*Countries) {
                        countries_.insert(Poco::toLower(country.toString()));
                    }
                }
                return true;
            } catch (...) {
                poco_error(Logger,fmt::format("Cannot parse restrictions for device {}", serialNumber));
            }
            return false;
        }

        [[nodiscard]] inline auto dfs_not_allowed() const { return dfs_; }
        [[nodiscard]] inline auto ssh_not_allowed() const { return ssh_; }
        [[nodiscard]] inline auto rtty_not_allowed() const { return rtty_; }
        [[nodiscard]] inline auto tty_not_allowed() const { return tty_; }
        [[nodiscard]] inline auto developer_not_allowed() const { return developer_; }
        [[nodiscard]] inline auto sysupgrade_not_allowed() const { return sysupgrade_; }
        [[nodiscard]] inline auto commands_not_allowed() const { return commands_; }
        [[nodiscard]] inline bool valid_country(const std::string &c) const {
            if(countries_.empty())
                return true;
            return countries_.find(Poco::toLower(c))!=countries_.end();
        }

    private:
        std::set<std::string>   countries_;
        bool    dfs_ = false;
        bool    ssh_ = false;
        bool    rtty_ = false;
        bool    tty_ = false;
        bool    developer_ = false;
        bool    sysupgrade_ = false;
        bool    commands_ = false;
    };
}