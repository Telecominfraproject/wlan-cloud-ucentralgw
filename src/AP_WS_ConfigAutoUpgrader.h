//
// Created by stephane bourque on 2023-05-23.
//

#pragma once

#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Timer.h"

#include <framework/SubSystemServer.h>
#include <framework/utils.h>

namespace OpenWifi {

	class CheckConfiguration : public Poco::Notification {
	  public:
		explicit CheckConfiguration(std::uint64_t s, std::uint64_t c) :
 			serial_(s), uuid_(c) {
		}
		std::uint64_t serial_;
		std::uint64_t uuid_;
	};

	struct ConfigurationCacheEntry {
		std::uint64_t last_check_=0;
		std::uint64_t current_config_=0;
		std::uint64_t pending_config_=0;
	};

	class AP_WS_ConfigAutoUpgradeAgent : public SubSystemServer, Poco::Runnable {
	  public:
		int Start() final;
		void Stop() final;
		void run() final;

		static auto instance() {
			static auto instance = new AP_WS_ConfigAutoUpgradeAgent;
			return instance;
		}

		inline void AddConfiguration(std::uint64_t serial, std::uint64_t config_uuid) {
			std::lock_guard			Guard(CacheMutex_);
			auto hint = Cache_.find(serial);
			if(hint==end(Cache_)) {
				Cache_[serial] = { Utils::Now(),config_uuid , 0 };
				return;
			}
			if(hint->second.pending_config_==0) {
				hint->second.last_check_ = Utils::Now();
				hint->second.current_config_ = config_uuid;
				return;
			}
		}

		inline void AddConfiguration(std::uint64_t serial, std::uint64_t config_uuid, std::uint64_t pending_config_uuid) {
			std::lock_guard			Guard(CacheMutex_);
			auto hint = Cache_.find(serial);
			if(hint==end(Cache_)) {
				Cache_[serial] = { Utils::Now(), config_uuid , pending_config_uuid };
				return;
			}
			if(hint->second.pending_config_==0) {
				hint->second.last_check_ = Utils::Now();
				hint->second.current_config_ = config_uuid;
				hint->second.pending_config_ = pending_config_uuid;
				return;
			}
		}

		[[nodiscard]] inline ConfigurationCacheEntry GetSerialInfo(std::uint64_t serial) const {
			std::lock_guard			Guard(CacheMutex_);
			auto hint = Cache_.find(serial);
			if(hint==end(Cache_)) {
				return {0,0,0};
			}
			return hint->second;
		}

		inline bool UpdateConfiguration(std::uint64_t serial, std::uint64_t config) {

			if(serial==0)
				return false;

			std::lock_guard			Guard(CacheMutex_);
			auto hint = Cache_.find(serial);
			if(hint!=end(Cache_)) {

				if(hint->second.current_config_==config) {
					return false;
				}

				if(config==hint->second.pending_config_) {
					Queue_.enqueueNotification(new CheckConfiguration(serial,config));
					return true;
				}

				if(config!=hint->second.current_config_ && hint->second.pending_config_==0) {
					Queue_.enqueueNotification(new CheckConfiguration(serial,config));
					return true;
				}

				if((Utils::Now()-hint->second.last_check_)<60*5) {
					return false;
				}

				if(hint->second.pending_config_!=0) {
					return false;
				}

			}
			return true;
		}

		inline void SetDeviceCacheEntry(std::uint64_t serial, std::uint64_t t, std::uint64_t uuid, std::uint64_t pending_uuid) {
			std::lock_guard			Guard(CacheMutex_);
			Cache_[serial] = { t, uuid, pending_uuid };
		}


	  private:
		Poco::NotificationQueue		Queue_;
		Poco::Thread				QueueManager_;
		std::atomic_bool 			Running_=false;

		mutable std::mutex			CacheMutex_;
		std::map<std::uint64_t, ConfigurationCacheEntry> Cache_;

		AP_WS_ConfigAutoUpgradeAgent() noexcept
			: SubSystemServer("AutoConfigUpgrade", "AUTO-CFG-MGR", "auto.config.updater") {
		}
	};

	inline auto AP_WS_ConfigAutoUpgradeAgent() { return AP_WS_ConfigAutoUpgradeAgent::instance(); }

} // namespace OpenWifi

