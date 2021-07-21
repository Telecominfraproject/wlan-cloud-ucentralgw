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
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"

#include "Dashboard.h"
#include "MicroService.h"
#include "uCentralTypes.h"

namespace uCentral {

	static const char * vDAEMON_PROPERTIES_FILENAME = "ucentralgw.properties";
	static const char * vDAEMON_ROOT_ENV_VAR = "UCENTRALGW_ROOT";
	static const char * vDAEMON_CONFIG_ENV_VAR = "UCENTRALGW_CONFIG";
	static const char * vDAEMON_APP_NAME = uSERVICE_GATEWAY.c_str();
	static const uint64_t vDAEMON_BUS_TIMER = 10000;

    class Daemon : public MicroService {
		public:
			explicit Daemon(std::string PropFile,
							std::string RootEnv,
							std::string ConfigEnv,
							std::string AppName,
						  	uint64_t 	BusTimer,
							Types::SubSystemVec SubSystems) :
				MicroService( PropFile, RootEnv, ConfigEnv, AppName, BusTimer, SubSystems) {};

			bool AutoProvisioning() const { return AutoProvisioning_ ; }
			[[nodiscard]] std::string IdentifyDevice(const std::string & Compatible) const;
			void initialize(Poco::Util::Application &self);
			static Daemon *instance();
			inline DeviceDashboard	& GetDashboard() { return DB_; }
	  	private:
			static Daemon 				*instance_;
			bool                        AutoProvisioning_ = false;
			Types::StringMapStringSet   DeviceTypeIdentifications_;
			DeviceDashboard				DB_;

    };

	inline Daemon * Daemon() { return Daemon::instance(); }
}

#endif //UCENTRAL_UCENTRAL_H
