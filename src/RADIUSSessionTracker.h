//
// Created by stephane bourque on 2023-03-19.
//

#pragma once

#include <framework/SubSystemServer.h>
#include <Poco/Runnable.h>
#include <Poco/Notification.h>
#include <Poco/NotificationQueue.h>
#include <Poco/JSON/Object.h>
#include <Poco/Timer.h>

#include "RADIUS_helpers.h"

#include <RESTObjects/RESTAPI_GWobjects.h>

namespace OpenWifi {

	class SessionNotification : public Poco::Notification {
	  public:

		enum class NotificationType {
			accounting_session_message,
			authentication_session_message,
			ap_disconnect
		};

		explicit SessionNotification(NotificationType T, const std::string &Destination, const std::string &SerialNumber, const RADIUS::RadiusPacket &P, const std::string &secret)
			: Type_(T), Destination_(Destination), SerialNumber_(SerialNumber), Packet_(P), Secret_(secret) {
		}

		explicit SessionNotification(const std::string &SerialNumber)
			: Type_(NotificationType::ap_disconnect), SerialNumber_(SerialNumber) {

		}

		NotificationType			Type_;
		std::string 				Destination_;
		std::string 				SerialNumber_;
		RADIUS::RadiusPacket		Packet_;
		std::string					Secret_;
	};

	class TrackerFutureCompletion {
	  public:
		virtual bool Completed(const RADIUS::RadiusPacket &P) = 0;
		virtual bool StillValid() = 0;
	  private:
	};

	class CoADisconnectResponse : public TrackerFutureCompletion {
	  public:
		CoADisconnectResponse(const std::string &serialNumber, std::uint8_t id, const std::vector<std::uint8_t> &types, const std::string &callingStationId):
 			SerialNumber_(serialNumber),
			Id_(id),
			PacketTypes_(types),
			CallingStationId_(callingStationId) {
			Created_ = Utils::Now();
		}

		bool Completed(const RADIUS::RadiusPacket &P) final {
			if(P.Identifier()==Id_) {
				if(P.P_.code == RADIUS::Disconnect_ACK) {

				} else if (P.P_.code == RADIUS::Disconnect_NAK) {

				}
			}
			return true;
		}

		bool StillValid() final {
			return (Utils::Now()-Created_) < 20;
		}

	  private:
		std::string 	SerialNumber_;
		std::uint8_t 	Id_;
		std::vector<std::uint8_t>	PacketTypes_;
		std::uint64_t 	Created_;
		std::string 	CallingStationId_;
	};

	using RADIUSSessionPtr = std::shared_ptr<GWObjects::RADIUSSession>;

	class RADIUSSessionTracker : public SubSystemServer, Poco::Runnable {
	  public:

		static auto instance() {
			static auto instance_ = new RADIUSSessionTracker;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void run() final;

		inline void AddAccountingSession(const std::string &Destination, const std::string &SerialNumber,
										 const RADIUS::RadiusPacket &P, const std::string &secret) {
			SessionMessageQueue_.enqueueNotification(new SessionNotification(SessionNotification::NotificationType::accounting_session_message, Destination, SerialNumber, P, secret));
		}

		inline void AddAuthenticationSession(const std::string &Destination, const std::string &SerialNumber,
											 const RADIUS::RadiusPacket &P, const std::string &secret) {
			std::lock_guard	G(Mutex_);
			auto ap_hint = AccountingSessions_.find(SerialNumber);
			if(AccountingSessions_.find(SerialNumber)!=end(AccountingSessions_)) {
				//	if we have already added the info, do not need to add it again
				auto CallingStationId = P.ExtractCallingStationID();
				auto AccountingSessionId = P.ExtractAccountingSessionID();
				if(ap_hint->second.find(CallingStationId+AccountingSessionId)!=end(ap_hint->second)) {
					return;
				}
			}
			SessionMessageQueue_.enqueueNotification(new SessionNotification(SessionNotification::NotificationType::authentication_session_message, Destination, SerialNumber, P, secret));
		}

		inline void DeviceDisconnect(const std::string &serialNumber) {
			SessionMessageQueue_.enqueueNotification(new SessionNotification(serialNumber));
		}

		inline void GetAPList(std::vector<std::string> &SerialNumbers) {
			std::lock_guard	G(Mutex_);

			for(const auto &[serialNumber,_]:AccountingSessions_) {
				SerialNumbers.emplace_back(serialNumber);
			}
		}

		inline void GetAPSessions(const std::string &SerialNumber, GWObjects::RADIUSSessionList & list) {
			std::lock_guard	G(Mutex_);

			auto ap_hint = AccountingSessions_.find(SerialNumber);
			if(ap_hint!=end(AccountingSessions_)) {
				for(const auto &[index,session]:ap_hint->second) {
					list.sessions.emplace_back(*session);
				}
			}
		}

		inline void GetUserNameAPSessions(const std::string &userName, GWObjects::RADIUSSessionList & list) {
			std::lock_guard	G(Mutex_);

			for(const auto &[_,sessions]:AccountingSessions_) {
				for(const auto &[_,session]:sessions) {
					if(Utils::match(userName.c_str(),session->userName.c_str())) {
						list.sessions.emplace_back(*session);
					}
				}
			}
		}

		inline void GetMACAPSessions(const std::string &mac, GWObjects::RADIUSSessionList & list) {
			std::lock_guard	G(Mutex_);

			for(const auto &[_,sessions]:AccountingSessions_) {
				for(const auto &[_,session]:sessions) {
					if(Utils::match(mac.c_str(),session->callingStationId.c_str())) {
						list.sessions.emplace_back(*session);
					}
				}
			}
		}

		bool SendCoADM(const std::string &serialNumber, const std::string &sessionId);
		bool SendCoADM(const RADIUSSessionPtr &session);
		bool DisconnectUser(const std::string &UserName);

		inline std::uint32_t HasSessions(const std::string & serialNumber) {
			std::lock_guard	G(Mutex_);
			auto ap_hint = AccountingSessions_.find(serialNumber);
			if(ap_hint==end(AccountingSessions_)) {
				return 0;
			}
			return ap_hint->second.size();
		}

		void GarbageCollection(Poco::Timer &timer);

	  private:
		std::atomic_bool 			Running_=false;
		Poco::NotificationQueue 	SessionMessageQueue_;
		Poco::Thread				QueueManager_;

		using SessionMap = std::map<std::string,RADIUSSessionPtr>;	//	calling-station-id + accounting-session-id
		std::map<std::string,SessionMap>		AccountingSessions_;				//	serial-number -> session< accounting-session -> session>

		Poco::Timer 												GarbageCollectionTimer_;
		std::unique_ptr<Poco::TimerCallback<RADIUSSessionTracker>> 	GarbageCollectionCallback_;

		std::uint64_t 				SessionTimeout_=10*60;


		void ProcessAccountingSession(SessionNotification &Notification);
		void ProcessAuthenticationSession(SessionNotification &Notification);
		void DisconnectSession(const std::string &SerialNumber);

		RADIUSSessionTracker() noexcept
			: SubSystemServer("RADIUSSessionTracker", "RADIUS-SESSION", "radius.session") {}

		std::string ComputeSessionIndex(RADIUSSessionPtr S);
	};

	inline auto RADIUSSessionTracker() { return RADIUSSessionTracker::instance(); }

} // namespace OpenWifi

