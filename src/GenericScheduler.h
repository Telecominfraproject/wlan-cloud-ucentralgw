//
// Created by stephane bourque on 2023-04-19.
//

#pragma once

#include <framework/SubSystemServer.h>
#include <libs/Scheduler.h>
#include <Poco/Environment.h>

namespace OpenWifi {

	class GenericScheduler : public SubSystemServer {
	  public:

		static auto instance() {
			static auto instance_ = new GenericScheduler;
			return instance_;
		}

		int Start() override;
		void Stop() override;

		auto & Scheduler() { return Scheduler_; }

	  private:
		GenericScheduler() noexcept
			: SubSystemServer("Scheduler", "SCHEDULER", "scheduler"),
			  Scheduler_(Poco::Environment::processorCount()*2) {

		}
		Bosma::Scheduler	Scheduler_;
	};

	inline auto GenericScheduler() { return GenericScheduler::instance(); }

} // namespace OpenWifi
