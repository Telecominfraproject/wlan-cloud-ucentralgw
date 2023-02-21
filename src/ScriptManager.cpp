//
// Created by stephane bourque on 2022-11-21.
//

#include "ScriptManager.h"
#include "Poco/JSON/Parser.h"
#include "framework/MicroServiceFuncs.h"
#include <fstream>

namespace OpenWifi {

	int ScriptManager::Start() {
		poco_notice(Logger(), "Starting...");
		ScriptDir_ = MicroServiceConfigPath("script.manager.directory",
											MicroServiceDataDirectory() + "/included_scripts");
		return 0;
	}

	void ScriptManager::Stop() {
		poco_notice(Logger(), "Stopping...");
		poco_notice(Logger(), "Stopped...");
	}

} // namespace OpenWifi