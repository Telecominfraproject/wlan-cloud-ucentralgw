//
// Created by stephane bourque on 2022-10-26.
//

#include "framework/EventBusManager.h"
#include "framework/KafkaManager.h"
#include "framework/utils.h"
#include "framework/MicroServiceFuncs.h"

namespace OpenWifi {

	EventBusManager::EventBusManager(Poco::Logger &L) :
		Logger_(L) {
	}

	void EventBusManager::run() {
		Running_ = true;
		Utils::SetThreadName("fmwk:EventMgr");
		auto Msg = MicroServiceMakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_JOIN);
		KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,MicroServicePrivateEndPoint(),Msg, false);
		while(Running_) {
			Poco::Thread::trySleep((unsigned long)MicroServiceDaemonBusTimer());
			if(!Running_)
				break;
			Msg = MicroServiceMakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE);
			KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,MicroServicePrivateEndPoint(),Msg, false);
		}
		Msg = MicroServiceMakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_LEAVE);
		KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,MicroServicePrivateEndPoint(),Msg, false);
	};

	void EventBusManager::Start() {
        poco_information(Logger(),"Starting...");
		if(KafkaManager()->Enabled()) {
			Thread_.start(*this);
		}
	}

	void EventBusManager::Stop() {
		if(KafkaManager()->Enabled()) {
			poco_information(Logger(),"Stopping...");
			Running_ = false;
			Thread_.wakeUp();
			Thread_.join();
			poco_information(Logger(),"Stopped...");
		}
	}

} // namespace OpenWifi