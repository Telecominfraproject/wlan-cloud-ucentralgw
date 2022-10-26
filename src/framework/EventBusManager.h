//
// Created by stephane bourque on 2022-10-26.
//

#pragma once

#include "Poco/Runnable.h"
#include "Poco/Logger.h"
#include "Poco/Thread.h"

namespace OpenWifi {

	class EventBusManager : public Poco::Runnable {
	  public:
		explicit EventBusManager(Poco::Logger &L);
		void run() final;
		void Start();
		void Stop();
		inline Poco::Logger & Logger() { return Logger_; }

	  private:
		mutable std::atomic_bool 	Running_ = false;
		Poco::Thread		Thread_;
		Poco::Logger		&Logger_;
	};

} // namespace OpenWifi

