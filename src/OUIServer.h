//
// Created by stephane bourque on 2021-06-17.
//

#pragma once

#include <shared_mutex>

#include "framework/SubSystemServer.h"

#include "Poco/Timer.h"

namespace OpenWifi {

	class OUIServer : public SubSystemServer {
	  public:

		typedef std::map<uint64_t,std::string>	OUIMap;

		static auto instance() {
		    static auto instance_ = new OUIServer;
			return instance_;
		}

		int Start() override;
		void Stop() override;

		void onTimer(Poco::Timer & timer);

		void reinitialize(Poco::Util::Application &self) override;
		[[nodiscard]] std::string GetManufacturer(const std::string &MAC);
		[[nodiscard]] bool GetFile(const std::string &FileName);
		[[nodiscard]] bool ProcessFile(const std::string &FileName, OUIMap &Map);

	  private:
		std::shared_mutex	LocalMutex_;
		uint64_t 			LastUpdate_ = 0 ;
		bool 				Initialized_ = false;
		OUIMap 				OUIs_;
		volatile std::atomic_bool 	Updating_=false;
		volatile std::atomic_bool 	Running_=false;
		Poco::Timer         Timer_;
		std::unique_ptr<Poco::TimerCallback<OUIServer>>   UpdaterCallBack_;
		std::string 		LatestOUIFileName_,CurrentOUIFileName_;

		OUIServer() noexcept:
			SubSystemServer("OUIServer", "OUI-SVR", "ouiserver")
		{
		}
	};

	inline auto OUIServer() { return OUIServer::instance(); }

}

