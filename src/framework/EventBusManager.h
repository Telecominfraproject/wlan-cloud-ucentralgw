//
// Created by stephane bourque on 2022-10-26.
//

#pragma once

#include "Poco/Logger.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"

namespace OpenWifi {

	class EventBusManager : public Poco::Runnable {
	  public:
		EventBusManager() :
			Logger_(Poco::Logger::create(
				"EventBusManager", Poco::Logger::root().getChannel(), Poco::Logger::root().getLevel())) {
		}

		static auto instance() {
			static auto instance_ = new EventBusManager;
			return instance_;
		}

		void run() final;
		void Start();
		void Stop();
		inline Poco::Logger &Logger() { return Logger_; }

	  private:
		mutable std::atomic_bool Running_ = false;
		Poco::Thread Thread_;
		Poco::Logger &Logger_;
	};

	inline auto EventBusManager() { return EventBusManager::instance(); }

} // namespace OpenWifi
