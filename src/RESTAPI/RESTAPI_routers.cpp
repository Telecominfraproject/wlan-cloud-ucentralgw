//
// Created by stephane bourque on 2021-10-23.
//

#include "RESTAPI/RESTAPI_blacklist.h"
#include "RESTAPI/RESTAPI_blacklist_list.h"
#include "RESTAPI/RESTAPI_capabilities_handler.h"
#include "RESTAPI/RESTAPI_command.h"
#include "RESTAPI/RESTAPI_commands.h"
#include "RESTAPI/RESTAPI_default_configuration.h"
#include "RESTAPI/RESTAPI_default_configurations.h"
#include "RESTAPI/RESTAPI_deviceDashboardHandler.h"
#include "RESTAPI/RESTAPI_device_commandHandler.h"
#include "RESTAPI/RESTAPI_device_handler.h"
#include "RESTAPI/RESTAPI_devices_handler.h"
#include "RESTAPI/RESTAPI_file.h"
#include "RESTAPI/RESTAPI_iptocountry_handler.h"
#include "RESTAPI/RESTAPI_ouis.h"
#include "RESTAPI/RESTAPI_radiusProxyConfig_handler.h"
#include "RESTAPI/RESTAPI_regulatory.h"
#include "RESTAPI/RESTAPI_script_handler.h"
#include "RESTAPI/RESTAPI_scripts_handler.h"
#include "RESTAPI/RESTAPI_telemetryWebSocket.h"
#include "RESTAPI/RESTAPI_radiussessions_handler.h"
#include "RESTAPI/RESTAPI_default_firmware.h"
#include "RESTAPI/RESTAPI_default_firmwares.h"

#include "framework/RESTAPI_SystemCommand.h"
#include "framework/RESTAPI_SystemConfiguration.h"
#include "framework/RESTAPI_WebSocketServer.h"

namespace OpenWifi {

	Poco::Net::HTTPRequestHandler *
	RESTAPI_ExtRouter(const std::string &Path, RESTAPIHandler::BindingMap &Bindings,
					  Poco::Logger &L, RESTAPI_GenericServerAccounting &S, uint64_t TransactionId) {

		return RESTAPI_Router<
			RESTAPI_devices_handler, RESTAPI_device_handler, RESTAPI_device_commandHandler,
			RESTAPI_default_configurations, RESTAPI_default_configuration, RESTAPI_command,
			RESTAPI_commands, RESTAPI_ouis, RESTAPI_file, RESTAPI_system_command,
			RESTAPI_system_configuration, RESTAPI_deviceDashboardHandler, RESTAPI_webSocketServer,
			RESTAPI_blacklist, RESTAPI_blacklist_list, RESTAPI_iptocountry_handler,
			RESTAPI_radiusProxyConfig_handler, RESTAPI_scripts_handler, RESTAPI_script_handler,
			RESTAPI_capabilities_handler, RESTAPI_telemetryWebSocket, RESTAPI_radiussessions_handler,
			RESTAPI_regulatory, RESTAPI_default_firmwares,
			RESTAPI_default_firmware>(Path, Bindings, L, S,
																	  TransactionId);
	}

	Poco::Net::HTTPRequestHandler *
	RESTAPI_IntRouter(const std::string &Path, RESTAPIHandler::BindingMap &Bindings,
					  Poco::Logger &L, RESTAPI_GenericServerAccounting &S, uint64_t TransactionId) {

		return RESTAPI_Router_I<
			RESTAPI_devices_handler, RESTAPI_device_handler, RESTAPI_device_commandHandler,
			RESTAPI_default_configurations, RESTAPI_default_configuration, RESTAPI_command,
			RESTAPI_commands, RESTAPI_ouis, RESTAPI_file, RESTAPI_blacklist,
			RESTAPI_iptocountry_handler, RESTAPI_radiusProxyConfig_handler, RESTAPI_scripts_handler,
			RESTAPI_script_handler, RESTAPI_blacklist_list, RESTAPI_radiussessions_handler,
			RESTAPI_regulatory, RESTAPI_default_firmwares,
			RESTAPI_default_firmware>(Path, Bindings, L, S, TransactionId);
	}
} // namespace OpenWifi