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
#include <cstdlib>
#include <vector>
#include <set>

#include "framework/MicroService.h"

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Crypto/RSAKey.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"

#include "Dashboard.h"
#include "framework/OpenWifiTypes.h"
#include "GwWebSocketClient.h"

namespace OpenWifi {

	[[maybe_unused]] static const char * vDAEMON_PROPERTIES_FILENAME = "owgw.properties";
	[[maybe_unused]] static const char * vDAEMON_ROOT_ENV_VAR = "OWGW_ROOT";
	[[maybe_unused]] static const char * vDAEMON_CONFIG_ENV_VAR = "OWGW_CONFIG";
	[[maybe_unused]] static const char * vDAEMON_APP_NAME = uSERVICE_GATEWAY.c_str();
	[[maybe_unused]] static const uint64_t vDAEMON_BUS_TIMER = 10000;

    class Daemon : public MicroService {
		public:
			explicit Daemon(const std::string & PropFile,
							const std::string & RootEnv,
							const std::string & ConfigEnv,
							const std::string & AppName,
						  	uint64_t 	BusTimer,
							const SubSystemVec & SubSystems) :
				MicroService( PropFile, RootEnv, ConfigEnv, AppName, BusTimer, SubSystems) {};

			bool AutoProvisioning() const { return AutoProvisioning_ ; }
			[[nodiscard]] std::string IdentifyDevice(const std::string & Compatible) const;
			static Daemon *instance();
			inline DeviceDashboard	& GetDashboard() { return DB_; }
			Poco::Logger & Log() { return Poco::Logger::get(AppName()); }
			void PostInitialization(Poco::Util::Application &self);
	  	private:
			bool                        AutoProvisioning_ = false;
			std::vector<std::pair<std::string,std::string>> DeviceTypes_;
			DeviceDashboard				DB_;
			std::unique_ptr<GwWebSocketClient>    WebSocketProcessor_;
    };

	inline Daemon * Daemon() { return Daemon::instance(); }
	inline void DaemonPostInitialization(Poco::Util::Application &self) {
		Daemon()->PostInitialization(self);
	}
}

