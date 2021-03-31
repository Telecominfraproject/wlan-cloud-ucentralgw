//
// Created by stephane bourque on 2021-03-06.
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
        Poco::JSON::Object::Ptr   to_json();

    private:
        std::string Config_;
    };

    class Capabilities {
    public:
        explicit Capabilities(std::string Caps)
        :   Capabilities_(std::move(Caps)),
            Parsed_(false)
        {

        }

        Capabilities():
            Parsed_(false)
        {
            Capabilities_ = Default();
        }

        static std::string Default();
        [[nodiscard]] const std::string & Get() const { return Capabilities_; };
        [[nodiscard]] const std::string & Manufacturer();
        [[nodiscard]] const std::string & DeviceType();
        [[nodiscard]] const std::string & ModelId();

    private:
        void Parse();
        std::string Capabilities_;
        bool        Parsed_;
        std::string Manufacturer_;
        std::string DeviceType_;
        std::string ModelId_;
    };

}; // namespace

#endif //UCENTRAL_UCENTRALCONFIG_H
