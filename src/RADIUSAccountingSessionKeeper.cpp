//
// Created by stephane bourque on 2023-03-19.
//

#include "RADIUSAccountingSessionKeeper.h"
#include <framework/utils.h>
#include <fmt/format.h>

namespace OpenWifi {

	int RADIUSAccountingSessionKeeper::Start() {
		QueueManager_.start(*this);
		return 0;
	}

	void RADIUSAccountingSessionKeeper::Stop() {
		Running_ = false;
		SessionMessageQueue_.wakeUpAll();
		QueueManager_.join();
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
		std::cout << "Sending accounting packet to proxy..." << std::endl;

		// Notification.Packet_.Log(std::cout);

		std::lock_guard     Guard(Mutex_);

		auto hint = Sessions_.find(Notification.SerialNumber_);
		if(hint==end(Sessions_)) {
			//  find the calling_station_id
			std::string CallingStationId;
			std::uint8_t AccountingPacketType = 0;
			for (const auto &attribute : Notification.Packet_.Attrs_) {
				switch (attribute.type) {
				case RADIUS::CALLING_STATION_ID: {
					CallingStationId.assign(
						&Notification.Packet_.P_.attributes[attribute.pos],
						&Notification.Packet_.P_.attributes[attribute.pos + attribute.len]);
					std::cout << "Calling station ID:" << CallingStationId << std::endl;
				} break;
				case RADIUS::ACCT_STATUS_TYPE: {
					AccountingPacketType = Notification.Packet_.P_.attributes[attribute.pos + 3];
				} break;
				default: {
				} break;
				}
			}
			std::cout << "Calling Station Id: " << CallingStationId << " type=" << (std::uint16_t) AccountingPacketType << std::endl;
		} else {

		}
	}

	void RADIUSAccountingSessionKeeper::DisconnectSession(const std::string &SerialNumber) {
		poco_information(Logger(),fmt::format("{}: Disconnecting.", SerialNumber));
	}

} // namespace OpenWifi