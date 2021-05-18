//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UCENTRALCONFIG_H
#define UCENTRAL_UCENTRALCONFIG_H

#include <string>
#include "Poco/JSON/Object.h"

namespace uCentral::Config {
    class Config {
    public:
        explicit Config(std::string c)
        :Config_(std::move(c)) {}

        Config()
        {
            Config_ = Default();
        }
        bool SetUUID(uint64_t UUID);
        bool Valid();
        std::string get() { return Config_; };
        static std::string Default();
		static std::string Default_Old();
		void to_json(Poco::JSON::Object &Obj);

    private:
        std::string Config_;
    };

    class Capabilities {
    public:
        explicit Capabilities(std::string Caps)
        :   Capabilities_(std::move(Caps))
        {

        }

        Capabilities()
        {
            Capabilities_ = Default();
        }

        static std::string Default();
        [[nodiscard]] const std::string & Get() const { return Capabilities_; };
        [[nodiscard]] const std::string & Manufacturer();
        [[nodiscard]] const std::string & DeviceType();
        [[nodiscard]] const std::string & ModelId();

    private:
        std::string Capabilities_;
        bool        Parsed_=false;
        std::string Manufacturer_;
        std::string DeviceType_;
        std::string ModelId_;

		void Parse();
    };

} // namespace

#endif //UCENTRAL_UCENTRALCONFIG_H
