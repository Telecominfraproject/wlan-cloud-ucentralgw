//
// Created by stephane bourque on 2021-02-15.
//

#include "uCentral.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/IntValidator.h"
#include <iostream>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::Util::IntValidator;
using Poco::AutoPtr;

#include "TIPGWServer.h"
#include "uCentralRESTAPIServer.h"
#include "uCentralWebSocketServer.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"
#include "uAuthService.h"
#include "uCentralConfig.h"

namespace uCentral {

    Daemon::Daemon() : helpRequested_(false) {
    }

    void Daemon::initialize(Application &self) {
        // logging_channel_.assign(new FileChannel);
        loadConfiguration(); // load default configuration files, if present

        addSubsystem(uCentral::Storage::Service::instance());
        addSubsystem(uCentral::Auth::Service::instance());
        addSubsystem(uCentral::DeviceRegistry::Service::instance());
        addSubsystem(uCentral::TIPGW::Service::instance());
        addSubsystem(uCentral::RESTAPI::Service::instance());
        addSubsystem(uCentral::WebSocket::Service::instance());

        ServerApplication::initialize(self);
        logger().information("Starting...");
        // add your own initialization code here
    }

    void Daemon::uninitialize() {
        // add your own uninitialization code here
        Application::uninitialize();
    }

    void Daemon::reinitialize(Application &self) {
        Application::reinitialize(self);
        // add your own reinitialization code here
    }

    void Daemon::defineOptions(OptionSet &options) {
        Application::defineOptions(options);

        options.addOption(
                Option("help", "h", "display help information on command line arguments")
                        .required(false)
                        .repeatable(false)
                        .callback(OptionCallback<Daemon>(this, &Daemon::handleHelp)));

        options.addOption(
                Option("file", "f", "specify the configuration file")
                        .required(false)
                        .repeatable(true)
                        .argument("file")
                        .callback(OptionCallback<Daemon>(this, &Daemon::handleConfig)));

        options.addOption(
                Option("debug", "d", "run in debug mode")
                        .required(false)
                        .repeatable(true)
                        .callback(OptionCallback<Daemon>(this, &Daemon::handleDebug)));

        options.addOption(
                Option("port", "p", "bind to port")
                        .required(false)
                        .repeatable(false)
                        .argument("value")
                        .validator(new IntValidator(0, 9999))
                        .callback(OptionCallback<Daemon>(this, &Daemon::handlePort)));
    }

    void Daemon::handleHelp(const std::string &name, const std::string &value) {
        helpRequested_ = true;
        displayHelp();
        stopOptionsProcessing();
    }

    void Daemon::handleDebug(const std::string &name, const std::string &value) {
        defineProperty(value);
    }

    void Daemon::handlePort(const std::string &name, const std::string &value) {
        defineProperty(value);
    }

    void Daemon::handleConfig(const std::string &name, const std::string &value) {
        std::cout << "Configuration file name: " << value << std::endl;
        loadConfiguration(value);
    }

    void Daemon::displayHelp() {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("A uCentral gateway implementation for TIP.");
        helpFormatter.format(std::cout);
    }

    void Daemon::defineProperty(const std::string &def) {
        std::string name;
        std::string value;
        std::string::size_type pos = def.find('=');
        if (pos != std::string::npos) {
            name.assign(def, 0, pos);
            value.assign(def, pos + 1, def.length() - pos);
        } else name = def;
        config().setString(name, value);
    }

    void createTestRecord() {
        uint64_t Now = time(nullptr);
        std::string SerialNumber{"24f5a207a130"};

        uCentral::Storage::Service::instance()->DeleteDevice(SerialNumber);

        uCentralDevice D{.SerialNumber = SerialNumber,
                .DeviceType = "AP",
                .MACAddress = "24:f5:a2:07:a1:33",
                .Manufacturer = "LinkSys",
                .UUID = Now,
                .Configuration = uCentral::Config::Config().get(),
                .Notes = "test device"};

        uCentral::Storage::Service::instance()->CreateDevice(D);
    }

    static bool path_match(const char *p, const char *r, std::map<std::string, std::string> &keys) {
        std::string param, value;

        while (*r) {
            if (*r == '{') {
                r++;
                while (*r != '}')
                    param += *r++;
                r++;
                while (*p != '/' && *p)
                    value += *p++;
                keys[param] = value;
                value.clear();
                param.clear();
            } else if (*p != *r) {
                return false;
            } else {
                r++;
                p++;
            }
        }

        return (*p == *r);
    }

    void show_kvs(std::map<std::string, std::string> &C) {
        for (auto &[key, value]:C)
            std::cout << "  " << key << ": " << value << std::endl;
    }

    void d(const char *p, const char *r) {
        std::map<std::string, std::string> KVs;

        if (path_match(p, r, KVs)) {
            std::cout << "Match: " << p << " >>> " << r << std::endl;
            show_kvs(KVs);
        } else
            std::cout << "NO Match: " << p << " >>> " << r << std::endl;
    }

    int Daemon::main(const ArgVec &args) {
        if (!helpRequested_) {
            Logger &logger = Logger::get("uCentral");

            std::cout << "Time: " << time(nullptr) << std::endl;

            uCentral::Storage::Service::instance()->Start();
            uCentral::Auth::Service::instance()->Start();
            uCentral::DeviceRegistry::Service::instance()->Start();
            uCentral::TIPGW::Service::instance()->Start();
            uCentral::RESTAPI::Service::instance()->Start();
            uCentral::WebSocket::Service::instance()->Start();

            createTestRecord();

            waitForTerminationRequest();

            uCentral::WebSocket::Service::instance()->Stop();
            uCentral::RESTAPI::Service::instance()->Stop();
            uCentral::TIPGW::Service::instance()->Stop();
            uCentral::DeviceRegistry::Service::instance()->Stop();
            uCentral::Auth::Service::instance()->Stop();
            uCentral::Storage::Service::instance()->Stop();
        }

        return Application::EXIT_OK;
    }

}; // end of namespace