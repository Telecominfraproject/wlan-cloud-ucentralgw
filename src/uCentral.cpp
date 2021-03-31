//
// Created by stephane bourque on 2021-02-15.
//
#include <iostream>

#include "uCentral.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Environment.h"
#include "Poco/Path.h"

#ifndef SMALL_BUILD
#include "TIPGWServer.h"
#endif

#include "uCentralRESTAPIServer.h"
#include "uCentralWebSocketServer.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"
#include "uAuthService.h"
#include "uCommandManager.h"
#include "base64util.h"
#include "uFileUploader.h"
#include "uCentralConfig.h"

namespace uCentral {

    Daemon * instance() { return reinterpret_cast<Daemon *>(&uCentral::Daemon::instance()); }

    void MyErrorHandler::exception(const Poco::Exception & E) {
        Poco::Thread * CurrentThread = Poco::Thread::current();
        uCentral::instance()->logger().warning(Poco::format("generic Poco exception on %s",CurrentThread->getName()));
    }

    void MyErrorHandler::exception(const std::exception & E) {
        Poco::Thread * CurrentThread = Poco::Thread::current();
        uCentral::instance()->logger().warning(Poco::format("std::exception on %s",CurrentThread->getName()));
    }

    void MyErrorHandler::exception() {
        Poco::Thread * CurrentThread = Poco::Thread::current();
        uCentral::instance()->logger().warning(Poco::format("exception on %s",CurrentThread->getName()));
    }

    void Daemon::initialize(Application &self) {

        std::string Location = Poco::Environment::get("UCENTRAL_CONFIG",".");
        Poco::Path ConfigFile;

        ConfigFile = ConfigFileName_.empty() ? Location + "/ucentral.properties" : ConfigFileName_;

        if(!ConfigFile.isFile())
        {
            std::cerr << "uCentral: Configuration " << ConfigFile.toString() << " does not seem to exist. Please set UCENTRAL_CONFIG env variable the path of the ucentral.properties file." << std::endl;
            std::exit(EXIT_CONFIG);
        }

        static const char * LogFilePathKey = "logging.channels.c2.path";

        loadConfiguration(ConfigFile.toString());

        if(LogDir_.empty()) {
            std::string OriginalLogFileValue = config().getString(LogFilePathKey);
            std::string RealLogFileValue = Poco::Path::expand(OriginalLogFileValue);
            config().setString(LogFilePathKey, RealLogFileValue);
        } else {
            config().setString(LogFilePathKey, LogDir_);
        }

        addSubsystem(uCentral::Storage::Service::instance());
        addSubsystem(uCentral::Auth::Service::instance());
        addSubsystem(uCentral::DeviceRegistry::Service::instance());
#ifndef SMALL_BUILD
        addSubsystem(uCentral::TIPGW::Service::instance());
#endif
        addSubsystem(uCentral::RESTAPI::Service::instance());
        addSubsystem(uCentral::WebSocket::Service::instance());
        addSubsystem(uCentral::CommandManager::Service::instance());
        addSubsystem(uCentral::uFileUploader::Service::instance());

        ServerApplication::initialize(self);

        logger().information("Starting...");

        if(!DebugMode_)
            DebugMode_ = config().getBool("ucentral.system.debug",false);

        ID_ = config().getInt64("ucentral.system.id",1);

        // add your own initialization code here
        AutoProvisioning_ = config().getBool("ucentral.autoprovisioning",false);

        // DeviceTypeIdentifications_
        std::vector<std::string>    Keys;
        uCentral::instance()->config().keys("ucentral.autoprovisioning.type",Keys);
        for(const auto & i:Keys)
        {
            std::string Line = config().getString("ucentral.autoprovisioning.type."+i);
            auto P1 = Line.find_first_of(':');
            auto Type = Line.substr(0, P1);
            auto List = Line.substr(P1+1);
            std::vector<std::string>    Tokens;

            auto P=0;

            while(P<List.size())
            {
                auto P2 = List.find_first_of(',', P);
                if(P2==std::string::npos) {
                    Tokens.push_back(List.substr(P));
                    break;
                }
                else
                    Tokens.push_back(List.substr(P,P2));
                P=P2+1;
            }
            auto Entry = DeviceTypeIdentifications_[Type];
            Entry.insert(Entry.end(),Tokens.begin(),Tokens.end());
        }
    }

