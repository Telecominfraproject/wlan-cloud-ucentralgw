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

#include "AuthService.h"
#include "CallbackManager.h"
#include "CommandChannel.h"
#include "CommandManager.h"
#include "DeviceRegistry.h"
#include "FileUploader.h"
#include "FirmwareManager.h"
#include "RESTAPI_server.h"
#include "StorageService.h"
#include "WebSocketServer.h"

#include "StateProcessor.h"

#include "Utils.h"

#ifndef SMALL_BUILD
#include "kafka_service.h"
#endif

#include "ALBHealthCheckServer.h"
#include "Daemon.h"

namespace uCentral {
	class Daemon *Daemon::instance_ = nullptr;

    void MyErrorHandler::exception(const Poco::Exception & E) {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		Daemon()->logger().log(E);
		Daemon()->logger().error(Poco::format("Exception occurred in %s",CurrentThread->getName()));
    }

    void MyErrorHandler::exception(const std::exception & E) {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		Daemon()->logger().warning(Poco::format("std::exception on %s",CurrentThread->getName()));
    }

    void MyErrorHandler::exception() {
        Poco::Thread * CurrentThread = Poco::Thread::current();
		Daemon()->logger().warning(Poco::format("exception on %s",CurrentThread->getName()));
    }

	void Daemon::Exit(int Reason) {
		std::exit(Reason);
	}

	void Daemon::initialize(Application &self) {

		Poco::Net::initializeSSL();

		SubSystems_ = std::make_unique<std::list<SubSystemServer*>>(
			std::list<SubSystemServer*>{
				Storage(),
				AuthService(),
				DeviceRegistry(),
				RESTAPI_server(),
				WebSocketServer(),
				CommandManager(),
				FileUploader(),
				CommandChannel(),
				CallbackManager(),
				FirmwareManager(),
				KafkaManager(),
				ALBHealthCheckServer()
			}
		);

		std::string Location = Poco::Environment::get(DAEMON_CONFIG_ENV_VAR,".");
        Poco::Path ConfigFile;

        ConfigFile = ConfigFileName_.empty() ? Location + "/" + std::string(DAEMON_PROPERTIES_FILENAME) : ConfigFileName_;

        if(!ConfigFile.isFile())
        {
            std::cerr << DAEMON_APP_NAME << ": Configuration "
					  << ConfigFile.toString() << " does not seem to exist. Please set " + std::string(DAEMON_CONFIG_ENV_VAR)
					  + " env variable the path of the " + std::string(DAEMON_PROPERTIES_FILENAME) + " file." << std::endl;
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

		InitializeSubSystemServers();
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
		helpFormatter.setHeader("A " + std::string(uCentral::DAEMON_APP_NAME) + " implementation for TIP.");
        helpFormatter.format(std::cout);
    }

	void Daemon::InitializeSubSystemServers() {
		for(auto i:*SubSystems_)
			addSubsystem(i);
	}

	void Daemon::StartSubSystemServers() {
		for(auto i:*SubSystems_)
			i->Start();
	}

	void Daemon::StopSubSystemServers() {
		for(auto i=(*SubSystems_).rbegin(); i!=(*SubSystems_).rend(); ++i)
			(*i)->Stop();
	}

	std::string Daemon::CreateUUID() {
        return UUIDGenerator_.create().toString();
    }

	bool Daemon::SetSubsystemLogLevel(const std::string &SubSystem, const std::string &Level) {
		try {
			auto P = Poco::Logger::parseLevel(Level);
			auto Sub = boost::algorithm::to_lower_copy(SubSystem);
			if (Sub == "ufileuploader")
				FileUploader()->Logger().setLevel(P);
			else if (Sub == "websocket")
				WebSocketServer()->Logger().setLevel(P);
			else if (Sub == "storage")
				Storage()->Logger().setLevel(P);
			else if (Sub == "restapi")
				RESTAPI_server()->Logger().setLevel(P);
			else if (Sub == "commandmanager")
				CommandManager()->Logger().setLevel(P);
			else if (Sub == "auth")
				AuthService()->Logger().setLevel(P);
			else if (Sub == "deviceregistry")
				DeviceRegistry()->Logger().setLevel(P);
			else if (Sub == "all") {
				AuthService()->Logger().setLevel(P);
				FileUploader()->Logger().setLevel(P);
				WebSocketServer()->Logger().setLevel(P);
				Storage()->Logger().setLevel(P);
				RESTAPI_server()->Logger().setLevel(P);
				CommandManager()->Logger().setLevel(P);
				DeviceRegistry()->Logger().setLevel(P);
			} else
				return false;
			return true;
		} catch (const Poco::Exception & E) {

		}
		return false;
	}

	uint64_t Daemon::ConfigGetInt(const std::string &Key,uint64_t Default) {
		return (uint64_t) config().getInt64(Key,Default);
	}

	uint64_t Daemon::ConfigGetInt(const std::string &Key) {
		return config().getInt(Key);
	}

	uint64_t Daemon::ConfigGetBool(const std::string &Key,bool Default) {
		return config().getBool(Key,Default);
	}

	uint64_t Daemon::ConfigGetBool(const std::string &Key) {
		return config().getBool(Key);
	}

	std::string Daemon::ConfigGetString(const std::string &Key,const std::string & Default) {
		std::string R = config().getString(Key, Default);
		return Poco::Path::expand(R);
	}

	std::string Daemon::ConfigGetString(const std::string &Key) {
		std::string R = config().getString(Key);
		return Poco::Path::expand(R);
	}

    int Daemon::main(const ArgVec &args) {

        Poco::ErrorHandler::set(&AppErrorHandler_);

        if (!HelpRequested_) {
            Poco::Logger &logger = Poco::Logger::get(DAEMON_APP_NAME);
			logger.notice(Poco::format("Starting %s version %s.",std::string(uCentral::DAEMON_APP_NAME), Version()));

			if(Poco::Net::Socket::supportsIPv6())
				logger.information("System supports IPv6.");
			else
				logger.information("System does NOT support IPv6.");

			if (config().getBool("application.runAsDaemon", false))
			{
				logger.information("Starting as a daemon.");
			}

			StartSubSystemServers();
			instance()->waitForTerminationRequest();
			StopSubSystemServers();

			logger.notice(Poco::format("Stopped %s...",std::string(uCentral::DAEMON_APP_NAME)));
        }

        return Application::EXIT_OK;
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