//
// Created by stephane bourque on 2022-02-03.
//

#pragma once

#include <string>
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Environment.h"

namespace OpenWifi {
	class ReactorThreadPool {
	  public:
		explicit ReactorThreadPool() {
			if(Poco::Environment::processorCount()>8)
				NumberOfThreads_ = Poco::Environment::processorCount()/2;
			else
				NumberOfThreads_ = 2;
			Start("ReactorThreadPool");
		}

		~ ReactorThreadPool() {
			Stop();
		}

		void Start(const std::string & ThreadNamePrefix) {
			for (uint64_t i = 0; i < NumberOfThreads_; ++i) {
				auto NewReactor = std::make_unique<Poco::Net::SocketReactor>();
				auto NewThread = std::make_unique<Poco::Thread>();
				NewThread->start(*NewReactor);
				NewThread->setName(ThreadNamePrefix + "#" + std::to_string(i));
				Reactors_.emplace_back(std::move(NewReactor));
				Threads_.emplace_back(std::move(NewThread));
			}
		}

		inline static auto instance() {
			static auto instance_ = new ReactorThreadPool;
			return instance_;
		}

		void Stop() {
			for (auto &i : Reactors_)
				i->stop();
			for (auto &i : Threads_) {
				i->join();
			}
		}

		Poco::Net::SocketReactor &NextReactor() {
			std::lock_guard		G(Mutex_);
			NextReactor_++;
			NextReactor_ %= NumberOfThreads_;
			return *Reactors_[NextReactor_];
		}

	  private:
		std::mutex			Mutex_;
		uint64_t 			NumberOfThreads_;
		uint64_t 			NextReactor_ = 0;
		std::vector<std::unique_ptr<Poco::Net::SocketReactor>> Reactors_;
		std::vector<std::unique_ptr<Poco::Thread>> Threads_;
	};
	inline auto ReactorThreadPool() { return ReactorThreadPool::instance(); }
}