    std::string Daemon::IdentifyDevice(const std::string & Id ) const {
        for(const auto &[Type,List]:DeviceTypeIdentifications_)
        {
            for(const auto & i : List)
            {
                if(Id.find(i)!=std::string::npos)
                    return Type;
            }
        }
        return "AP_Default";
    }

    void Daemon::uninitialize() {
        // add your own uninitialization code here
        ServerApplication::uninitialize();
    }

    void Daemon::reinitialize(Poco::Util::Application &self) {
        ServerApplication::reinitialize(self);
        // add your own reinitialization code here
    }

    void Daemon::defineOptions(Poco::Util::OptionSet &options) {
        ServerApplication::defineOptions(options);

        options.addOption(
                Poco::Util::Option("help", "", "display help information on command line arguments")
                        .required(false)
                        .repeatable(false)
                        .callback(Poco::Util::OptionCallback<Daemon>(this, &Daemon::handleHelp)));

        options.addOption(
                Poco::Util::Option("file", "", "specify the configuration file")
                        .required(false)
                        .repeatable(false)
                        .argument("file")
                        .callback(Poco::Util::OptionCallback<Daemon>(this, &Daemon::handleConfig)));

        options.addOption(
                Poco::Util::Option("debug", "", "to run in debug, set to true")
                        .required(false)
                        .repeatable(false)
                        .callback(Poco::Util::OptionCallback<Daemon>(this, &Daemon::handleDebug)));

        options.addOption(
                Poco::Util::Option("logs", "", "specify the log directory and file (i.e. dir/file.log)")
                        .required(false)
                        .repeatable(false)
                        .argument("dir")
                        .callback(Poco::Util::OptionCallback<Daemon>(this, &Daemon::handleLogs)));

    }

    void Daemon::handleHelp(const std::string &name, const std::string &value) {
        helpRequested_ = true;
        displayHelp();
        stopOptionsProcessing();
    }

    void Daemon::handleDebug(const std::string &name, const std::string &value) {
        if(value == "true")
            DebugMode_ = true ;
    }

    void Daemon::handleLogs(const std::string &name, const std::string &value) {
        LogDir_ = value;
    }

    void Daemon::handleConfig(const std::string &name, const std::string &value) {
        ConfigFileName_ = value;
    }

    void Daemon::displayHelp() {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("A uCentral gateway implementation for TIP.");
        helpFormatter.format(std::cout);
    }

    std::string Daemon::CreateUUID() {
        return UUIDGenerator_.create().toString();
    }

    int Daemon::main(const ArgVec &args) {

        Poco::ErrorHandler::set(&AppErrorHandler_);

        if (!helpRequested_) {

            std::cout << "Starting ucentral..." << std::endl;

            Poco::Logger &logger = Poco::Logger::get("uCentral");

            uCentral::Storage::Start();
            uCentral::Auth::Start();
            uCentral::DeviceRegistry::Start();
            uCentral::RESTAPI::Start();
            uCentral::WebSocket::Start();
            uCentral::CommandManager::Start();
            uCentral::uFileUploader::Start();


#ifndef SMALL_BUILD
            uCentral::TIPGW::Start();
#endif
            waitForTerminationRequest();

#ifndef SMALL_BUILD
            uCentral::TIPGW::Stop();
#endif

            uCentral::uFileUploader::Stop();
            uCentral::CommandManager::Stop();
            uCentral::WebSocket::Stop();
            uCentral::RESTAPI::Stop();
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

        std::string getString(const std::string &Key,const std::string & Default) {
            std::string R = uCentral::Daemon::instance().config().getString(Key, Default);

            return Poco::Path::expand(R);
        }

        std::string getString(const std::string &Key) {
            std::string R = uCentral::Daemon::instance().config().getString(Key);

            return Poco::Path::expand(R);
        }
    }

}; // end of namespace