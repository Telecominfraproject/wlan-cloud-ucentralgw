//
// Created by stephane bourque on 2022-02-03.
//

#pragma once

#include <mutex>
#include <string>

#include "Poco/Environment.h"
#include "Poco/Net/SocketAcceptor.h"
#include <Poco/Data/SessionPool.h>
#include "framework/utils.h"
#include <StorageService.h>

namespace OpenWifi {

	class AP_WS_ReactorThreadPool {
	  public:
		explicit AP_WS_ReactorThreadPool(Poco::Logger &Logger) : Logger_(Logger) {
			NumberOfThreads_ = Poco::Environment::processorCount() * 4;
			if (NumberOfThreads_ == 0)
				NumberOfThreads_ = 8;
			NumberOfThreads_ = std::min(NumberOfThreads_, (std::uint64_t) 128);
		}

		~AP_WS_ReactorThreadPool() { Stop(); }

		void Start() {
			Reactors_.reserve(NumberOfThreads_);
			DbSessions_.reserve(NumberOfThreads_);
			Threads_.reserve(NumberOfThreads_);
			Logger_.information(fmt::format("WebSocket Processor: starting {} threads.", NumberOfThreads_));
			for (uint64_t i = 0; i < NumberOfThreads_; ++i) {
				auto NewReactor = std::make_shared<Poco::Net::SocketReactor>();
				auto NewThread = std::make_unique<Poco::Thread>();
				NewThread->start(*NewReactor);
				std::string ThreadName{"ap:react:" + std::to_string(i)};
				Utils::SetThreadName(*NewThread, ThreadName.c_str());
				Reactors_.emplace_back(std::move(NewReactor));
				Threads_.emplace_back(std::move(NewThread));
				DbSessions_.emplace_back(std::make_shared<LockedDbSession>());
			}
			Logger_.information(fmt::format("WebSocket Processor: {} threads started.", NumberOfThreads_));
		}

		void Stop() {
			for (auto &i : Reactors_)
				i->stop();
			for (auto &i : Threads_) {
				i->join();
			}
			Reactors_.clear();
			Threads_.clear();
			DbSessions_.clear();
		}

		std::pair<std::shared_ptr<Poco::Net::SocketReactor>, std::shared_ptr<LockedDbSession> > NextReactor() {
			std::lock_guard Lock(Mutex_);
			NextReactor_++;
			NextReactor_ %= NumberOfThreads_;
			return std::make_pair(Reactors_[NextReactor_], DbSessions_[NextReactor_]);
		}

	  private:
		std::mutex Mutex_;
		uint64_t NumberOfThreads_;
		uint64_t NextReactor_ = 0;
		std::vector<std::shared_ptr<Poco::Net::SocketReactor>> 	Reactors_;
		std::vector<std::unique_ptr<Poco::Thread>> 				Threads_;
		std::vector<std::shared_ptr<LockedDbSession>>			DbSessions_;
		Poco::Logger &Logger_;

	};
} // namespace OpenWifi