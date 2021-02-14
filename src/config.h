//
// Created by stephane bourque on 2021-02-12.
//

#ifndef UCENTRAL_CONFIG_H
#define UCENTRAL_CONFIG_H

#include <string>
#include <memory>

#include "yaml-cpp/yaml.h"

namespace uConfig {

    bool init(const char * filename);

    class Config {
    public:
        Config() {
        }

        bool init(const char *filename);
        auto get(const char *node_name, const char *sub_node) { return (*cfg_)[node_name][sub_node]; }
        template<typename T> T get(const char *node_name, const char *sub_node) { return (*cfg_)[node_name][sub_node].as<T>(); }

        static Config *instance() {
            if(!instance_)
                instance_ = new Config;
            return instance_;
        }

    private:
        static Config *instance_;
        std::string filename_;
        std::shared_ptr<YAML::Node> cfg_;
    };

    template<typename T> T get(const char *node_name, const char *sub_node)
    {
        return uConfig::Config::instance()->get<T>(node_name,sub_node);
    }

}

#endif //UCENTRAL_CONFIG_H
