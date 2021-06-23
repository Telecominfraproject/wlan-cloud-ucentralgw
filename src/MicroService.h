//
// Created by stephane bourque on 2021-06-22.
//

#ifndef UCENTRALGW_MICROSERVICE_H
#define UCENTRALGW_MICROSERVICE_H

#include <array>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Crypto/RSAKey.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"

#include "uCentralTypes.h"
#include "SubSystemServer.h"

namespace uCentral {

	class MyErrorHandler : public Poco::ErrorHandler {
	  public:
		explicit MyErrorHandler(Poco::Util::Application &App) : App_(App) {}
		void exception(const Poco::Exception & E) override;
		void exception(const std::exception & E) override;
		void exception() override;
	  private:
		Poco::Util::Application	&App_;
	};

	class MicroService : public Poco::Util::ServerApplication {
	  public:
		explicit MicroService( 	std::string PropFile,
					 	std::string RootEnv,
					 	std::string ConfigVar,
					 	std::string AppName,
					  	Types::SubSystemVec Subsystems) :
			DAEMON_PROPERTIES_FILENAME(std::move(PropFile)),
			DAEMON_ROOT_ENV_VAR(std::move(RootEnv)),
			DAEMON_CONFIG_ENV_VAR(std::move(ConfigVar)),
			DAEMON_APP_NAME(std::move(AppName)),
			SubSystems_(Subsystems) {}

		int main(const ArgVec &args) override;
		void initialize(Application &self) override;
		void uninitialize() override;
		void reinitialize(Application &self) override;
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
		void Exit(int Reason);
		bool SetSubsystemLogLevel(const std::string & SubSystem, const std::string & Level);
		[[nodiscard]] static std::string Version();
		[[nodiscard]] const Poco::SharedPtr<Poco::Crypto::RSAKey> & Key() { return AppKey_; }
		[[nodiscard]] inline const std::string & DataDir() { return DataDir_; }
		[[nodiscard]] std::string CreateUUID();
		[[nodiscard]] bool Debug() const { return DebugMode_; }
		[[nodiscard]] uint64_t ID() const { return ID_; }
		[[nodiscard]] Types::StringVec GetSubSystems() const;
		[[nodiscard]] Types::StringPairVec GetLogLevels() const;
		[[nodiscard]] const Types::StringVec & GetLogLevelNames() const;
		[[nodiscard]] std::string ConfigGetString(const std::string &Key,const std::string & Default);
		[[nodiscard]] std::string ConfigGetString(const std::string &Key);
		[[nodiscard]] std::string ConfigPath(const std::string &Key,const std::string & Default);
		[[nodiscard]] std::string ConfigPath(const std::string &Key);
		[[nodiscard]] uint64_t ConfigGetInt(const std::string &Key,uint64_t Default);
		[[nodiscard]] uint64_t ConfigGetInt(const std::string &Key);
		[[nodiscard]] uint64_t ConfigGetBool(const std::string &Key,bool Default);
		[[nodiscard]] uint64_t ConfigGetBool(const std::string &Key);
		[[nodiscard]] std::string Encrypt(const std::string &S);
		[[nodiscard]] std::string Decrypt(const std::string &S);

	  private:
		bool                        HelpRequested_ = false;
		std::string                 LogDir_;
		std::string                 ConfigFileName_;
		Poco::UUIDGenerator         UUIDGenerator_;
		uint64_t                    ID_ = 1;
		Poco::SharedPtr<Poco::Crypto::RSAKey>	AppKey_ = nullptr;
		bool                        DebugMode_ = false;
		std::string 				DataDir_;
		Types::SubSystemVec			SubSystems_;
		Poco::Crypto::CipherFactory & CipherFactory_ = Poco::Crypto::CipherFactory::defaultFactory();
		Poco::Crypto::Cipher        * Cipher_ = nullptr;

		std::string DAEMON_PROPERTIES_FILENAME;
		std::string DAEMON_ROOT_ENV_VAR;
		std::string DAEMON_CONFIG_ENV_VAR;
		std::string DAEMON_APP_NAME;
	};
}

#endif // UCENTRALGW_MICROSERVICE_H
