//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UCENTRAL_H
#define UCENTRAL_UCENTRAL_H

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

#include "uCentralTypes.h"

#include "SubSystemServer.h"

using Poco::Util::ServerApplication;

namespace uCentral {

	static const char * DAEMON_PROPERTIES_FILENAME = "ucentral.properties";
	static const char * DAEMON_ROOT_ENV_VAR = "UCENTRAL_ROOT";
	static const char * DAEMON_CONFIG_ENV_VAR = "UCENTRAL_CONFIG";
	static const char * DAEMON_APP_NAME = "uCentral";

	class MyErrorHandler : public Poco::ErrorHandler {
    public:
        void exception(const Poco::Exception & E) override;
        void exception(const std::exception & E) override;
        void exception() override;
    private:

    };

    class Daemon : public ServerApplication {

    public:
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

		std::string CreateUUID();
		[[nodiscard]] std::string IdentifyDevice(const std::string & Compatible) const;
        bool AutoProvisioning() const { return AutoProvisioning_ ; }
        bool Debug() const { return DebugMode_; }
        uint64_t ID() const { return ID_; }

		bool SetSubsystemLogLevel(const std::string & SubSystem, const std::string & Level);
		[[nodiscard]] Types::StringVec GetSubSystems() const;
		[[nodiscard]] Types::StringPairVec GetLogLevels() const;
		[[nodiscard]] const Types::StringVec & GetLogLevelNames() const;

		static std::string Version();
		const Poco::SharedPtr<Poco::Crypto::RSAKey> & Key() { return AppKey_; }
		void Exit(int Reason);
		[[nodiscard]] inline const std::string & DataDir() { return DataDir_; }

		void InitializeSubSystemServers();
		void StartSubSystemServers();
		void StopSubSystemServers();

		static Daemon *instance() {
			if (instance_ == nullptr) {
				instance_ = new Daemon;
			}
			return instance_;
		}

		uint64_t ConfigGetInt(const std::string &Key,uint64_t Default);
		uint64_t ConfigGetInt(const std::string &Key);
		std::string ConfigGetString(const std::string &Key,const std::string & Default);
		std::string ConfigGetString(const std::string &Key);
		uint64_t ConfigGetBool(const std::string &Key,bool Default);
		uint64_t ConfigGetBool(const std::string &Key);

	  private:
		static Daemon 				*instance_;
        bool                        HelpRequested_ = false;
        bool                        AutoProvisioning_ = false;
        Types::StringMapStringSet   DeviceTypeIdentifications_;
        std::string                 ConfigFileName_;
        std::string                 LogDir_;
        bool                        DebugMode_ = false;
        uint64_t                    ID_ = 1;
        Poco::UUIDGenerator         UUIDGenerator_;
        MyErrorHandler              AppErrorHandler_;
		Poco::SharedPtr<Poco::Crypto::RSAKey>	AppKey_ = nullptr;
		std::string 				DataDir_;
		Types::SubSystemVec			SubSystems_;
    };

	inline Daemon * Daemon() { return Daemon::instance(); }
}

#endif //UCENTRAL_UCENTRAL_H
