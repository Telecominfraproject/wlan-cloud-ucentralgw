//
// Created by stephane bourque on 2022-02-03.
//

#pragma once

#include <string>
#include <shared_mutex>

#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Environment.h"

namespace OpenWifi {
	class AP_WS_ReactorThreadPool {
	  public:
		explicit AP_WS_ReactorThreadPool() {
			NumberOfThreads_ = Poco::Environment::processorCount()*2;
			if(NumberOfThreads_==0)
				NumberOfThreads_=4;
		}

		~ AP_WS_ReactorThreadPool() {
			Stop();
		}

		void Start() {
			for (uint64_t i = 0; i < NumberOfThreads_; ++i) {
				auto NewReactor = std::make_unique<Poco::Net::SocketReactor>();
				auto NewThread = std::make_unique<Poco::Thread>();
				NewThread->start(*NewReactor);
				std::string ThreadName{"ap:react:" + std::to_string(i)};
				Utils::SetThreadName(*NewThread,ThreadName.c_str());
				Reactors_.emplace_back(std::move(NewReactor));
				Threads_.emplace_back(std::move(NewThread));
			}
		}

		void Stop() {
			for (auto &i : Reactors_)
				i->stop();
			for (auto &i : Threads_) {
				i->join();
			}
			Reactors_.clear();
			Threads_.clear();
		}

		Poco::Net::SocketReactor &NextReactor() {
			std::shared_lock		Lock(Mutex_);
			NextReactor_++;
			NextReactor_ %= NumberOfThreads_;
			return *Reactors_[NextReactor_];
		}

	  private:
		std::shared_mutex	Mutex_;
		uint64_t 			NumberOfThreads_;
		uint64_t 			NextReactor_ = 0;
		std::vector<std::unique_ptr<Poco::Net::SocketReactor>> Reactors_;
		std::vector<std::unique_ptr<Poco::Thread>> Threads_;
	};
}