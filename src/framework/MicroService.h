//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <ctime>


// This must be defined for poco_debug and poco_trace macros to function.

#ifndef POCO_LOG_DEBUG
#define POCO_LOG_DEBUG true
#endif

namespace OpenWifi {
    inline uint64_t Now() { return std::time(nullptr); };
}

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Crypto/RSAKey.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Process.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/JSON/Object.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/JWT/Signer.h"
#include "Poco/Environment.h"

#include "framework/OpenWifiTypes.h"

#include "framework/ow_constants.h"
#include "framework/utils.h"
#include "framework/SubSystemServer.h"
#include "framework/EventBusManager.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "cppkafka/cppkafka.h"

#include "nlohmann/json.hpp"
#include "ow_version.h"
#include "fmt/core.h"

#define _OWDEBUG_ std::cout<< __FILE__ <<":" << __LINE__ << std::endl;
// #define _OWDEBUG_ Logger().debug(Poco::format("%s: %lu",__FILE__,__LINE__));

namespace OpenWifi {

	class MicroService : public Poco::Util::ServerApplication {
	  public:
		explicit MicroService( 	std::string PropFile,
					 	std::string RootEnv,
					 	std::string ConfigVar,
					 	std::string AppName,
					  	uint64_t BusTimer,
					  	SubSystemVec Subsystems) :
				DAEMON_PROPERTIES_FILENAME(std::move(PropFile)),
				DAEMON_ROOT_ENV_VAR(std::move(RootEnv)),
				DAEMON_CONFIG_ENV_VAR(std::move(ConfigVar)),
				DAEMON_APP_NAME(std::move(AppName)),
				DAEMON_BUS_TIMER(BusTimer),
				SubSystems_(std::move(Subsystems)),
				Logger_(Poco::Logger::get("FRAMEWORK")) {
		    instance_ = this;
		    RandomEngine_.seed(std::chrono::steady_clock::now().time_since_epoch().count());
			// Logger_ = Poco::Logger::root().get("BASE-SVC");
		}

		inline static const char * ExtraConfigurationFilename = "/configuration_override.json";

		inline void SaveConfig() { PropConfigurationFile_->save(ConfigFileName_); }
		inline auto UpdateConfig() { return PropConfigurationFile_; }
		inline bool NoAPISecurity() const { return NoAPISecurity_; }
		inline Poco::ThreadPool & TimerPool() { return TimerPool_; }
		[[nodiscard]] std::string Version() { return Version_; }
		[[nodiscard]] inline const std::string & DataDir() { return DataDir_; }
		[[nodiscard]] inline const std::string & WWWAssetsDir() { return WWWAssetsDir_; }
		[[nodiscard]] bool Debug() const { return DebugMode_; }
		[[nodiscard]] uint64_t ID() const { return ID_; }
		[[nodiscard]] std::string Hash() const { return MyHash_; };
		[[nodiscard]] std::string ServiceType() const { return DAEMON_APP_NAME; };
		[[nodiscard]] std::string PrivateEndPoint() const { return MyPrivateEndPoint_; };
		[[nodiscard]] std::string PublicEndPoint() const { return MyPublicEndPoint_; };
		[[nodiscard]] const SubSystemVec & GetFullSubSystems() { return SubSystems_; }
		inline uint64_t DaemonBusTimer() const { return DAEMON_BUS_TIMER; };
        [[nodiscard]] const std::string & AppName() { return DAEMON_APP_NAME; }
		static inline uint64_t GetPID() { return Poco::Process::id(); };
		[[nodiscard]] inline const std::string GetPublicAPIEndPoint() { return MyPublicEndPoint_ + "/api/v1"; };
		[[nodiscard]] inline const std::string & GetUIURI() const { return UIURI_;};
		[[nodiscard]] inline uint64_t Random(uint64_t ceiling) { return (RandomEngine_() % ceiling); }
		[[nodiscard]] inline uint64_t Random(uint64_t min, uint64_t max) {
		    return ((RandomEngine_() % (max-min)) + min);
		}
        virtual void GetExtraConfiguration(Poco::JSON::Object & Cfg) {
            Cfg.set("additionalConfiguration",false);
        }
		static MicroService & instance() { return *instance_; }

