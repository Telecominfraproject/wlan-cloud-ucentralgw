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

		enum class NotificationType {
			session_message,
			ap_disconnect
		};

		explicit SessionNotification(const std::string &Destination, const std::string &SerialNumber, const RADIUS::RadiusPacket &P)
			: Type_(NotificationType::session_message), Destination_(Destination), SerialNumber_(SerialNumber), Packet_(P) {

		}

		explicit SessionNotification(const std::string &SerialNumber)
			: Type_(NotificationType::ap_disconnect), SerialNumber_(SerialNumber) {

		}

		NotificationType			Type_;
		std::string 				Destination_;
		std::string 				SerialNumber_;
		RADIUS::RadiusPacket		Packet_;
	};

	struct RADIUSAccountingSession {
		std::uint64_t 			Started_=0;
		std::string 			Destination;
		RADIUS::RadiusPacket	Packet_;
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
		void AddSession(const std::string &Destination, const std::string &SerialNumber, const RADIUS::RadiusPacket &P) {
			SessionMessageQueue_.enqueueNotification(new SessionNotification(Destination, SerialNumber, P));
		}

	  private:
		std::atomic_bool 			Running_=false;
		Poco::NotificationQueue 	SessionMessageQueue_;
		Poco::Thread				QueueManager_;

		std::map<std::string,std::map<std::string,RADIUSAccountingSession>>	Sessions_;

		void ProcessSession(SessionNotification &Notification);
		void DisconnectSession(const std::string &SerialNumber);

		RADIUSAccountingSessionKeeper() noexcept
			: SubSystemServer("RADIUSAccountingSessionKeeper", "RADIUS-SESSION", "radius.session") {}
	};

	inline auto RADIUSAccountingSessionKeeper() { return RADIUSAccountingSessionKeeper::instance(); }

} // namespace OpenWifi

