//
// Created by stephane bourque on 2023-03-19.
//

#include "RADIUSSessionTracker.h"
#include <fmt/format.h>
#include <framework/utils.h>

#include "RADIUS_proxy_server.h"
#include "framework/RESTAPI_utils.h"

using OpenWifi::RESTAPI_utils::field_from_json;
using OpenWifi::RESTAPI_utils::field_to_json;

namespace OpenWifi {

	int RADIUSSessionTracker::Start() {
		poco_information(Logger(),"Starting...");
		QueueManager_.start(*this);
		return 0;
	}

	void RADIUSSessionTracker::Stop() {
		poco_information(Logger(),"Stopping...");
		Running_ = false;
		SessionMessageQueue_.wakeUpAll();
		QueueManager_.join();
		poco_information(Logger(),"Stopped...");
	}

	void RADIUSSessionTracker::run() {
		Utils::SetThreadName("rad:sessmgr");
		Running_ = true;

		Poco::AutoPtr<Poco::Notification> NextSession(SessionMessageQueue_.waitDequeueNotification());
		while (NextSession && Running_) {
			auto Session = dynamic_cast<SessionNotification *>(NextSession.get());

			try {
				if (Session != nullptr) {
					switch(Session->Type_) {
						case SessionNotification::NotificationType::accounting_session_message: {
							ProcessAccountingSession(*Session);
						} break;
						case SessionNotification::NotificationType::authentication_session_message: {
							ProcessAuthenticationSession(*Session);
						} break;
						case SessionNotification::NotificationType::ap_disconnect: {
							DisconnectSession(Session->SerialNumber_);
						} break;
					}
				}
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			} catch (...) {
				poco_warning(Logger(), "Exception occurred during run.");
			}
			NextSession = SessionMessageQueue_.waitDequeueNotification();
		}
		poco_information(Logger(), "RADIUS session manager stopping.");
	}

