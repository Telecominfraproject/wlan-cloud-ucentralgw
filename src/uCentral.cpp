//
// Created by stephane bourque on 2021-02-15.
//
#include <cstdlib>
#include <boost/algorithm/string.hpp>

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Environment.h"
#include "Poco/Path.h"

#include "uCentralRESTAPIServer.h"
#include "uCentralWebSocketServer.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"
#include "uAuthService.h"
#include "uCommandManager.h"
#include "base64util.h"
#include "uFileUploader.h"
#include "CommandChannel.h"
#include "utils.h"

#ifndef SMALL_BUILD
#include "kafka_service.h"
#endif

#include "uCentral.h"

namespace uCentral {

	Daemon App;

    Daemon * instance() { return &App; }

    void MyErrorHandler::exception(const Poco::Exception & E) {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		App.logger().log(E);
		App.logger().error(Poco::format("Exception occurred in %s",CurrentThread->getName()));
    }

    void MyErrorHandler::exception(const std::exception & E) {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		App.logger().warning(Poco::format("std::exception on %s",CurrentThread->getName()));
    }

    void MyErrorHandler::exception() {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		App.logger().warning(Poco::format("exception on %s",CurrentThread->getName()));
    }

    void Daemon::initialize(Application &self) {

		Poco::Net::initializeSSL();

        std::string Location = Poco::Environment::get("UCENTRAL_CONFIG",".");
        Poco::Path ConfigFile;

        ConfigFile = ConfigFileName_.empty() ? Location + "/ucentral.properties" : ConfigFileName_;

        if(!ConfigFile.isFile())
        {
            std::cerr << "uCentral: Configuration " << ConfigFile.toString() << " does not seem to exist. Please set UCENTRAL_CONFIG env variable the path of the ucentral.properties file." << std::endl;
            std::exit(Poco::Util::Application::EXIT_CONFIG);
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
		addSubsystem(uCentral::Kafka::Service::instance());
#endif
        addSubsystem(uCentral::RESTAPI::Service::instance());
        addSubsystem(uCentral::WebSocket::Service::instance());
        addSubsystem(uCentral::CommandManager::Service::instance());
        addSubsystem(uCentral::uFileUploader::Service::instance());
		addSubsystem(uCentral::CommandChannel::Service::instance());

        ServerApplication::initialize(self);

        logger().information("Starting...");

        if(!DebugMode_)
            DebugMode_ = config().getBool("ucentral.system.debug",false);

        ID_ = config().getInt64("ucentral.system.id",1);

        // add your own initialization code here
        AutoProvisioning_ = config().getBool("ucentral.autoprovisioning",false);

        // DeviceTypeIdentifications_
        std::vector<std::string>    Keys;
        config().keys("ucentral.autoprovisioning.type",Keys);
        for(const auto & i:Keys)
        {
            std::string Line = config().getString("ucentral.autoprovisioning.type."+i);
            auto P1 = Line.find_first_of(':');
            auto Type = Line.substr(0, P1);
            auto List = Line.substr(P1+1);

            std::vector<std::string>    Tokens = uCentral::Utils::Split(List);

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

		options.addOption(
			Poco::Util::Option("version", "", "get the version and quit.")
				.required(false)
				.repeatable(false)
				.callback(Poco::Util::OptionCallback<Daemon>(this, &Daemon::handleVersion)));

	}

	std::string Daemon::Version() {
		std::string V = APP_VERSION;
		std::string B = BUILD_NUMBER;
		return V + "(" + B +  ")";
	}

    void Daemon::handleHelp(const std::string &name, const std::string &value) {
        helpRequested_ = true;
        displayHelp();
        stopOptionsProcessing();
    }

	void Daemon::handleVersion(const std::string &name, const std::string &value) {
		helpRequested_ = true;
		std::cout << Version() << std::endl;
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

	bool Daemon::SetSubsystemLogLevel(const std::string &SubSystem, const std::string &Level) {
		try {
			auto P = Poco::Logger::parseLevel(Level);
			auto Sub = boost::algorithm::to_lower_copy(SubSystem);
			if (Sub == "ufileuploader")
				uCentral::uFileUploader::Service().Logger().setLevel(P);
			else if (Sub == "websocket")
				uCentral::WebSocket::Service().Logger().setLevel(P);
			else if (Sub == "storage")
				uCentral::Storage::Service().Logger().setLevel(P);
			else if (Sub == "restapi")
				uCentral::RESTAPI::Service().Logger().setLevel(P);
			else if (Sub == "commandmanager")
				uCentral::CommandManager::Service().Logger().setLevel(P);
			else if (Sub == "auth")
				uCentral::Auth::Service().Logger().setLevel(P);
			else if (Sub == "deviceregistry")
				uCentral::DeviceRegistry::Service().Logger().setLevel(P);
			else if (Sub == "all") {
				uCentral::Auth::Service().Logger().setLevel(P);
				uCentral::uFileUploader::Service().Logger().setLevel(P);
				uCentral::WebSocket::Service().Logger().setLevel(P);
				uCentral::Storage::Service().Logger().setLevel(P);
				uCentral::RESTAPI::Service().Logger().setLevel(P);
				uCentral::CommandManager::Service().Logger().setLevel(P);
				uCentral::DeviceRegistry::Service().Logger().setLevel(P);
			} else
				return false;
			return true;
		} catch (const Poco::Exception & E) {

		}
		return false;
	}

    int Daemon::main(const ArgVec &args) {

        Poco::ErrorHandler::set(&AppErrorHandler_);

        if (!helpRequested_) {

            Poco::Logger &logger = Poco::Logger::get("uCentral");

			logger.notice(Poco::format("Starting uCentral version %s.",Version()));

            uCentral::Storage::Start();
            uCentral::Auth::Start();
            uCentral::DeviceRegistry::Start();
            uCentral::RESTAPI::Start();
            uCentral::WebSocket::Start();
            uCentral::CommandManager::Start();
            uCentral::uFileUploader::Start();
			uCentral::CommandChannel::Start();

//			std::string f{"devices.db"};
//			uCentral::Storage::AttachFileToCommand(f);

#ifndef SMALL_BUILD
			uCentral::Kafka::Start();
#endif
			if(Poco::Net::Socket::supportsIPv6()) {
				logger.information("System supports IPv6.");
				std::cout << "System supports IPv6." << std::endl;
			}
			else
				logger.information("System does NOT suppord IPv6.");
            App.waitForTerminationRequest();

#ifndef SMALL_BUILD
			uCentral::Kafka::Stop();
#endif

			uCentral::CommandChannel::Stop();
            uCentral::uFileUploader::Stop();
            uCentral::CommandManager::Stop();
            uCentral::WebSocket::Stop();
            uCentral::RESTAPI::Stop();
            uCentral::DeviceRegistry::Stop();
            uCentral::Auth::Stop();
            uCentral::Storage::Stop();
			logger.notice("Stopped ucentral...");
        }

        return Application::EXIT_OK;
    }

    namespace ServiceConfig {

        uint64_t getInt(const std::string &Key,uint64_t Default) {
            return App.config().getInt64(Key,Default);
        }

        uint64_t getInt(const std::string &Key) {
            return App.config().getInt(Key);
        }

        uint64_t getBool(const std::string &Key,bool Default) {
            return App.config().getBool(Key,Default);
        }

        uint64_t getBool(const std::string &Key) {
            return App.config().getBool(Key);
        }

        std::string getString(const std::string &Key,const std::string & Default) {
            std::string R = App.config().getString(Key, Default);
            return Poco::Path::expand(R);
        }

        std::string getString(const std::string &Key) {
            std::string R = App.config().getString(Key);
            return Poco::Path::expand(R);
        }
    }
}

int main(int argc, char **argv) {
	try {
		return uCentral::App.run(argc, argv);
	} catch (Poco::Exception &exc) {
		std::cerr << exc.displayText() << std::endl;
		return Poco::Util::Application::EXIT_SOFTWARE;
	}
}

// end of namespace