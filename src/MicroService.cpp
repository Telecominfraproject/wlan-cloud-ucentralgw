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
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/FTPSStreamFactory.h"
#include "Poco/Net/FTPStreamFactory.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/String.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

#include "ALBHealthCheckServer.h"
#ifndef SMALL_BUILD
#include "KafkaManager.h"
#endif
#include "Kafka_topics.h"

#include "MicroService.h"
#include "Utils.h"

#ifndef TIP_SECURITY_SERVICE
#include "AuthClient.h"
#endif

namespace OpenWifi {

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

	void MicroService::BusMessageReceived(const std::string &Key, const std::string & Message) {
		std::lock_guard G(InfraMutex_);
		try {
			Poco::JSON::Parser P;
			auto Object = P.parse(Message).extract<Poco::JSON::Object::Ptr>();
			if (Object->has(KafkaTopics::ServiceEvents::Fields::ID) &&
				Object->has(KafkaTopics::ServiceEvents::Fields::EVENT)) {
				uint64_t 	ID = Object->get(KafkaTopics::ServiceEvents::Fields::ID);
				auto 		Event = Object->get(KafkaTopics::ServiceEvents::Fields::EVENT).toString();
				if (ID != ID_) {
					if(	Event==KafkaTopics::ServiceEvents::EVENT_JOIN ||
						Event==KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE ||
						Event==KafkaTopics::ServiceEvents::EVENT_LEAVE ) {
						if(	Object->has(KafkaTopics::ServiceEvents::Fields::TYPE) &&
						   	Object->has(KafkaTopics::ServiceEvents::Fields::PUBLIC) &&
							Object->has(KafkaTopics::ServiceEvents::Fields::PRIVATE) &&
						   	Object->has(KafkaTopics::ServiceEvents::Fields::VRSN) &&
							Object->has(KafkaTopics::ServiceEvents::Fields::KEY)) {

							if (Event == KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE && Services_.find(ID) != Services_.end()) {
								Services_[ID].LastUpdate = std::time(nullptr);
							} else if (Event == KafkaTopics::ServiceEvents::EVENT_LEAVE) {
								Services_.erase(ID);
								logger().information(Poco::format("Service %s ID=%Lu leaving system.",Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),ID));
							} else if (Event == KafkaTopics::ServiceEvents::EVENT_JOIN || Event == KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE) {
								logger().information(Poco::format("Service %s ID=%Lu joining system.",Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),ID));
								Services_[ID] = MicroServiceMeta{
									.Id = ID,
									.Type = Poco::toLower(Object->get(KafkaTopics::ServiceEvents::Fields::TYPE).toString()),
									.PrivateEndPoint = Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),
									.PublicEndPoint = Object->get(KafkaTopics::ServiceEvents::Fields::PUBLIC).toString(),
									.AccessKey = Object->get(KafkaTopics::ServiceEvents::Fields::KEY).toString(),
									.Version = Object->get(KafkaTopics::ServiceEvents::Fields::VRSN).toString(),
									.LastUpdate = (uint64_t)std::time(nullptr)};
								for (const auto &[Id, Svc] : Services_) {
									logger().information(Poco::format("ID: %Lu Type: %s EndPoint: %s",Id,Svc.Type,Svc.PrivateEndPoint));
								}
							}
						} else {
							logger().error(Poco::format("KAFKA-MSG: invalid event '%s', missing a field.",Event));
						}
					} else if (Event==KafkaTopics::ServiceEvents::EVENT_REMOVE_TOKEN) {
						if(Object->has(KafkaTopics::ServiceEvents::Fields::TOKEN)) {
#ifndef TIP_SECURITY_SERVICE
							AuthClient()->RemovedCachedToken(Object->get(KafkaTopics::ServiceEvents::Fields::TOKEN).toString());
#endif
						} else {
							logger().error(Poco::format("KAFKA-MSG: invalid event '%s', missing token",Event));
						}
					} else {
						logger().error(Poco::format("Unknown Event: %s Source: %Lu", Event, ID));
					}
				}
			} else {
				logger().error("Bad bus message.");
			}

			auto i=Services_.begin();
			auto Now = (uint64_t )std::time(nullptr);
			for(;i!=Services_.end();) {
			    if((Now - i->second.LastUpdate)>60) {
			        i = Services_.erase(i);
			    } else
			        ++i;
			}

		} catch (const Poco::Exception &E) {
			logger().log(E);
		}
	}

	MicroServiceMetaVec MicroService::GetServices(const std::string & Type) {
		std::lock_guard G(InfraMutex_);

		auto T = Poco::toLower(Type);
		MicroServiceMetaVec	Res;
		for(const auto &[Id,ServiceRec]:Services_) {
			if(ServiceRec.Type==T)
				Res.push_back(ServiceRec);
		}
		return Res;
	}

	MicroServiceMetaVec MicroService::GetServices() {
		std::lock_guard G(InfraMutex_);

		MicroServiceMetaVec	Res;
		for(const auto &[Id,ServiceRec]:Services_) {
			Res.push_back(ServiceRec);
		}
		return Res;
	}

	void MicroService::initialize(Poco::Util::Application &self) {
		// add the default services
		SubSystems_.push_back(KafkaManager());
		SubSystems_.push_back(ALBHealthCheckServer());

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
		Poco::File	DataDir(ConfigPath("openwifi.system.data"));
		DataDir_ = DataDir.path();
		if(!DataDir.exists()) {
			try {
				DataDir.createDirectory();
			} catch (const Poco::Exception &E) {
				logger().log(E);
			}
		}
		std::string KeyFile = ConfigPath("openwifi.service.key");
		std::string KeyFilePassword = ConfigPath("openwifi.service.key.password" , "" );
		AppKey_ = Poco::SharedPtr<Poco::Crypto::RSAKey>(new Poco::Crypto::RSAKey("", KeyFile, KeyFilePassword));
		Cipher_ = CipherFactory_.createCipher(*AppKey_);
		ID_ = Utils::GetSystemId();
		if(!DebugMode_)
			DebugMode_ = ConfigGetBool("openwifi.system.debug",false);
		MyPrivateEndPoint_ = ConfigGetString("openwifi.system.uri.private");
		MyPublicEndPoint_ = ConfigGetString("openwifi.system.uri.public");
		UIURI_ = ConfigGetString("openwifi.system.uri.ui");
		MyHash_ = CreateHash(MyPublicEndPoint_);
		InitializeSubSystemServers();
		ServerApplication::initialize(self);

		Types::TopicNotifyFunction F = [this](std::string s1,std::string s2) { this->BusMessageReceived(s1,s2); };
		KafkaManager()->RegisterTopicWatcher(KafkaTopics::SERVICE_EVENTS, F);
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
		for(auto i:SubSystems_) {
			i->Start();
		}
		BusEventManager_.Start();
	}

	void MicroService::StopSubSystemServers() {
		BusEventManager_.Stop();
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
				// std::cout << "Sub:" << SubSystem << " Level:" << Level << std::endl;
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

	void MicroService::Reload(const std::string &Sub) {
		for (auto i : SubSystems_) {
			if (Poco::toLower(Sub) == Poco::toLower(i->Name())) {
				i->reinitialize(Poco::Util::Application::instance());
				return;
			}
		}
	}

	Types::StringVec MicroService::GetSubSystems() const {
		Types::StringVec Result;
		for(auto i:SubSystems_)
			Result.push_back(Poco::toLower(i->Name()));
		return Result;
	}

	Types::StringPairVec MicroService::GetLogLevels() {
		Types::StringPairVec Result;

		for(auto &i:SubSystems_) {
			auto P = std::make_pair( i->Name(), Utils::LogLevelToString(i->GetLoggingLevel()));
			Result.push_back(P);
		}
		return Result;
	}

	const Types::StringVec & MicroService::GetLogLevelNames() {
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

	std::string MicroService::CreateHash(const std::string &S) {
		SHA2_.update(S);
		return Utils::ToHex(SHA2_.digest());
	}

	std::string MicroService::MakeSystemEventMessage( const std::string & Type ) const {
		Poco::JSON::Object	Obj;
		Obj.set(KafkaTopics::ServiceEvents::Fields::EVENT,Type);
		Obj.set(KafkaTopics::ServiceEvents::Fields::ID,ID_);
		Obj.set(KafkaTopics::ServiceEvents::Fields::TYPE,Poco::toLower(DAEMON_APP_NAME));
		Obj.set(KafkaTopics::ServiceEvents::Fields::PUBLIC,MyPublicEndPoint_);
		Obj.set(KafkaTopics::ServiceEvents::Fields::PRIVATE,MyPrivateEndPoint_);
		Obj.set(KafkaTopics::ServiceEvents::Fields::KEY,MyHash_);
		Obj.set(KafkaTopics::ServiceEvents::Fields::VRSN,Version_);
		std::stringstream ResultText;
		Poco::JSON::Stringifier::stringify(Obj, ResultText);
		return ResultText.str();
	}

	void BusEventManager::run() {
		Running_ = true;
		auto Msg = Daemon()->MakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_JOIN);
		KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,Daemon()->PrivateEndPoint(),Msg, false);
		while(Running_) {
			Poco::Thread::trySleep((unsigned long)Daemon()->DaemonBusTimer());
			if(!Running_)
				break;
			Msg = Daemon()->MakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE);
			KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,Daemon()->PrivateEndPoint(),Msg, false);
		}
		Msg = Daemon()->MakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_LEAVE);
		KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,Daemon()->PrivateEndPoint(),Msg, false);
	};

	void BusEventManager::Start() {
		if(KafkaManager()->Enabled()) {
			Thread_.start(*this);
		}
	}

	void BusEventManager::Stop() {
		if(KafkaManager()->Enabled()) {
			Running_ = false;
			Thread_.wakeUp();
			Thread_.join();
		}
	}

	[[nodiscard]] bool MicroService::IsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request) {
		try {
			auto APIKEY = Request.get("X-API-KEY");
			return APIKEY == MyHash_;
		} catch (const Poco::Exception &E) {
			logger().log(E);
		}
		return false;
	}

	void MicroService::SavePID() {
		try {
			std::ofstream O;
			O.open(Daemon()->DataDir() + "/pidfile",std::ios::binary | std::ios::trunc);
			O << Poco::Process::id();
			O.close();
		} catch (...)
		{
			std::cout << "Could not save system ID" << std::endl;
		}
	}

	int MicroService::main(const ArgVec &args) {

		MyErrorHandler	ErrorHandler(*this);
		Poco::ErrorHandler::set(&ErrorHandler);

		if (!HelpRequested_) {
			SavePID();
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