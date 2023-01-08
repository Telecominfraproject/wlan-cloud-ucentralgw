//
// Created by stephane bourque on 2022-10-26.
//

#include "Poco/FileChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/AsyncChannel.h"
#include "Poco/NullChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/FTPSStreamFactory.h"
#include "Poco/Net/FTPStreamFactory.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/JSON/JSONException.h"

#include "framework/MicroService.h"
#include "framework/MicroServiceErrorHandler.h"
#include "framework/UI_WebSocketClientServer.h"
#include "framework/MicroServiceNames.h"
#include "framework/AuthClient.h"
#include "framework/ALBserver.h"
#include "framework/KafkaManager.h"
#include "framework/RESTAPI_GenericServerAccounting.h"
#include "framework/RESTAPI_ExtServer.h"
#include "framework/RESTAPI_IntServer.h"
#include "framework/utils.h"
#include "framework/WebSocketLogger.h"

namespace OpenWifi {

	void MicroService::Exit(int Reason) {
		std::exit(Reason);
	}

	void MicroService::BusMessageReceived([[maybe_unused]] const std::string &Key, const std::string & Payload) {
		std::lock_guard G(InfraMutex_);
		try {
			Poco::JSON::Parser P;
			auto Object = P.parse(Payload).extract<Poco::JSON::Object::Ptr>();

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
							auto PrivateEndPoint = Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString();
							if (Event == KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE && Services_.find(PrivateEndPoint) != Services_.end()) {
								Services_[PrivateEndPoint].LastUpdate = Utils::Now();
							} else if (Event == KafkaTopics::ServiceEvents::EVENT_LEAVE) {
								Services_.erase(PrivateEndPoint);
								poco_debug(logger(),fmt::format("Service {} ID={} leaving system.",Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),ID));
							} else if (Event == KafkaTopics::ServiceEvents::EVENT_JOIN || Event == KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE) {
								poco_debug(logger(),fmt::format("Service {} ID={} joining system.",Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),ID));
								Services_[PrivateEndPoint] = Types::MicroServiceMeta{
									.Id = ID,
									.Type = Poco::toLower(Object->get(KafkaTopics::ServiceEvents::Fields::TYPE).toString()),
									.PrivateEndPoint = Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),
									.PublicEndPoint = Object->get(KafkaTopics::ServiceEvents::Fields::PUBLIC).toString(),
									.AccessKey = Object->get(KafkaTopics::ServiceEvents::Fields::KEY).toString(),
									.Version = Object->get(KafkaTopics::ServiceEvents::Fields::VRSN).toString(),
									.LastUpdate = Utils::Now() };

