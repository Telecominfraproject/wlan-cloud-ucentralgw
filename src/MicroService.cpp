//
// Created by stephane bourque on 2021-06-22.
//
#include <cstdlib>
#include <boost/algorithm/string.hpp>

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Environment.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/FTPSStreamFactory.h"
#include "Poco/Net/FTPStreamFactory.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "CentralConfig.h"

#include "MicroService.h"
#include "Utils.h"

namespace uCentral {

	void MyErrorHandler::exception(const Poco::Exception & E) {
		Poco::Thread * CurrentThread = Poco::Thread::current();
		App_.logger().log(E);
		App_.logger().error(Poco::format("Exception occurred in %s",CurrentThread->getName()));
	}

	void MyErrorHandler::exception(const std::exception & E) {
		Poco::Thread * CurrentThread = Poco::Thread::current();
		App_.logger().warning(Poco::format("std::exception on %s",CurrentThread->getName()));
	}

	void MyErrorHandler::exception() {
		Poco::Thread * CurrentThread = Poco::Thread::current();
		App_.logger().warning(Poco::format("exception on %s",CurrentThread->getName()));
	}

	void MicroService::Exit(int Reason) {
		std::exit(Reason);
	}

	void MicroService::initialize(Poco::Util::Application &self) {
		Poco::Net::initializeSSL();
		Poco::Net::HTTPStreamFactory::registerFactory();
		Poco::Net::HTTPSStreamFactory::registerFactory();
		Poco::Net::FTPStreamFactory::registerFactory();
		Poco::Net::FTPSStreamFactory::registerFactory();
		std::string Location = Poco::Environment::get(DAEMON_CONFIG_ENV_VAR,".");
		Poco::Path ConfigFile;

		ConfigFile = ConfigFileName_.empty() ? Location + "/" + DAEMON_PROPERTIES_FILENAME : ConfigFileName_;

		if(!ConfigFile.isFile())
		{
			std::cerr << DAEMON_APP_NAME << ": Configuration "
					  << ConfigFile.toString() << " does not seem to exist. Please set " + DAEMON_CONFIG_ENV_VAR
												  + " env variable the path of the " + DAEMON_PROPERTIES_FILENAME + " file." << std::endl;
			std::exit(Poco::Util::Application::EXIT_CONFIG);
		}

		static const char * LogFilePathKey = "logging.channels.c2.path";

		loadConfiguration(ConfigFile.toString());

		if(LogDir_.empty()) {
			std::string OriginalLogFileValue = ConfigPath(LogFilePathKey);
			config().setString(LogFilePathKey, OriginalLogFileValue);
		} else {
			config().setString(LogFilePathKey, LogDir_);
		}
		Poco::File	DataDir(ConfigPath("ucentral.system.data"));
		DataDir_ = DataDir.path();
		if(!DataDir.exists()) {
			try {
				DataDir.createDirectory();
			} catch (const Poco::Exception &E) {
				logger().log(E);
			}
		}
		std::string KeyFile = ConfigPath("ucentral.service.key");
		AppKey_ = Poco::SharedPtr<Poco::Crypto::RSAKey>(new Poco::Crypto::RSAKey("", KeyFile, ""));
		Cipher_ = CipherFactory_.createCipher(*AppKey_);
		ID_ = Utils::GetSystemId();
		if(!DebugMode_)
			DebugMode_ = ConfigGetBool("ucentral.system.debug",false);

		InitializeSubSystemServers();
		ServerApplication::initialize(self);
	}

	void MicroService::uninitialize() {
		// add your own uninitialization code here
		ServerApplication::uninitialize();
	}

	void MicroService::reinitialize(Poco::Util::Application &self) {
		ServerApplication::reinitialize(self);
		// add your own reinitialization code here
	}

