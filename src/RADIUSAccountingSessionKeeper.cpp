//
// Created by stephane bourque on 2023-03-19.
//

#include "RADIUSAccountingSessionKeeper.h"
#include <framework/utils.h>

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
					std::cout << "Sending accounting packet to proxy..." << std::endl;
					Session->Packet_.Log(std::cout);
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

} // namespace OpenWifi