//
// Created by stephane bourque on 2021-02-12.
//
#include <iostream>
#include <fstream>

#include "uCentral.h"
#include "PropertiesFileServerList.h"

void PropertiesFileServerList::initialize() {

    auto i=0;
    bool good=true;

    while(good) {
        std::string root{prefix_ + ".host." + std::to_string(i) + "."};

        std::string address{root + "address"};
        if(uCentral::Daemon::instance().config().getString(address,"") == "") {
            good = false;
        }
        else {
            std::string port{root + "port"};
            std::string key{root + "key"};
            std::string key_password{root + "key.password"};
            std::string cert{root + "cert"};

            PropertiesFileServerEntry entry(   uCentral::Daemon::instance().config().getString(address,""),
                                        uCentral::Daemon::instance().config().getInt(port,0),
                                        uCentral::Daemon::instance().config().getString(key,""),
                                        uCentral::Daemon::instance().config().getString(cert,""),
                                        uCentral::Daemon::instance().config().getString(key_password,""));
            list_.push_back(entry);
            i++;
        }
    }
    // uCentral::instance().config().
}

const PropertiesFileServerEntry & PropertiesFileServerList::operator[](int index)
{
    return list_[index];
}

