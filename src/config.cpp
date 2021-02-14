//
// Created by stephane bourque on 2021-02-12.
//
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

#include "config.h"

uConfig::Config *uConfig::Config::instance_ = 0;

bool uConfig::Config::init(const char *filename) {

    filename_ = filename;

    cfg_ = std::make_shared<YAML::Node>(YAML::LoadFile(filename_));

    return true;
}

bool uConfig::init(const char *filename) {
    return uConfig::Config::instance()->init(filename);
}
