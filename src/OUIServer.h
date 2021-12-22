//
// Created by stephane bourque on 2021-06-17.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

	class OUIServer : public SubSystemServer, Poco::Runnable {
	  public:

		typedef std::map<uint64_t,std::string>	OUIMap;

		static auto instance() {
		    static auto instance_ = new OUIServer;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void run() override;

		void reinitialize(Poco::Util::Application &self) override;

		void Update();
		void UpdateImpl();
		[[nodiscard]] std::string GetManufacturer(const std::string &MAC);
		[[nodiscard]] bool GetFile(const std::string &FileName);
		[[nodiscard]] bool ProcessFile(const std::string &FileName, OUIMap &Map);

	  private:
		uint64_t 			LastUpdate_ = 0 ;
		bool 				ValidFile_=false;
		OUIMap 				OUIs_;
		Poco::Thread		UpdaterThread_;
		std::atomic_bool 	Updating_=false;
		std::atomic_bool 	Running_=false;

		OUIServer() noexcept:
			SubSystemServer("OUIServer", "OUI-SVR", "ouiservr")
		{
		}
	};

	inline auto OUIServer() { return OUIServer::instance(); }

}

