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

		std::string CallingStationId, AccountingSessionId, AccountingMultiSessionId, UserName, ChargeableUserIdentity, Interface;
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
			case RADIUS::CHARGEABLE_USER_IDENTITY:{
				ChargeableUserIdentity.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::PROXY_STATE: {
				std::string Tmp;
				Tmp.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
				auto ProxyParts = Poco::StringTokenizer(Tmp,":");
				if(ProxyParts.count()==4)
					Interface=ProxyParts[3];

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
			NewSession->serialNumber = Notification.SerialNumber_;
			NewSession->started = NewSession->lastTransaction = Utils::Now();
			NewSession->userName = UserName;
			NewSession->callingStationId = CallingStationId;
			NewSession->accountingSessionId = AccountingSessionId;
			NewSession->accountingMultiSessionId = AccountingMultiSessionId;
			NewSession->chargeableUserIdentity = ChargeableUserIdentity;
			NewSession->interface = Interface;
			NewSession->secret = Notification.Secret_;
			ap_hint->second[Index] = NewSession;
		} else {
			session_hint->second->lastTransaction = Utils::Now();
		}

	}

	std::uint32_t GetUiInt32(const std::uint8_t *buf) {
		return (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3] << 0);
	}

	void
	RADIUSSessionTracker::ProcessAccountingSession(OpenWifi::SessionNotification &Notification) {
		std::lock_guard     Guard(Mutex_);

		std::string CallingStationId, AccountingSessionId, AccountingMultiSessionId, UserName, ChargeableUserIdentity, Interface;
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
			case RADIUS::CHARGEABLE_USER_IDENTITY:{
				ChargeableUserIdentity.assign(
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
			case RADIUS::PROXY_STATE: {
				std::string Tmp;
				Tmp.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
				auto ProxyParts = Poco::StringTokenizer(Tmp,":");
				if(ProxyParts.count()==4)
					Interface=ProxyParts[3];

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
			NewSession->serialNumber = Notification.SerialNumber_;
			NewSession->destination = Notification.Destination_;
			NewSession->started = NewSession->lastTransaction = Utils::Now();
			NewSession->userName = UserName;
			NewSession->callingStationId = CallingStationId;
			NewSession->accountingSessionId = AccountingSessionId;
			NewSession->accountingMultiSessionId = AccountingMultiSessionId;
			NewSession->accountingPacket = Notification.Packet_;
			NewSession->destination = Notification.Destination_;
			NewSession->inputOctets = InputOctets;
			NewSession->inputPackets = InputPackets;
			NewSession->inputGigaWords = InputGigaWords;
			NewSession->outputOctets = OutputOctets;
			NewSession->outputOctets = OutputPackets;
			NewSession->outputGigaWords = OutputGigaWords;
			NewSession->sessionTime = SessionTime;
			NewSession->chargeableUserIdentity = ChargeableUserIdentity;
			NewSession->interface = Interface;
			NewSession->secret = Notification.Secret_;

			poco_debug(Logger(),fmt::format("{}: Creating session", CallingStationId));
			ap_hint->second[Index] = NewSession;

		} else {

			//  If we receive a stop, just remove that session
			if(AccountingPacketType==OpenWifi::RADIUS::ACCT_STATUS_TYPE_STOP) {
				poco_debug(Logger(),fmt::format("{}: Deleting session", CallingStationId));
				ap_hint->second.erase(Index);
			} else {
				poco_debug(Logger(),fmt::format("{}: Updating session", CallingStationId));
				session_hint->second->accountingPacket = Notification.Packet_;
				session_hint->second->destination = Notification.Destination_;
				session_hint->second->lastTransaction = Utils::Now();
				session_hint->second->inputOctets = InputOctets;
				session_hint->second->inputPackets = InputPackets;
				session_hint->second->inputGigaWords = InputGigaWords;
				session_hint->second->outputOctets = OutputOctets;
				session_hint->second->outputOctets = OutputPackets;
				session_hint->second->outputGigaWords = OutputGigaWords;
				session_hint->second->sessionTime = SessionTime;
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

	bool RADIUSSessionTracker::SendCoADM(const RADIUSSessionPtr &session) {

		RADIUS::RadiusPacket	P;

		P.PacketType(RADCMD_DISCON_REQ);
		P.Identifier(std::rand() & 0x00ff);
		P.AppendAttribute(RADIUS::AUTH_USERNAME, session->userName);
		P.AppendAttribute(RADIUS::NAS_IP, (std::uint32_t)(0x7f000001));
		P.AppendAttribute(RADIUS::CALLING_STATION_ID, session->callingStationId);
		if(!session->accountingSessionId.empty())
			P.AppendAttribute(RADIUS::ACCT_SESSION_ID, session->accountingSessionId);
		if(!session->accountingMultiSessionId.empty())
			P.AppendAttribute(RADIUS::ACCT_MULTI_SESSION_ID, session->accountingMultiSessionId);
		if(!session->chargeableUserIdentity.empty())
			P.AppendAttribute(RADIUS::CHARGEABLE_USER_IDENTITY, session->chargeableUserIdentity);
		auto ProxyState = session->serialNumber + ":" + "0.0.0.0" + ":" + "3799" + ":" + session->interface;
		std::cout << "Proxy state: " << ProxyState << "   Secret: " << session->secret << std::endl;
		// P.AppendAttribute(RADIUS::PROXY_STATE, ProxyState);
		P.RecomputeAuthenticator(session->secret);
		P.Log(std::cout);
		AP_WS_Server()->SendRadiusCoAData(session->serialNumber, P.Buffer(), P.Size_);

		return true;
	}

	bool RADIUSSessionTracker::SendCoADM(const std::string &serialNumber, const std::string &sessionId) {
		poco_information(Logger(),fmt::format("{}: SendCoADM for {}.", serialNumber, sessionId));
		std::lock_guard		Guard(Mutex_);

		auto ap_hint = AccountingSessions_.find(serialNumber);
		if(ap_hint==end(AccountingSessions_)) {
			return false;
		}

		for(const auto &[_,session]:ap_hint->second) {
			if(session->accountingSessionId==sessionId) {
				SendCoADM(session);
			}
		}

		return true;
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

			RADIUS::RadiusPacket	P(session.second->accountingPacket);

			P.P_.identifier++;
			P.ReplaceAttribute(RADIUS::ACCT_STATUS_TYPE, (std::uint32_t) RADIUS::ACCT_STATUS_TYPE_STOP);
			P.ReplaceOrAdd(RADIUS::EVENT_TIMESTAMP, (std::uint32_t) std::time(nullptr));
			P.AppendAttribute(RADIUS::ACCT_TERMINATE_CAUSE, (std::uint32_t) RADIUS::ACCT_TERMINATE_LOST_CARRIER);
			RADIUS_proxy_server()->RouteAndSendAccountingPacket(session.second->destination, SerialNumber, P, true, session.second->secret);
		}

		AccountingSessions_.erase(hint);
	}


} // namespace OpenWifi