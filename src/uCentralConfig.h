//
// Created by stephane bourque on 2021-03-06.
//

#ifndef UCENTRAL_UCENTRALCONFIG_H
#define UCENTRAL_UCENTRALCONFIG_H

#include <string>

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

    private:
        std::string Config_;
    };

}; // namespace

#endif //UCENTRAL_UCENTRALCONFIG_H
