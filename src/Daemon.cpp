//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <boost/algorithm/string.hpp>
#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Environment.h"

#include "CentralConfig.h"
#include "CommandManager.h"
#include "Daemon.h"
#include "DeviceRegistry.h"
#include "FileUploader.h"
#include "OUIServer.h"
#include "SerialNumberCache.h"
#include "StorageArchiver.h"
#include "StorageService.h"
#include "TelemetryStream.h"
#include "WebSocketServer.h"
#include "framework/ConfigurationValidator.h"
#include "framework/MicroService.h"

namespace OpenWifi {
	class Daemon *Daemon::instance() {
	    static Daemon instance(vDAEMON_PROPERTIES_FILENAME,
								   vDAEMON_ROOT_ENV_VAR,
								   vDAEMON_CONFIG_ENV_VAR,
								   vDAEMON_APP_NAME,
								   vDAEMON_BUS_TIMER,
								   SubSystemVec{
									   StorageService(),
									   SerialNumberCache(),
									   ConfigurationValidator(),
									   DeviceRegistry(),
									   WebSocketServer(),
									   CommandManager(),
									   FileUploader(),
									   OUIServer(),
									   StorageArchiver(),
									   TelemetryStream()
								   });
        return &instance;
	}

	void Daemon::initialize() {
		Config::Config::Init();
        AutoProvisioning_ = config().getBool("openwifi.autoprovisioning",false);

        // DeviceTypeIdentifications_
        Types::StringVec   Keys;
        config().keys("openwifi.devicetypes",Keys);
        for(const auto & i:Keys)
        {
        	std::string Line = config().getString("openwifi.devicetypes."+i);
            auto P1 = Line.find_first_of(':');
            auto Type = Line.substr(0, P1);
            auto List = Line.substr(P1+1);

            Types::StringVec  Tokens = Utils::Split(List);

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

    void MicroServicePostInitialization() {
		Daemon()->initialize();
	}

    [[nodiscard]] std::string Daemon::IdentifyDevice(const std::string & Id ) const {
        for(const auto &[Type,List]:DeviceTypeIdentifications_)
        {
			if(List.find(Id)!=List.end())
				return Type;
        }
        return "AP";
    }
}


int main(int argc, char **argv) {
	try {

		auto App = OpenWifi::Daemon::instance();
		auto ExitCode =  App->run(argc, argv);
		return ExitCode;

	} catch (Poco::Exception &exc) {
		std::cerr << exc.displayText() << std::endl;
		return Poco::Util::Application::EXIT_SOFTWARE;
	}
}

// end of namespace