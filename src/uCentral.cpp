//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
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

#include "RESTAPI_server.h"
#include "uAuthService.h"
#include "uCallbackManager.h"
#include "uCentralWebSocketServer.h"
#include "uCommandChannel.h"
#include "uCommandManager.h"
#include "uDeviceRegistry.h"
#include "uFileUploader.h"
#include "uStorageService.h"
#include "uFirmwareManager.h"

#include "uUtils.h"

#ifndef SMALL_BUILD
#include "kafka_service.h"
#endif

#include "uCentral.h"

namespace uCentral {
	Daemon *Daemon::instance_ = nullptr;

    Daemon * instance() { return uCentral::Daemon::instance(); }

    void MyErrorHandler::exception(const Poco::Exception & E) {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		instance()->logger().log(E);
		instance()->logger().error(Poco::format("Exception occurred in %s",CurrentThread->getName()));
    }

    void MyErrorHandler::exception(const std::exception & E) {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		instance()->logger().warning(Poco::format("std::exception on %s",CurrentThread->getName()));
    }

    void MyErrorHandler::exception() {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		instance()->logger().warning(Poco::format("exception on %s",CurrentThread->getName()));
    }

	void Daemon::Exit(int Reason) {
		std::exit(Reason);
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

		Poco::Path	DataDir(config().getString("system.directory.data"));
		try {
			DataDir.makeDirectory();
			DataDir_ = DataDir.toString();
		} catch(...) {
		}

		std::string KeyFile = Poco::Path::expand(config().getString("ucentral.service.key"));

		AppKey_ = Poco::SharedPtr<Poco::Crypto::RSAKey>(new Poco::Crypto::RSAKey("", KeyFile, ""));

        addSubsystem(uCentral::Storage::Service::instance());
        addSubsystem(uCentral::Auth::Service::instance());
        addSubsystem(uCentral::DeviceRegistry::Service::instance());
		addSubsystem(uCentral::Kafka::Service::instance());
        addSubsystem(uCentral::RESTAPI::Service::instance());
        addSubsystem(uCentral::WebSocket::Service::instance());
        addSubsystem(uCentral::CommandManager::Service::instance());
        addSubsystem(uCentral::uFileUploader::Service::instance());
		addSubsystem(uCentral::CommandChannel::Service::instance());
		addSubsystem(uCentral::CallbackManager::Service::instance());
		addSubsystem(uCentral::FirmwareManager::Service::instance());

        ServerApplication::initialize(self);

        logger().information("Starting...");

        if(!DebugMode_)
            DebugMode_ = config().getBool("ucentral.system.debug",false);

        ID_ = config().getInt64("ucentral.system.id",1);

        // add your own initialization code here
        AutoProvisioning_ = config().getBool("ucentral.autoprovisioning",false);

        // DeviceTypeIdentifications_
        std::vector<std::string>    Keys;
        config().keys("ucentral.devicetypes",Keys);
        for(const auto & i:Keys)
        {
            std::string Line = config().getString("ucentral.devicetypes."+i);
            auto P1 = Line.find_first_of(':');
            auto Type = Line.substr(0, P1);
            auto List = Line.substr(P1+1);

            std::vector<std::string>    Tokens = uCentral::Utils::Split(List);

            auto Entry = DeviceTypeIdentifications_.find(Type);
			if(DeviceTypeIdentifications_.end() == Entry) {
				std::set<std::string>	S;
				S.insert(Tokens.begin(),Tokens.end());
				DeviceTypeIdentifications_[Type] = S;
			} else {
				Entry->second.insert(Tokens.begin(),Tokens.end());
			}
        }

		for(auto &[Type,List]:DeviceTypeIdentifications_) {
			std::cout << "Type: " << Type << std::endl;
			for(auto &i:List)
				std::cout << "    " << i << std::endl;
		}
    }

    [[nodiscard]] std::string Daemon::IdentifyDevice(const std::string & Id ) const {
        for(const auto &[Type,List]:DeviceTypeIdentifications_)
        {
			if(List.find(Id)!=List.end())
				return Type;
        }
        return "AP";
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
        HelpRequested_ = true;
        displayHelp();
        stopOptionsProcessing();
    }

	void Daemon::handleVersion(const std::string &name, const std::string &value) {
		HelpRequested_ = true;
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

        if (!HelpRequested_) {
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
			uCentral::CallbackManager::Start();
			uCentral::FirmwareManager::Start();
			uCentral::Kafka::Start();

			if(Poco::Net::Socket::supportsIPv6())
				logger.information("System supports IPv6.");
			else
				logger.information("System does NOT support IPv6.");

			std::string T{"2021-05-18T21:31:39.000Z"};

			uint64_t TT = uCentral::Utils::from_RFC3339(T);
			//std::cout << "Time:" << TT << " now " << time(nullptr) << std::endl;

			instance()->waitForTerminationRequest();

			uCentral::Kafka::Stop();
			uCentral::FirmwareManager::Stop();
			uCentral::CallbackManager::Stop();
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

        uint64_t GetInt(const std::string &Key,uint64_t Default) {
            return (uint64_t) instance()->config().getInt64(Key,Default);
        }

        uint64_t GetInt(const std::string &Key) {
            return instance()->config().getInt(Key);
        }

        uint64_t GetBool(const std::string &Key,bool Default) {
            return instance()->config().getBool(Key,Default);
        }

        uint64_t GetBool(const std::string &Key) {
            return instance()->config().getBool(Key);
        }

        std::string GetString(const std::string &Key,const std::string & Default) {
            std::string R = instance()->config().getString(Key, Default);
            return Poco::Path::expand(R);
        }

        std::string GetString(const std::string &Key) {
            std::string R = instance()->config().getString(Key);
            return Poco::Path::expand(R);
        }
    }
}

int main(int argc, char **argv) {
	try {

		auto App = uCentral::Daemon::instance();
		auto ExitCode =  App->run(argc, argv);
		delete App;

		return ExitCode;

	} catch (Poco::Exception &exc) {
		std::cerr << exc.displayText() << std::endl;
		return Poco::Util::Application::EXIT_SOFTWARE;
	}
}

// end of namespace