								std::string SvcList;
								for (const auto &Svc: Services_) {
									if(SvcList.empty())
										SvcList = Svc.second.Type;
									else
										SvcList += ", " + Svc.second.Type;
								}
								poco_information(logger(),fmt::format("Current list of microservices: {}", SvcList));
							}
						} else {
							poco_error(logger(),fmt::format("KAFKA-MSG: invalid event '{}', missing a field.",Event));
						}
					} else if (Event==KafkaTopics::ServiceEvents::EVENT_REMOVE_TOKEN) {
						if(Object->has(KafkaTopics::ServiceEvents::Fields::TOKEN)) {
	#ifndef TIP_SECURITY_SERVICE
							AuthClient()->RemovedCachedToken(Object->get(KafkaTopics::ServiceEvents::Fields::TOKEN).toString());
	#endif
						} else {
							poco_error(logger(),fmt::format("KAFKA-MSG: invalid event '{}', missing token",Event));
						}
					} else {
						poco_error(logger(),fmt::format("Unknown Event: {} Source: {}", Event, ID));
					}
				}
			} else {
				poco_error(logger(),"Bad bus message.");
			}

			auto i=Services_.begin();
			auto now = Utils::Now();
			for(;i!=Services_.end();) {
				if((now - i->second.LastUpdate)>60) {
					i = Services_.erase(i);
				} else
					++i;
			}

		} catch (const Poco::Exception &E) {
			logger().log(E);
		}
	}

	Types::MicroServiceMetaVec MicroService::GetServices(const std::string & Type) {
		std::lock_guard G(InfraMutex_);

		auto T = Poco::toLower(Type);
		Types::MicroServiceMetaVec	Res;
		for(const auto &[_,ServiceRec]:Services_) {
			if(ServiceRec.Type==T)
				Res.push_back(ServiceRec);
		}
		return Res;
	}

	Types::MicroServiceMetaVec MicroService::GetServices() {
		std::lock_guard G(InfraMutex_);

		Types::MicroServiceMetaVec	Res;
		for(const auto &[_,ServiceRec]:Services_) {
			Res.push_back(ServiceRec);
		}
		return Res;
	}

	void MicroService::LoadConfigurationFile() {
		std::string Location = Poco::Environment::get(DAEMON_CONFIG_ENV_VAR,".");
		ConfigFileName_ = ConfigFileName_.empty() ? Location + "/" + DAEMON_PROPERTIES_FILENAME : ConfigFileName_;
		Poco::Path ConfigFile(ConfigFileName_);

		if(!ConfigFile.isFile())
		{
			std::cerr << DAEMON_APP_NAME << ": Configuration "
					  << ConfigFile.toString() << " does not seem to exist. Please set " + DAEMON_CONFIG_ENV_VAR
													  + " env variable the path of the " + DAEMON_PROPERTIES_FILENAME + " file." << std::endl;
			std::exit(Poco::Util::Application::EXIT_CONFIG);
		}

		// 	    loadConfiguration(ConfigFile.toString());
		PropConfigurationFile_ = new Poco::Util::PropertyFileConfiguration(ConfigFile.toString());
		configPtr()->addWriteable(PropConfigurationFile_, PRIO_DEFAULT);
	}

	void MicroService::Reload() {
		LoadConfigurationFile();
		LoadMyConfig();
	}

	void MicroService::LoadMyConfig() {
		NoAPISecurity_ = ConfigGetBool("openwifi.security.restapi.disable",false);
		std::string KeyFile = ConfigPath("openwifi.service.key","");
		if(!KeyFile.empty()) {
			std::string KeyFilePassword = ConfigPath("openwifi.service.key.password", "");
			AppKey_ = Poco::SharedPtr<Poco::Crypto::RSAKey>(new Poco::Crypto::RSAKey("", KeyFile, KeyFilePassword));
			Cipher_ = CipherFactory_.createCipher(*AppKey_);
			Signer_.setRSAKey(AppKey_);
			Signer_.addAllAlgorithms();
			NoBuiltInCrypto_ = false;
		} else {
			NoBuiltInCrypto_ = true;
		}

		ID_ = Utils::GetSystemId();
		if(!DebugMode_)
			DebugMode_ = ConfigGetBool("openwifi.system.debug",false);
		MyPrivateEndPoint_ = ConfigGetString("openwifi.system.uri.private");
		MyPublicEndPoint_ = ConfigGetString("openwifi.system.uri.public");
		UIURI_ = ConfigGetString("openwifi.system.uri.ui");
		MyHash_ = Utils::ComputeHash(MyPublicEndPoint_);
	}

	void MicroService::InitializeLoggingSystem() {
		static auto initialized = false;

		if(!initialized) {
			initialized = true;
			LoadConfigurationFile();

			auto LoggingDestination = MicroService::instance().ConfigGetString("logging.type", "file");
			auto LoggingFormat = MicroService::instance().ConfigGetString("logging.format",
																		  "%Y-%m-%d %H:%M:%S.%i %s: [%p][thr:%I] %t");
			auto UseAsyncLogs_ = MicroService::instance().ConfigGetBool("logging.asynch", true);
			auto DisableWebSocketLogging = MicroService::instance().ConfigGetBool("logging.websocket",false);

			if (LoggingDestination == "null") {
				Poco::AutoPtr<Poco::NullChannel> DevNull(new Poco::NullChannel);
				Poco::Logger::root().setChannel(DevNull);
			} else if (LoggingDestination == "console") {
                SetConsoleLogs(UseAsyncLogs_, DisableWebSocketLogging, LoggingFormat);
			} else if (LoggingDestination == "colorconsole") {
                SetColorConsoleLogs(UseAsyncLogs_, DisableWebSocketLogging, LoggingFormat);
			} else if (LoggingDestination == "sql") {
                SetSQLLogs(UseAsyncLogs_, DisableWebSocketLogging, LoggingFormat);
			} else if (LoggingDestination == "syslog") {
                SetSyslogLogs(UseAsyncLogs_, DisableWebSocketLogging, LoggingFormat);
			} else {
                SetFileLogs(UseAsyncLogs_, DisableWebSocketLogging, LoggingFormat, DAEMON_ROOT_ENV_VAR);
            }

			auto Level = Poco::Logger::parseLevel(MicroService::instance().ConfigGetString("logging.level", "debug"));
			Poco::Logger::root().setLevel(Level);
			if(!DisableWebSocketLogging) {
				static const UI_WebSocketClientServer::NotificationTypeIdVec Notifications = {
					{1, "log"}};
				UI_WebSocketClientServer()->RegisterNotifications(Notifications);
			}
		}
    }

    void MicroService::SetConsoleLogs(bool UseAsync, bool DisableWebSocketLogging, const std::string & FormatterPattern) {

        Poco::AutoPtr<Poco::ConsoleChannel> Console(new Poco::ConsoleChannel);
        Poco::AutoPtr<Poco::PatternFormatter> Formatter(new Poco::PatternFormatter);
        Formatter->setProperty("pattern", FormatterPattern);
        Poco::AutoPtr<Poco::FormattingChannel> FormattingChannel(new Poco::FormattingChannel(Formatter, Console));

        if(DisableWebSocketLogging) {
            if(UseAsync) {
                Poco::AutoPtr<Poco::AsyncChannel> Async(new Poco::AsyncChannel(FormattingChannel));
                Poco::Logger::root().setChannel(Async);
            } else {
                Poco::Logger::root().setChannel(FormattingChannel);
            }
        } else {
            Poco::AutoPtr<WebSocketLogger>			WSLogger(new WebSocketLogger);
            Poco::AutoPtr<Poco::SplitterChannel>	Splitter(new Poco::SplitterChannel);
            Splitter->addChannel(WSLogger);
            Splitter->addChannel(FormattingChannel);
            if(UseAsync) {
                Poco::AutoPtr<Poco::AsyncChannel> Async(new Poco::AsyncChannel(Splitter));
                Poco::Logger::root().setChannel(Async);
            } else {
                Poco::Logger::root().setChannel(Splitter);
            }
        }
		Poco::Logger::root().information(fmt::format("Enabled console logs: asynch={} websocket={}",UseAsync,DisableWebSocketLogging));
    }

    void MicroService::SetColorConsoleLogs(bool UseAsync, bool DisableWebSocketLogging, const std::string & FormatterPattern) {

        Poco::AutoPtr<Poco::ColorConsoleChannel> Console(new Poco::ColorConsoleChannel);
        Poco::AutoPtr<Poco::PatternFormatter> Formatter(new Poco::PatternFormatter);
        Formatter->setProperty("pattern", FormatterPattern);
        Poco::AutoPtr<Poco::FormattingChannel> FormattingChannel(new Poco::FormattingChannel(Formatter, Console));

        if(DisableWebSocketLogging) {
            if(UseAsync) {
                Poco::AutoPtr<Poco::AsyncChannel> Async(new Poco::AsyncChannel(FormattingChannel));
                Poco::Logger::root().setChannel(Async);
            } else {
                Poco::Logger::root().setChannel(FormattingChannel);
            }
        } else {
            Poco::AutoPtr<WebSocketLogger>			WSLogger(new WebSocketLogger);
            Poco::AutoPtr<Poco::SplitterChannel>	Splitter(new Poco::SplitterChannel);
            Splitter->addChannel(WSLogger);
            Splitter->addChannel(FormattingChannel);
            if(UseAsync) {
                Poco::AutoPtr<Poco::AsyncChannel> Async(new Poco::AsyncChannel(Splitter));
                Poco::Logger::root().setChannel(Async);
            } else {
                Poco::Logger::root().setChannel(Splitter);
            }
        }
		Poco::Logger::root().information(fmt::format("Enabled color console logs: asynch={} websocket={}",UseAsync,DisableWebSocketLogging));
    }

    void MicroService::SetSQLLogs([[maybe_unused]] bool UseAsync,[[maybe_unused]]  bool DisableWebSocketLogging,[[maybe_unused]]  const std::string & FormatterPattern) {
        //"CREATE TABLE T_POCO_LOG (Source VARCHAR, Name VARCHAR, ProcessId INTEGER, Thread VARCHAR, ThreadId INTEGER, Priority INTEGER, Text VARCHAR, DateTime DATE)"
    }

    void MicroService::SetSyslogLogs([[maybe_unused]] bool UseAsync,[[maybe_unused]]  bool DisableWebSocketLogging,[[maybe_unused]]  const std::string & FormatterPattern) {

    }

    void MicroService::SetFileLogs(bool UseAsync, bool DisableWebSocketLogging, const std::string & FormatterPattern, const std::string & root_env_var) {
        std::string DefaultLogPath = fmt::format("${}/logs",root_env_var);
        auto LoggingLocationDir = MicroService::instance().ConfigPath("logging.path", DefaultLogPath);
        Poco::File      LD(LoggingLocationDir);
        try {
            if(!LD.exists()) {
                LD.createDirectory();
            }
        } catch(const Poco::Exception &E) {
            std::cout << "Cannot create " << LD.path() << "  Error: " << E.message() << std::endl;
        }
        auto LoggingLocationDirFilePattern = LoggingLocationDir + "/log";

        Poco::AutoPtr<Poco::FileChannel> FileChannel(new Poco::FileChannel);
        FileChannel->setProperty("rotation", "10 M");
        FileChannel->setProperty("archive", "timestamp");
        FileChannel->setProperty("purgeCount", "10");
        FileChannel->setProperty("path", LoggingLocationDirFilePattern);

        Poco::AutoPtr<Poco::PatternFormatter> Formatter(new Poco::PatternFormatter);
        Formatter->setProperty("pattern", FormatterPattern);
        Poco::AutoPtr<Poco::FormattingChannel> FormattingChannel(new Poco::FormattingChannel(Formatter, FileChannel));

        if(DisableWebSocketLogging) {
            if(UseAsync) {
                Poco::AutoPtr<Poco::AsyncChannel> Async(new Poco::AsyncChannel(FormattingChannel));
                Poco::Logger::root().setChannel(Async);
            } else {
                Poco::Logger::root().setChannel(FormattingChannel);
            }
        } else {
            Poco::AutoPtr<WebSocketLogger>			WSLogger(new WebSocketLogger);
            Poco::AutoPtr<Poco::SplitterChannel>	Splitter(new Poco::SplitterChannel);
            Splitter->addChannel(WSLogger);
            Splitter->addChannel(FormattingChannel);
            if(UseAsync) {
                Poco::AutoPtr<Poco::AsyncChannel> Async(new Poco::AsyncChannel(Splitter));
                Poco::Logger::root().setChannel(Async);
            } else {
                Poco::Logger::root().setChannel(Splitter);
            }
        }
		Poco::Logger::root().information(fmt::format("Enabled file logs: asynch={} websocket={}",UseAsync,DisableWebSocketLogging));
    }

	void DaemonPostInitialization(Poco::Util::Application &self);

	void MicroService::initialize(Poco::Util::Application &self) {
		// add the default services
		LoadConfigurationFile();
		InitializeLoggingSystem();

		SubSystems_.push_back(KafkaManager());
		SubSystems_.push_back(ALBHealthCheckServer());
		SubSystems_.push_back(RESTAPI_ExtServer());
		SubSystems_.push_back(RESTAPI_IntServer());
	#ifndef TIP_SECURITY_SERVICE
		SubSystems_.push_back(AuthClient());
	#endif
		Poco::Net::initializeSSL();
		Poco::Net::HTTPStreamFactory::registerFactory();
		Poco::Net::HTTPSStreamFactory::registerFactory();
		Poco::Net::FTPStreamFactory::registerFactory();
		Poco::Net::FTPSStreamFactory::registerFactory();

		Poco::File	DataDir(ConfigPath("openwifi.system.data"));
		DataDir_ = DataDir.path();
		if(!DataDir.exists()) {
			try {
				DataDir.createDirectory();
			} catch (const Poco::Exception &E) {
				logger().log(E);
			}
		}
		WWWAssetsDir_ = ConfigPath("openwifi.restapi.wwwassets","");
		if(WWWAssetsDir_.empty())
			WWWAssetsDir_ = DataDir_;

		LoadMyConfig();

        AllowExternalMicroServices_ = ConfigGetBool("allowexternalmicroservices",true);

		InitializeSubSystemServers();
		ServerApplication::initialize(self);
		DaemonPostInitialization(self);

		Types::TopicNotifyFunction F = [this](const std::string &Key,const std::string &Payload) { this->BusMessageReceived(Key, Payload); };
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

	void MicroService::handleHelp([[maybe_unused]] const std::string &name, [[maybe_unused]] const std::string &value) {
		HelpRequested_ = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void MicroService::handleVersion([[maybe_unused]] const std::string &name, [[maybe_unused]] const std::string &value) {
		HelpRequested_ = true;
		std::cout << Version() << std::endl;
		stopOptionsProcessing();
	}

	void MicroService::handleDebug([[maybe_unused]] const std::string &name, const std::string &value) {
		if(value == "true")
			DebugMode_ = true ;
	}

	void MicroService::handleLogs([[maybe_unused]] const std::string &name, const std::string &value) {
		LogDir_ = value;
	}

	void MicroService::handleConfig([[maybe_unused]] const std::string &name, const std::string &value) {
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
		for(auto i:SubSystems_) {
			addSubsystem(i);
		}
	}

	void MicroService::StartSubSystemServers() {
		AddActivity("Starting");
		for(auto i:SubSystems_) {
			i->Start();
		}
		EventBusManager_ = std::make_unique<EventBusManager>(Poco::Logger::create("EventBusManager",Poco::Logger::root().getChannel(),Poco::Logger::root().getLevel()));
		EventBusManager_->Start();
	}

	void MicroService::StopSubSystemServers() {
		AddActivity("Stopping");
		EventBusManager_->Stop();
		for(auto i=SubSystems_.rbegin(); i!=SubSystems_.rend(); ++i) {
			(*i)->Stop();
		}
	}

	[[nodiscard]] std::string MicroService::CreateUUID() {
		static std::random_device              rd;
		static std::mt19937_64                 gen(rd());
		static std::uniform_int_distribution<> dis(0, 15);
		static std::uniform_int_distribution<> dis2(8, 11);

		std::stringstream ss;
		int i;
		ss << std::hex;
		for (i = 0; i < 8; i++) {
			ss << dis(gen);
		}
		ss << "-";
		for (i = 0; i < 4; i++) {
			ss << dis(gen);
		}
		ss << "-4";
		for (i = 0; i < 3; i++) {
			ss << dis(gen);
		}
		ss << "-";
		ss << dis2(gen);
		for (i = 0; i < 3; i++) {
			ss << dis(gen);
		}
		ss << "-";
		for (i = 0; i < 12; i++) {
			ss << dis(gen);
		};
		return ss.str();
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
				for (auto i : SubSystems_) {
					if (Sub == Poco::toLower(i->Name())) {
						i->Logger().setLevel(P);
						return true;
					}
				}
			}
		} catch (const Poco::Exception & E) {
			std::cerr << "Exception" << std::endl;
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
		if(NoBuiltInCrypto_) {
			return S;
		}
		return Cipher_->encryptString(S, Poco::Crypto::Cipher::Cipher::ENC_BASE64);;
	}

	std::string MicroService::Decrypt(const std::string &S) {
		if(NoBuiltInCrypto_) {
			return S;
		}
		return Cipher_->decryptString(S, Poco::Crypto::Cipher::Cipher::ENC_BASE64);;
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
			O.open(MicroService::instance().DataDir() + "/pidfile",std::ios::binary | std::ios::trunc);
			O << Poco::Process::id();
			O.close();
		} catch (...)
		{
			std::cout << "Could not save system ID" << std::endl;
		}
	}

	int MicroService::main([[maybe_unused]] const ArgVec &args) {
		MicroServiceErrorHandler	ErrorHandler(*this);
		Poco::ErrorHandler::set(&ErrorHandler);

		if (!HelpRequested_) {
			SavePID();

			Poco::Logger &logger = Poco::Logger::get(DAEMON_APP_NAME);
			logger.notice(fmt::format("Starting {} version {}.",DAEMON_APP_NAME, Version()));

			if(Poco::Net::Socket::supportsIPv6())
				poco_information(logger,"System supports IPv6.");
			else
				poco_information(logger,"System does NOT support IPv6.");

			if (config().getBool("application.runAsDaemon", false)) {
				poco_information(logger,"Starting as a daemon.");
			}

			poco_information(logger,fmt::format("System ID set to {}",ID_));
			StartSubSystemServers();
			waitForTerminationRequest();
			StopSubSystemServers();
			logger.notice(fmt::format("Stopped {}...",DAEMON_APP_NAME));
		}

		return Application::EXIT_OK;
	}

	void MicroService::AddActivity(const std::string &Activity) {
		if(!DataDir_.empty()) {
			std::string ActivityFile{ DataDir_ + "/activity.log"};
			try {
				std::ofstream of(ActivityFile,std::ios_base::app | std::ios_base::out );
				auto t = std::chrono::system_clock::now();
				std::time_t now = std::chrono::system_clock::to_time_t(t);
				of << Activity << " at " << std::ctime(&now) ;
			} catch (...) {

			}
		}
	}

	[[nodiscard]] std::string MicroService::Sign(Poco::JWT::Token &T, const std::string &Algo) {
		if(NoBuiltInCrypto_) {
			return T.toString();
		} else {
			return Signer_.sign(T,Algo);
		}
	}

	void MicroService::DeleteOverrideConfiguration() {
		Poco::File	F(DataDir_ + ExtraConfigurationFilename);

		try {
			if(F.exists())
				F.remove();
		} catch (...) {

		}
	}

}