//
// Created by stephane bourque on 2021-02-15.
//

#include "uCentral.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/IntValidator.h"
#include "Poco/Environment.h"
#include <iostream>
#include "Poco/Path.h"

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

        std::string Location = Poco::Environment::get("UCENTRAL_CONFIG",".");
        Poco::Path ConfigFile = Location + "/ucentral.properties";

        if(!ConfigFile.isFile())
        {
            std::cout << "uCentral: Configuration " << ConfigFile.toString() << " does not seem to exist. Please set UCENTRAL_CONFIG env variable the path of the ucentral.properties file." << std::endl;
            std::exit(EXIT_CONFIG);
        }

        char LogFilePathKey[] = "logging.channels.c2.path";

        loadConfiguration(ConfigFile.toString());
        std::string OriginalLogFileValue = config().getString(LogFilePathKey);
        std::string RealLogFileValue = uCentral::ServiceConfig::ReplaceEnvVar(OriginalLogFileValue);
        config().setString(LogFilePathKey,RealLogFileValue);

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

        uCentral::Storage::DeleteDevice(SerialNumber);

        uCentralDevice D{.SerialNumber = SerialNumber,
                .DeviceType = "AP",
                .MACAddress = "24:f5:a2:07:a1:33",
                .Manufacturer = "LinkSys",
                .UUID = Now,
                .Configuration = uCentral::Config::Config().get(),
                .Notes = "test device"};

        uCentral::Storage::CreateDevice(D);
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

            uCentral::Storage::Start();
            uCentral::Auth::Start();
            uCentral::DeviceRegistry::Start();
            uCentral::TIPGW::Start();
            uCentral::RESTAPI::Start();
            uCentral::WebSocket::Start();

            createTestRecord();

            waitForTerminationRequest();

            uCentral::WebSocket::Stop();
            uCentral::RESTAPI::Stop();
            uCentral::TIPGW::Stop();
            uCentral::DeviceRegistry::Stop();
            uCentral::Auth::Stop();
            uCentral::Storage::Stop();
        }

        return Application::EXIT_OK;
    }

    namespace ServiceConfig {

        uint64_t getInt(const std::string &Key,uint64_t Default) {
            return uCentral::Daemon::instance().config().getInt(Key,Default);
        }

        uint64_t getInt(const std::string &Key) {
            return uCentral::Daemon::instance().config().getInt(Key);
        }

        uint64_t getBool(const std::string &Key,bool Default) {
            return uCentral::Daemon::instance().config().getBool(Key,Default);
        }

        uint64_t getBool(const std::string &Key) {
            return uCentral::Daemon::instance().config().getBool(Key);
        }

        // for strings, we must replace environment variables
        //  if path is /path/to/${SOME_VAR}/afile
        //  should expand to something replacing ${SOME_VAR} with its value
        std::string ReplaceEnvVar(const std::string &Key) {

            std::string Result;
            std::string Var;
            bool InVar = false, GotDS = false;

            try {
                for (auto i:Key) {
                    if (i == '$' && GotDS)
                        Result += '$';
                    else if (i == '$')
                        GotDS = true;
                    else if (i == '{' and GotDS)
                        InVar = true;
                    else if (i == '}' and InVar) {
                        // evaluate var and add it to result
                        Result += Poco::Environment::get(Var);
                        Var.clear();
                        GotDS = InVar = false;
                    } else if (InVar)
                        Var += i;
                    else
                        Result += i;
                }

                return Result;
            }
            catch (const Poco::NotFoundException & E) {
                uCentral::Daemon::instance().logger().error( E.displayText() );
                std::exit(EXIT_FAILURE);
            }
        }

        std::string getString(const std::string &Key,const std::string & Default) {
            std::string R = uCentral::Daemon::instance().config().getString(Key, Default);

            return ReplaceEnvVar(R);
        }

        std::string getString(const std::string &Key) {
            std::string R = uCentral::Daemon::instance().config().getString(Key);

            return ReplaceEnvVar(R);
        }
    }

}; // end of namespace