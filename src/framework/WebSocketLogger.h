//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "framework/SubSystemServer.h"

namespace OpenWifi {

	class WebSocketLogger : public Poco::Channel {
	  public:

		inline std::string getProperty( [[maybe_unused]] const std::string &p ) const final {
			return "";
		}

		inline void close() final {
		}

		inline void open() final {
		}

		inline static std::string to_string(Poco::Message::Priority p) {
			switch(p) {
			case Poco::Message::PRIO_INFORMATION: return "information";
			case Poco::Message::PRIO_CRITICAL: return "critical";
			case Poco::Message::PRIO_DEBUG: return "debug";
			case Poco::Message::PRIO_ERROR: return "error";
			case Poco::Message::PRIO_FATAL: return "level";
			case Poco::Message::PRIO_NOTICE: return "notice";
			case Poco::Message::PRIO_TRACE: return "trace";
			case Poco::Message::PRIO_WARNING: return "warning";
			default: return "none";
			}
		}

		inline void log(const Poco::Message &m) final {
			if(Enabled_) {
				/*
				nlohmann::json log_msg;
				log_msg["msg"] = m.getText();
				log_msg["level"] = to_string(m.getPriority());
				log_msg["timestamp"] = Poco::DateTimeFormatter::format(m.getTime(), Poco::DateTimeFormat::ISO8601_FORMAT);
				log_msg["source"] = m.getSource();
				log_msg["thread_name"] = m.getThread();
				log_msg["thread_id"] = m.getTid();

				std::cout << log_msg << std::endl;
				 */
				std::lock_guard	G(Mutex_);
				std::vector<uint64_t>	Remove;
				for(const auto &[Id,CallBack]:CallBacks_) {
					try {
						CallBack(m);
					} catch (...) {
						Remove.push_back(Id);
					}
				}
				for(const auto &i:Remove)
					CallBacks_.erase(i);
			}
		}

		inline void setProperty([[maybe_unused]] const std::string &name, [[maybe_unused]] const std::string &value) final {

		}

		inline static auto instance() {
			static auto instance_ = new WebSocketLogger;
			return instance_;
		}
		inline void Enable(bool enable) { Enabled_ = enable; }
		typedef std::function<void(const Poco::Message &M)> logmuxer_callback_func_t;
		inline void RegisterCallback(const logmuxer_callback_func_t & R, uint64_t &Id) {
			std::lock_guard	G(Mutex_);
			Id = CallBackId_++;
			CallBacks_[Id] = R;
		}
	  private:
		std::recursive_mutex	Mutex_;
		std::map<uint64_t,logmuxer_callback_func_t>  CallBacks_;
		inline static uint64_t CallBackId_=1;
		bool Enabled_ = false;
	};

	inline auto WebSocketLogger() { return WebSocketLogger::instance(); }

}