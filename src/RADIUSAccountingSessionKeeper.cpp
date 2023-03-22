//
// Created by stephane bourque on 2023-03-19.
//

#include "RADIUSAccountingSessionKeeper.h"
#include <framework/utils.h>
#include <fmt/format.h>

#include "RADIUS_proxy_server.h"

namespace OpenWifi {

	int RADIUSAccountingSessionKeeper::Start() {
		poco_information(Logger(),"Starting...");
		QueueManager_.start(*this);
		return 0;
	}

	void RADIUSAccountingSessionKeeper::Stop() {
		poco_information(Logger(),"Stopping...");
		Running_ = false;
		SessionMessageQueue_.wakeUpAll();
		QueueManager_.join();
		poco_information(Logger(),"Stopped...");
	}

	void RADIUSAccountingSessionKeeper::run() {
		Utils::SetThreadName("rad:sessmgr");
		Running_ = true;

		Poco::AutoPtr<Poco::Notification> NextSession(SessionMessageQueue_.waitDequeueNotification());
		while (NextSession && Running_) {
			auto Session = dynamic_cast<SessionNotification *>(NextSession.get());

			try {
				if (Session != nullptr) {
					switch(Session->Type_) {
						case SessionNotification::NotificationType::session_message: {
							ProcessSession(*Session);
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

	void RADIUSAccountingSessionKeeper::ProcessSession(OpenWifi::SessionNotification &Notification) {
		std::lock_guard     Guard(Mutex_);

		std::string CallingStationId;
		std::uint8_t AccountingPacketType = 0;
		for (const auto &attribute : Notification.Packet_.Attrs_) {
			switch (attribute.type) {
			case RADIUS::CALLING_STATION_ID: {
				CallingStationId.assign(
					&Notification.Packet_.P_.attributes[attribute.pos],
					&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
			} break;
			case RADIUS::ACCT_STATUS_TYPE: {
				AccountingPacketType = Notification.Packet_.P_.attributes[attribute.pos + 3];
			} break;
			default: {
			} break;
			}
		}

		auto hint = Sessions_.find(Notification.SerialNumber_);
		if(hint==end(Sessions_)) {
			//  find the calling_station_id
			//  if we are getting a stop for something we do not know, nothing to do...
			if( AccountingPacketType!=OpenWifi::RADIUS::ACCT_STATUS_TYPE_START &&
				AccountingPacketType!=OpenWifi::RADIUS::ACCT_STATUS_TYPE_INTERIM_UPDATE)
				return;

			RADIUSAccountingSession S;
			S.Started_ = Utils::Now();
			S.Destination = Notification.Destination_;
			S.Packet_ = Notification.Packet_;

			std::map<std::string,RADIUSAccountingSession>  Sessions;
			Sessions[CallingStationId] = S;
			Sessions_[Notification.SerialNumber_] = Sessions;
			poco_debug(Logger(),fmt::format("{}: Creating session", CallingStationId));
		} else {

			//  If we receive a stop, just remove that session
			if(AccountingPacketType==OpenWifi::RADIUS::ACCT_STATUS_TYPE_STOP) {
				poco_debug(Logger(),fmt::format("{}: Deleting session", CallingStationId));
				hint->second.erase(CallingStationId);
			} else {
				//  we are either starting or interim, which means ths same.
				auto device_session = hint->second.find(CallingStationId);
				if(device_session == end(hint->second)) {
					poco_debug(Logger(),fmt::format("{}: Creating session", CallingStationId));
					RADIUSAccountingSession S;
					S.Started_ = Utils::Now();
					S.Destination = Notification.Destination_;
					S.Packet_ = Notification.Packet_;
					hint->second[CallingStationId] = S;
				} else {
					poco_debug(Logger(),fmt::format("{}: Updating session", CallingStationId));
					device_session->second.Packet_ = Notification.Packet_;
					device_session->second.Destination = Notification.Destination_;
				}
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

	void RADIUSAccountingSessionKeeper::DisconnectSession(const std::string &SerialNumber) {
		poco_information(Logger(),fmt::format("{}: Disconnecting.", SerialNumber));

		std::lock_guard		Guard(Mutex_);

		auto hint = Sessions_.find(SerialNumber);
		if(hint==end(Sessions_)) {
			return;
		}

		//	we need to go through all sessions and send an accounting stop
		for(const auto &session:hint->second) {
			poco_debug(Logger(), fmt::format("Stopping accounting for {}:{}", SerialNumber, session.first ));

			RADIUS::RadiusPacket	P(session.second.Packet_);

			store_packet(SerialNumber, (const char *)P.Buffer(), P.Size(), 0);

			P.P_.identifier++;
			P.ReplaceAttribute(RADIUS::ACCT_STATUS_TYPE, (std::uint32_t) RADIUS::ACCT_STATUS_TYPE_STOP);
			P.ReplaceOrAdd(RADIUS::EVENT_TIMESTAMP, (std::uint32_t) std::time(nullptr));
			P.AppendAttribute(RADIUS::ACCT_TERMINATE_CAUSE, (std::uint32_t) RADIUS::ACCT_TERMINATE_LOST_CARRIER);
			RADIUS_proxy_server()->RouteAndSendAccountingPacket(session.second.Destination, SerialNumber, P, true);
			store_packet(SerialNumber, (const char *)P.Buffer(), P.Size(), 1);
		}

		Sessions_.erase(hint);
	}

} // namespace OpenWifi