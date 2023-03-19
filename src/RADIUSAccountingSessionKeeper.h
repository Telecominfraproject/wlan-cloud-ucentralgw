//
// Created by stephane bourque on 2023-03-19.
//

#pragma once

#include <framework/SubSystemServer.h>
#include <Poco/Runnable.h>
#include <Poco/Notification.h>
#include <Poco/NotificationQueue.h>

#include "RADIUS_helpers.h"

namespace OpenWifi {

	class SessionNotification : public Poco::Notification {
	  public:
		SessionNotification(const std::string &SerialNumber, const RADIUS::RadiusPacket &P)
			: SerialNumber_(SerialNumber), Packet_(P) {}
		std::string 				SerialNumber_;
		RADIUS::RadiusPacket		Packet_;
	};

	class RADIUSAccountingSessionKeeper : public SubSystemServer, Poco::Runnable {
	  public:

		static auto instance() {
			static auto instance_ = new RADIUSAccountingSessionKeeper;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void run() final;
		void AddSession(const std::string &SerialNumber, const RADIUS::RadiusPacket &P) {
			SessionMessageQueue_.enqueueNotification(new SessionNotification(SerialNumber,P));
		}

	  private:
		std::atomic_bool 			Running_=false;
		Poco::NotificationQueue 	SessionMessageQueue_;
		Poco::Thread				QueueManager_;

		RADIUSAccountingSessionKeeper() noexcept
			: SubSystemServer("RADIUSAccountingSessionKeeper", "RADIUS-SESSION", "radius.session") {}
	};

	inline auto RADIUSAccountingSessionKeeper() { return RADIUSAccountingSessionKeeper::instance(); }

} // namespace OpenWifi