	void RADIUSSessionTracker::ProcessAuthenticationSession([[maybe_unused]] OpenWifi::SessionNotification &Notification) {
		std::lock_guard Guard(Mutex_);

		std::string CallingStationId, AccountingSessionId, AccountingMultiSessionId, UserName;
		for (const auto &attribute : Notification.Packet_.Attrs_) {
			switch (attribute.type) {
			case RADIUS::AUTH_USERNAME: {
				UserName.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::CALLING_STATION_ID: {
				CallingStationId.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::ACCT_SESSION_ID: {
				AccountingSessionId.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::ACCT_MULTI_SESSION_ID: {
				AccountingMultiSessionId.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			default: {
			} break;
			}
		}

		auto ap_hint = AccountingSessions_.find(Notification.SerialNumber_);
		if(ap_hint==end(AccountingSessions_)) {
			SessionMap M;
			AccountingSessions_[Notification.SerialNumber_ ] = M;
			ap_hint = AccountingSessions_.find(Notification.SerialNumber_);
		}

		auto Index = CallingStationId+AccountingSessionId+AccountingMultiSessionId;
		auto session_hint = ap_hint->second.find(Index);
		if(session_hint==end(ap_hint->second)) {
			auto NewSession = std::make_shared<GWObjects::RADIUSSession>();
			NewSession->Started_ = NewSession->LastTransaction_ = Utils::Now();
			NewSession->UserName_ = UserName;
			NewSession->CallingStationId_ = CallingStationId;
			NewSession->AccountingSessionId_ = AccountingSessionId;
			NewSession->AccountingMultiSessionId_ = AccountingMultiSessionId;
			ap_hint->second[Index] = NewSession;
		} else {
			session_hint->second->LastTransaction_ = Utils::Now();
		}

	}

	std::uint32_t GetUiInt32(const std::uint8_t *buf) {
		return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
	}

	void
	RADIUSSessionTracker::ProcessAccountingSession(OpenWifi::SessionNotification &Notification) {
		std::lock_guard     Guard(Mutex_);

		std::string CallingStationId, AccountingSessionId, AccountingMultiSessionId, UserName;
		std::uint8_t AccountingPacketType = 0;
		std::uint32_t InputOctets=0, OutputOctets=0, InputPackets=0, OutputPackets=0, InputGigaWords=0, OutputGigaWords=0,
					  SessionTime = 0;
		for (const auto &attribute : Notification.Packet_.Attrs_) {
			switch (attribute.type) {
			case RADIUS::AUTH_USERNAME: {
				UserName.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::CALLING_STATION_ID: {
				CallingStationId.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::ACCT_SESSION_ID: {
				AccountingSessionId.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::ACCT_MULTI_SESSION_ID: {
				AccountingMultiSessionId.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::ACCT_STATUS_TYPE: {
				AccountingPacketType = Notification.Packet_.P_.attributes[attribute.pos + 3];
			} break;
			case RADIUS::ACCT_INPUT_OCTETS: {
				InputOctets = GetUiInt32(&Notification.Packet_.P_.attributes[attribute.pos]);
			} break;
			case RADIUS::ACCT_INPUT_PACKETS: {
				InputPackets = GetUiInt32(&Notification.Packet_.P_.attributes[attribute.pos]);
			} break;
			case RADIUS::ACCT_INPUT_GIGAWORDS: {
				InputGigaWords = GetUiInt32(&Notification.Packet_.P_.attributes[attribute.pos]);
			} break;
			case RADIUS::ACCT_OUTPUT_OCTETS: {
				OutputOctets = GetUiInt32(&Notification.Packet_.P_.attributes[attribute.pos]);
			} break;
			case RADIUS::ACCT_OUTPUT_PACKETS: {
				OutputPackets= GetUiInt32(&Notification.Packet_.P_.attributes[attribute.pos]);
			} break;
			case RADIUS::ACCT_OUTPUT_GIGAWORDS: {
				OutputGigaWords = GetUiInt32(&Notification.Packet_.P_.attributes[attribute.pos]);
			} break;
			case RADIUS::ACCT_SESSION_TIME: {
				SessionTime = GetUiInt32(&Notification.Packet_.P_.attributes[attribute.pos]);
			} break;
			default: {
			} break;
			}
		}

		auto Index = CallingStationId+AccountingSessionId+AccountingMultiSessionId;
		auto ap_hint = AccountingSessions_.find(Notification.SerialNumber_);
		if(ap_hint==end(AccountingSessions_)) {
			SessionMap M;
			AccountingSessions_[Notification.SerialNumber_ ] = M;
			ap_hint = AccountingSessions_.find(Notification.SerialNumber_);
		}

		auto session_hint = ap_hint->second.find(Notification.SerialNumber_);
		if(session_hint==end(ap_hint->second)) {
			//  find the calling_station_id
			//  if we are getting a stop for something we do not know, nothing to do...
			if( AccountingPacketType!=OpenWifi::RADIUS::ACCT_STATUS_TYPE_START &&
				AccountingPacketType!=OpenWifi::RADIUS::ACCT_STATUS_TYPE_INTERIM_UPDATE) {
				return;
			}

			auto NewSession = std::make_shared<GWObjects::RADIUSSession>();
			NewSession->Destination_ = Notification.Destination_;
			NewSession->Started_ = NewSession->LastTransaction_ = Utils::Now();
			NewSession->UserName_ = UserName;
			NewSession->CallingStationId_ = CallingStationId;
			NewSession->AccountingSessionId_ = AccountingSessionId;
			NewSession->AccountingMultiSessionId_ = AccountingMultiSessionId;
			NewSession->AccountingPacket_ = Notification.Packet_;
			NewSession->Destination_ = Notification.Destination_;
			NewSession->InputOctets_ = InputOctets;
			NewSession->InputPackets_ = InputPackets;
			NewSession->InputGigaWords_ = InputGigaWords;
			NewSession->OutputOctets_ = OutputOctets;
			NewSession->OutputOctets_ = OutputPackets;
			NewSession->OutputGigaWords_ = OutputGigaWords;
			NewSession->SessionTime_ = SessionTime;

			poco_debug(Logger(),fmt::format("{}: Creating session", CallingStationId));
			ap_hint->second[Index] = NewSession;

		} else {

			//  If we receive a stop, just remove that session
			if(AccountingPacketType==OpenWifi::RADIUS::ACCT_STATUS_TYPE_STOP) {
				poco_debug(Logger(),fmt::format("{}: Deleting session", CallingStationId));
				ap_hint->second.erase(Index);
			} else {
				poco_debug(Logger(),fmt::format("{}: Updating session", CallingStationId));
				session_hint->second->AccountingPacket_ = Notification.Packet_;
				session_hint->second->Destination_ = Notification.Destination_;
				session_hint->second->LastTransaction_ = Utils::Now();
				session_hint->second->InputOctets_ = InputOctets;
				session_hint->second->InputPackets_ = InputPackets;
				session_hint->second->InputGigaWords_ = InputGigaWords;
				session_hint->second->OutputOctets_ = OutputOctets;
				session_hint->second->OutputOctets_ = OutputPackets;
				session_hint->second->OutputGigaWords_ = OutputGigaWords;
				session_hint->second->SessionTime_ = SessionTime;
			}
		}
	}

	[[maybe_unused]] static void store_packet(const std::string &serialNumber, const char *buffer, std::size_t size, int i) {
		static std::uint64_t pkt=0;

		std::string filename = MicroServiceDataDirectory() + "/radius." + serialNumber + ".stop." +
							   std::to_string(pkt++) + "." + std::to_string(i) + ".bin";

		std::ofstream ofs(filename,std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);
		ofs.write(buffer,size);
		ofs.close();
	}

	void RADIUSSessionTracker::DisconnectSession(const std::string &SerialNumber) {
		poco_information(Logger(),fmt::format("{}: Disconnecting.", SerialNumber));

		std::lock_guard		Guard(Mutex_);

		auto hint = AccountingSessions_.find(SerialNumber);
		if(hint==end(AccountingSessions_)) {
			return;
		}

		//	we need to go through all sessions and send an accounting stop
		for(const auto &session:hint->second) {
			poco_debug(Logger(), fmt::format("Stopping accounting for {}:{}", SerialNumber, session.first ));

			RADIUS::RadiusPacket	P(session.second->AccountingPacket_);

			P.P_.identifier++;
			P.ReplaceAttribute(RADIUS::ACCT_STATUS_TYPE, (std::uint32_t) RADIUS::ACCT_STATUS_TYPE_STOP);
			P.ReplaceOrAdd(RADIUS::EVENT_TIMESTAMP, (std::uint32_t) std::time(nullptr));
			P.AppendAttribute(RADIUS::ACCT_TERMINATE_CAUSE, (std::uint32_t) RADIUS::ACCT_TERMINATE_LOST_CARRIER);
			RADIUS_proxy_server()->RouteAndSendAccountingPacket(session.second->Destination_, SerialNumber, P, true);
		}

		AccountingSessions_.erase(hint);
	}


} // namespace OpenWifi