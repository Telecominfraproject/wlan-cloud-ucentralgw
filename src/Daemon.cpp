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
#include "Poco/Net/HTTPStreamFactory.h"

#include "Daemon.h"

#include "CommandManager.h"
#include "DeviceRegistry.h"
#include "FileUploader.h"
#include "RESTAPI_server.h"
#include "StorageService.h"
#include "WebSocketServer.h"
#include "CentralConfig.h"
#include "OUIServer.h"
#include "StateProcessor.h"
#include "Utils.h"
#include "RESTAPI_InternalServer.h"
#include "AuthClient.h"
#include "StorageArchiver.h"
#include "SerialNumberCache.h"
#include "TelemetryStream.h"
#include "ConfigurationValidator.h"

namespace OpenWifi {
	class Daemon *Daemon::instance_ = nullptr;

	class Daemon *Daemon::instance() {
		if (instance_ == nullptr) {
			instance_ = new Daemon(vDAEMON_PROPERTIES_FILENAME,
								   vDAEMON_ROOT_ENV_VAR,
								   vDAEMON_CONFIG_ENV_VAR,
								   vDAEMON_APP_NAME,
								   vDAEMON_BUS_TIMER,
								   Types::SubSystemVec{
									   Storage(),
									   SerialNumberCache(),
									   ConfigurationValidator(),
									   AuthClient(),
									   DeviceRegistry(),
									   RESTAPI_server(),
									   RESTAPI_InternalServer(),
									   WebSocketServer(),
									   CommandManager(),
									   FileUploader(),
									   OUIServer(),
									   StorageArchiver(),
									   TelemetryStream()
								   });
		}
		return instance_;
	}

	void Daemon::initialize(Poco::Util::Application &self) {
		MicroService::initialize(*this);
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
		delete App;

		return ExitCode;

	} catch (Poco::Exception &exc) {
		std::cerr << exc.displayText() << std::endl;
		return Poco::Util::Application::EXIT_SOFTWARE;
	}
}

// end of namespace