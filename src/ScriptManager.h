//
// Created by stephane bourque on 2022-11-21.
//

#pragma once

#include "framework/SubSystemServer.h"

namespace OpenWifi {

	class ScriptManager : public SubSystemServer {
	  public:
		static auto instance() {
			auto static instance_ = new ScriptManager;
			return instance_;
		}

		int Start();
		void Stop();

	  private:
		std::string ScriptDir_;

		explicit ScriptManager() noexcept
			: SubSystemServer("ScriptManager", "SCRIPT-MGR", "script.manager") {}
	};

	inline auto ScriptManager() { return ScriptManager::instance(); }

} // namespace OpenWifi