        inline void Exit(int Reason);
		void BusMessageReceived(const std::string &Key, const std::string & Payload);
		Types::MicroServiceMetaVec GetServices(const std::string & Type);
		Types::MicroServiceMetaVec GetServices();
		void LoadConfigurationFile();
		void Reload();
		void LoadMyConfig();
		void initialize(Poco::Util::Application &self) override;
		void uninitialize() override;
		void reinitialize(Poco::Util::Application &self) override;
		void defineOptions(Poco::Util::OptionSet &options) override;
		void handleHelp(const std::string &name, const std::string &value);
		void handleVersion(const std::string &name, const std::string &value);
		void handleDebug(const std::string &name, const std::string &value);
		void handleLogs(const std::string &name, const std::string &value);
		void handleConfig(const std::string &name, const std::string &value);
		void displayHelp();
		void InitializeSubSystemServers();
		void StartSubSystemServers();
		void StopSubSystemServers();
		[[nodiscard]] static std::string CreateUUID();
		bool SetSubsystemLogLevel(const std::string &SubSystem, const std::string &Level);
		void Reload(const std::string &Sub);
		Types::StringVec GetSubSystems() const;
		Types::StringPairVec GetLogLevels();
		const Types::StringVec & GetLogLevelNames();
		uint64_t ConfigGetInt(const std::string &Key,uint64_t Default);
		uint64_t ConfigGetInt(const std::string &Key);
		uint64_t ConfigGetBool(const std::string &Key,bool Default);
		uint64_t ConfigGetBool(const std::string &Key);
		std::string ConfigGetString(const std::string &Key,const std::string & Default);
		std::string ConfigGetString(const std::string &Key);
		std::string ConfigPath(const std::string &Key,const std::string & Default);
		std::string ConfigPath(const std::string &Key);
		std::string Encrypt(const std::string &S);
		std::string Decrypt(const std::string &S);
		std::string MakeSystemEventMessage( const std::string & Type ) const;
		[[nodiscard]] bool IsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request);
		static void SavePID();
		int main(const ArgVec &args) override;
        void InitializeLoggingSystem();

		void DeleteOverrideConfiguration();

		[[nodiscard]] std::string Sign(Poco::JWT::Token &T, const std::string &Algo);
		void AddActivity(const std::string &Activity);

        static void SetConsoleLogs(bool UseAsync, bool AllowWebSocket, const std::string & FormatterPattern);
        static void SetColorConsoleLogs(bool UseAsync, bool AllowWebSocket, const std::string & FormatterPattern);
        static void SetSQLLogs(bool UseAsync, bool AllowWebSocket, const std::string & FormatterPattern);
        static void SetSyslogLogs(bool UseAsync, bool AllowWebSocket, const std::string & FormatterPattern);
        static void SetFileLogs(bool UseAsync, bool AllowWebSocket, const std::string & FormatterPattern, const std::string & root_env_var);

	  private:
	    static MicroService         * instance_;
		bool                        HelpRequested_ = false;
		std::string                 LogDir_;
		std::string                 ConfigFileName_;
		uint64_t                    ID_ = 1;
		Poco::SharedPtr<Poco::Crypto::RSAKey>	AppKey_;
		bool                        DebugMode_ = false;
		std::string 				DataDir_;
		std::string                 WWWAssetsDir_;
		Poco::Crypto::CipherFactory & CipherFactory_ = Poco::Crypto::CipherFactory::defaultFactory();
		Poco::Crypto::Cipher        * Cipher_ = nullptr;
		Types::MicroServiceMetaMap	Services_;
		std::string 				MyHash_;
		std::string 				MyPrivateEndPoint_;
		std::string 				MyPublicEndPoint_;
		std::string                 UIURI_;
		std::string 				Version_{ OW_VERSION::VERSION + "("+ OW_VERSION::BUILD + ")" + " - " + OW_VERSION::HASH };
		std::recursive_mutex		InfraMutex_;
		std::default_random_engine  RandomEngine_;
        Poco::Util::PropertyFileConfiguration   * PropConfigurationFile_ = nullptr;
		std::string                 DAEMON_PROPERTIES_FILENAME;
		std::string                 DAEMON_ROOT_ENV_VAR;
		std::string                 DAEMON_CONFIG_ENV_VAR;
		std::string                 DAEMON_APP_NAME;
		uint64_t 	                DAEMON_BUS_TIMER;
        SubSystemVec			    SubSystems_;
        bool                        NoAPISecurity_=false;
        bool                        NoBuiltInCrypto_=false;
        Poco::JWT::Signer	        Signer_;
		Poco::Logger				&Logger_;
		Poco::ThreadPool				TimerPool_{"timer:pool",2,32};
		std::unique_ptr<EventBusManager>	EventBusManager_;
    };

	inline MicroService * MicroService::instance_ = nullptr;

}
