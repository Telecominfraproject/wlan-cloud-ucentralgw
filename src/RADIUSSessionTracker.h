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
			accounting_session_message,
			authentication_session_message,
			ap_disconnect
		};

		explicit  SessionNotification(NotificationType T, const std::string &Destination, const std::string &SerialNumber, const RADIUS::RadiusPacket &P)
			: Type_(T), Destination_(Destination), SerialNumber_(SerialNumber), Packet_(P) {
		}

		explicit SessionNotification(const std::string &SerialNumber)
			: Type_(NotificationType::ap_disconnect), SerialNumber_(SerialNumber) {

		}

		NotificationType			Type_;
		std::string 				Destination_;
		std::string 				SerialNumber_;
		RADIUS::RadiusPacket		Packet_;
	};


	struct RADIUSSession {
		std::uint64_t 			Started_=0,
								LastTransaction_=0;
		std::string 			Destination_;
		std::string 			UserName_;
		RADIUS::RadiusPacket	AccountingPacket_;
		std::string 			SessionId_;
		std::string 			MultiSessionId_;
	};

	class RADIUSSessionTracker : public SubSystemServer, Poco::Runnable {
	  public:

		static auto instance() {
			static auto instance_ = new RADIUSSessionTracker;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void run() final;

		inline void AddAccountingSession(const std::string &Destination, const std::string &SerialNumber, const RADIUS::RadiusPacket &P) {
			SessionMessageQueue_.enqueueNotification(new SessionNotification(SessionNotification::NotificationType::accounting_session_message, Destination, SerialNumber, P));
		}

		inline void AddAuthenticationSession(const std::string &Destination, const std::string &SerialNumber, const RADIUS::RadiusPacket &P) {
			SessionMessageQueue_.enqueueNotification(new SessionNotification(SessionNotification::NotificationType::authentication_session_message, Destination, SerialNumber, P));
		}

		inline void DeviceDisconnect(const std::string &serialNumber) {
			SessionMessageQueue_.enqueueNotification(new SessionNotification(serialNumber));
		}

	  private:
		std::atomic_bool 			Running_=false;
		Poco::NotificationQueue 	SessionMessageQueue_;
		Poco::Thread				QueueManager_;

		using SessionMap = std::map<std::string,std::shared_ptr<RADIUSSession>>;
		std::map<std::string,SessionMap>		AuthenticationSessions_;			//	serial-number -> session< username -> session >
		std::map<std::string,SessionMap>		AccountingSessions_;				//	serial-number -> session< accounting-session -> session>

		void ProcessAccountingSession(SessionNotification &Notification);
		void ProcessAuthenticationSession(SessionNotification &Notification);
		void DisconnectSession(const std::string &SerialNumber);

		RADIUSSessionTracker() noexcept
			: SubSystemServer("RADIUSSessionTracker", "RADIUS-SESSION", "radius.session") {}
	};

	inline auto RADIUSSessionTracker() { return RADIUSSessionTracker::instance(); }

} // namespace OpenWifi