	void MicroService::defineOptions(Poco::Util::OptionSet &options) {
		ServerApplication::defineOptions(options);

		options.addOption(
			Poco::Util::Option("help", "", "display help information on command line arguments")
				.required(false)
				.repeatable(false)
				.callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleHelp)));

		options.addOption(
			Poco::Util::Option("file", "", "specify the configuration file")
				.required(false)
				.repeatable(false)
				.argument("file")
				.callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleConfig)));

		options.addOption(
			Poco::Util::Option("debug", "", "to run in debug, set to true")
				.required(false)
				.repeatable(false)
				.callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleDebug)));

		options.addOption(
			Poco::Util::Option("logs", "", "specify the log directory and file (i.e. dir/file.log)")
				.required(false)
				.repeatable(false)
				.argument("dir")
				.callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleLogs)));

		options.addOption(
			Poco::Util::Option("version", "", "get the version and quit.")
				.required(false)
				.repeatable(false)
				.callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleVersion)));

	}

	std::string MicroService::Version() {
		std::string V = APP_VERSION;
		std::string B = BUILD_NUMBER;
		return V + "(" + B +  ")";
	}

	void MicroService::handleHelp(const std::string &name, const std::string &value) {
		HelpRequested_ = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void MicroService::handleVersion(const std::string &name, const std::string &value) {
		HelpRequested_ = true;
		std::cout << Version() << std::endl;
		stopOptionsProcessing();
	}

	void MicroService::handleDebug(const std::string &name, const std::string &value) {
		if(value == "true")
			DebugMode_ = true ;
	}

	void MicroService::handleLogs(const std::string &name, const std::string &value) {
		LogDir_ = value;
	}

	void MicroService::handleConfig(const std::string &name, const std::string &value) {
		ConfigFileName_ = value;
	}

	void MicroService::displayHelp() {
		Poco::Util::HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A " + DAEMON_APP_NAME + " implementation for TIP.");
		helpFormatter.format(std::cout);
	}

	void MicroService::InitializeSubSystemServers() {
		for(auto i:SubSystems_)
			addSubsystem(i);
	}

	void MicroService::StartSubSystemServers() {
		for(auto i:SubSystems_)
			i->Start();
	}

	void MicroService::StopSubSystemServers() {
		for(auto i=SubSystems_.rbegin(); i!=SubSystems_.rend(); ++i)
			(*i)->Stop();
	}

	std::string MicroService::CreateUUID() {
		return UUIDGenerator_.create().toString();
	}

	bool MicroService::SetSubsystemLogLevel(const std::string &SubSystem, const std::string &Level) {
		try {
			auto P = Poco::Logger::parseLevel(Level);
			auto Sub = Poco::toLower(SubSystem);

			if (Sub == "all") {
				for (auto i : SubSystems_) {
					i->Logger().setLevel(P);
				}
				return true;
			} else {
				std::cout << "Sub:" << SubSystem << " Level:" << Level << std::endl;
				for (auto i : SubSystems_) {
					if (Sub == Poco::toLower(i->Name())) {
						i->Logger().setLevel(P);
						return true;
					}
				}
			}
		} catch (const Poco::Exception & E) {
			std::cout << "Exception" << std::endl;
		}
		return false;
	}

	Types::StringVec MicroService::GetSubSystems() const {
		Types::StringVec Result;
		for(auto i:SubSystems_)
			Result.push_back(i->Name());
		return Result;
	}

	Types::StringPairVec MicroService::GetLogLevels() const {
		Types::StringPairVec Result;

		for(auto &i:SubSystems_) {
			auto P = std::make_pair( i->Name(), Utils::LogLevelToString(i->GetLoggingLevel()));
			Result.push_back(P);
		}
		return Result;
	}

	const Types::StringVec & MicroService::GetLogLevelNames() const {
		static Types::StringVec LevelNames{"none", "fatal", "critical", "error", "warning", "notice", "information", "debug", "trace" };
		return LevelNames;
	}

	uint64_t MicroService::ConfigGetInt(const std::string &Key,uint64_t Default) {
		return (uint64_t) config().getInt64(Key,Default);
	}

	uint64_t MicroService::ConfigGetInt(const std::string &Key) {
		return config().getInt(Key);
	}

	uint64_t MicroService::ConfigGetBool(const std::string &Key,bool Default) {
		return config().getBool(Key,Default);
	}

	uint64_t MicroService::ConfigGetBool(const std::string &Key) {
		return config().getBool(Key);
	}

	std::string MicroService::ConfigGetString(const std::string &Key,const std::string & Default) {
		return config().getString(Key, Default);
	}

	std::string MicroService::ConfigGetString(const std::string &Key) {
		return config().getString(Key);
	}

	std::string MicroService::ConfigPath(const std::string &Key,const std::string & Default) {
		std::string R = config().getString(Key, Default);
		return Poco::Path::expand(R);
	}

	std::string MicroService::ConfigPath(const std::string &Key) {
		std::string R = config().getString(Key);
		return Poco::Path::expand(R);
	}

	std::string MicroService::Encrypt(const std::string &S) {
		return Cipher_->encryptString(S, Poco::Crypto::Cipher::Cipher::ENC_BASE64);;
	}

	std::string MicroService::Decrypt(const std::string &S) {
		return Cipher_->decryptString(S, Poco::Crypto::Cipher::Cipher::ENC_BASE64);;
	}

	int MicroService::main(const ArgVec &args) {

		MyErrorHandler	ErrorHandler(*this);
		Poco::ErrorHandler::set(&ErrorHandler);

		if (!HelpRequested_) {
			Poco::Logger &logger = Poco::Logger::get(DAEMON_APP_NAME);
			logger.notice(Poco::format("Starting %s version %s.",DAEMON_APP_NAME, Version()));

			if(Poco::Net::Socket::supportsIPv6())
				logger.information("System supports IPv6.");
			else
				logger.information("System does NOT support IPv6.");

			if (config().getBool("application.runAsDaemon", false)) {
				logger.information("Starting as a daemon.");
			}
			logger.information(Poco::format("System ID set to %Lu",ID_));
			StartSubSystemServers();
			waitForTerminationRequest();
			StopSubSystemServers();

			logger.notice(Poco::format("Stopped %s...",DAEMON_APP_NAME));
		}

		return Application::EXIT_OK;
	}



}