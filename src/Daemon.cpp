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
#include "FindCountry.h"
#include "rttys/RTTYS_server.h"

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
										OUIServer(),
										FindCountryFromIP(),
										DeviceRegistry(),
										CommandManager(),
										FileUploader(),
										StorageArchiver(),
										TelemetryStream(),
										RTTYS_server(),
										WebSocketServer()
							   });
        return &instance;
	}

	static const std::vector<std::pair<std::string,std::string>>		DefaultDeviceTypes{
		{"cig_wf160d","AP"},
		{"cig_wf188","AP"},
		{"cig_wf188n","AP"},
		{"cig_wf194c","AP"},
		{"cig_wf194c4","AP"},
		{"edgecore_eap101","AP"},
		{"edgecore_eap102","AP"},
		{"edgecore_ecs4100-12ph","AP"},
		{"edgecore_ecw5211","AP"},
		{"edgecore_ecw5410","AP"},
		{"edgecore_oap100","AP"},
		{"edgecore_spw2ac1200","SWITCH"},
		{"edgecore_spw2ac1200-lan-poe","SWITCH"},
		{"edgecore_ssw2ac2600","SWITCH"},
		{"hfcl_ion4","AP"},
		{"indio_um-305ac","AP"},
		{"linksys_e8450-ubi","AP"},
		{"linksys_ea6350","AP"},
		{"linksys_ea6350-v4","AP"},
		{"linksys_ea8300","AP"},
		{"mikrotik_nand","AP"},
		{"tp-link_ec420-g1","AP"},
		{"tplink_cpe210_v3","AP"},
		{"tplink_cpe510_v3","AP"},
		{"tplink_eap225_outdoor_v1","AP"},
		{"tplink_ec420","AP"},
		{"tplink_ex227","AP"},
		{"tplink_ex228","AP"},
		{"tplink_ex447","AP"},
		{"wallys_dr40x9","AP"}
	};
	
	void Daemon::initialize() {
        AutoProvisioning_ = config().getBool("openwifi.autoprovisioning",false);
        DeviceTypes_ = DefaultDeviceTypes;
    }

    void MicroServicePostInitialization() {
		Daemon()->initialize();
	}

    [[nodiscard]] std::string Daemon::IdentifyDevice(const std::string & Id ) const {
	    for(const auto &[DeviceType,Type]:DeviceTypes_)
        {
        	if(Id == DeviceType)
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