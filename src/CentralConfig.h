//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <string>
#include "Poco/JSON/Object.h"

namespace OpenWifi::Config {

	class Config {
    public:
        explicit Config(const std::string &Config)
        	:Config_(Config) {
		}

        Config();
        bool SetUUID(uint64_t UUID);
        [[nodiscard]] bool Valid();
		[[nodiscard]] std::string get() { return Config_; };
		[[nodiscard]] std::string Default();
		[[nodiscard]] Poco::JSON::Object::Ptr to_json();

    private:
	  	void Init();
	  	void SetBasicConfigFile();
	  	inline static std::string DefaultConfiguration_ = "";
	  	std::string Config_;
    };

    class Capabilities {
    public:
        explicit Capabilities(const Poco::JSON::Object::Ptr &Caps);

/*        Capabilities()
        {
            Capabilities_ = Default();
        }

        static std::string Default();

        [[nodiscard]] const std::string & Get() const { return Capabilities_; };
*/

        [[nodiscard]] const std::string & Compatible() const;
        [[nodiscard]] const std::string & Model() const;
		[[nodiscard]] const std::string & Platform() const;
		[[nodiscard]] const std::string & AsString() const;

    private:
        std::string Compatible_;
        std::string Model_;
		std::string Platform_;
        std::string AsString_;

		void Parse();
    };

} // namespace

