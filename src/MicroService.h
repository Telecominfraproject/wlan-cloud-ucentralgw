//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
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
#include "Poco/SHA2Engine.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Process.h"

#include "OpenWifiTypes.h"
#include "SubSystemServer.h"

namespace OpenWifi {

	static const std::string uSERVICE_SECURITY{"owsec"};
	static const std::string uSERVICE_GATEWAY{"owgw"};
	static const std::string uSERVICE_FIRMWARE{ "owfms"};
    static const std::string uSERVICE_TOPOLOGY{ "owtopo"};
    static const std::string uSERVICE_PROVISIONING{ "owprov"};
    static const std::string uSERVICE_OWLS{ "owls"};

	class MyErrorHandler : public Poco::ErrorHandler {
	  public:
		explicit MyErrorHandler(Poco::Util::Application &App) : App_(App) {}
		void exception(const Poco::Exception & E) override;
		void exception(const std::exception & E) override;
		void exception() override;
	  private:
		Poco::Util::Application	&App_;
	};

	class BusEventManager : public Poco::Runnable {
	  public:
		void run() override;
		void Start();
		void Stop();
	  private:
		std::atomic_bool 	Running_ = false;
		Poco::Thread		Thread_;
	};

	struct MicroServiceMeta {
		uint64_t 		Id=0;
		std::string 	Type;
		std::string 	PrivateEndPoint;
		std::string 	PublicEndPoint;
		std::string 	AccessKey;
		std::string		Version;
		uint64_t 		LastUpdate=0;
	};

	typedef std::map<uint64_t, MicroServiceMeta>	MicroServiceMetaMap;
	typedef std::vector<MicroServiceMeta>			MicroServiceMetaVec;

	class MicroService : public Poco::Util::ServerApplication {
	  public:
		explicit MicroService( 	std::string PropFile,
					 	std::string RootEnv,
					 	std::string ConfigVar,
					 	std::string AppName,
					  	uint64_t BusTimer,
					  	Types::SubSystemVec Subsystems) :
			DAEMON_PROPERTIES_FILENAME(std::move(PropFile)),
			DAEMON_ROOT_ENV_VAR(std::move(RootEnv)),
			DAEMON_CONFIG_ENV_VAR(std::move(ConfigVar)),
			DAEMON_APP_NAME(std::move(AppName)),
			DAEMON_BUS_TIMER(BusTimer),
			SubSystems_(std::move(Subsystems)) {
		}

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
		[[nodiscard]] std::string Version() { return Version_; }
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
		[[nodiscard]] std::string CreateHash(const std::string &S);
		[[nodiscard]] std::string Hash() const { return MyHash_; };
		[[nodiscard]] std::string ServiceType() const { return DAEMON_APP_NAME; };
		[[nodiscard]] std::string PrivateEndPoint() const { return MyPrivateEndPoint_; };
		[[nodiscard]] std::string PublicEndPoint() const { return MyPublicEndPoint_; };
		[[nodiscard]] std::string MakeSystemEventMessage( const std::string & Type ) const ;
		inline uint64_t DaemonBusTimer() const { return DAEMON_BUS_TIMER; };

		void BusMessageReceived( const std::string & Key, const std::string & Message);
		[[nodiscard]] MicroServiceMetaVec GetServices(const std::string & type);
		[[nodiscard]] MicroServiceMetaVec GetServices();
		[[nodiscard]] bool IsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request);

		static void SavePID();
		static inline uint64_t GetPID() { return Poco::Process::id(); };
		[[nodiscard]] inline const std::string GetPublicAPIEndPoint() { return MyPublicEndPoint_ + "/api/v1"; };
		[[nodiscard]] inline const std::string & GetUIURI() const { return UIURI_;};

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
		Poco::SHA2Engine			SHA2_;
		MicroServiceMetaMap			Services_;
		std::string 				MyHash_;
		std::string 				MyPrivateEndPoint_;
		std::string 				MyPublicEndPoint_;
		std::string                 UIURI_;
		std::string 				Version_{std::string(APP_VERSION) + "("+ BUILD_NUMBER + ")"};
		BusEventManager				BusEventManager_;
		SubMutex 					InfraMutex_;

		std::string DAEMON_PROPERTIES_FILENAME;
		std::string DAEMON_ROOT_ENV_VAR;
		std::string DAEMON_CONFIG_ENV_VAR;
		std::string DAEMON_APP_NAME;
		uint64_t 	DAEMON_BUS_TIMER;
	};
}

#endif // UCENTRALGW_MICROSERVICE_H
