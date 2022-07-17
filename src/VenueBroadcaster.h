//
// Created by stephane bourque on 2022-07-16.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

	class VenueBroadcastNotification : public Poco::Notification {
	  public:
		VenueBroadcastNotification(const std::string &SourceSerialNumber, const std::string &Data, uint64_t TimeStamp) :
			SourceSerialNumber_(SourceSerialNumber),
			Data_(Data),
			TimeStamp_(TimeStamp) {

		}
		std::string 	SourceSerialNumber_;
		std::string 	Data_;
		uint64_t 		TimeStamp_=OpenWifi::Now();
	};

	class VenueBroadcaster : public SubSystemServer, Poco::Runnable {
	  public:
		static auto instance() {
			static auto instance_ = new VenueBroadcaster;
			return instance_;
		}

		int Start() override {
			Enabled_ = MicroService::instance().ConfigGetBool("venue_broadcast.enabled",true);
			if(Enabled_) {
				BroadcastManager_.start(*this);
			}
			return 0;
		}

		void Stop() override {
			if(Enabled_ && Running_) {
				BroadcastQueue_.wakeUpAll();
				BroadcastManager_.wakeUp();
				BroadcastManager_.join();
			}
		}

		void reinitialize([[maybe_unused]] Poco::Util::Application &self) override {
			Logger().information("Reinitializing.");
		}


		void run() final {
			Running_ = true;
			Utils::SetThreadName("venue-bcast");
			Poco::AutoPtr<Poco::Notification> NextNotification(BroadcastQueue_.waitDequeueNotification());
			while (NextNotification && Running_) {
				auto Notification = dynamic_cast<VenueBroadcastNotification *>(NextNotification.get());
				if (Notification != nullptr) {
					// find the source in our venues, if not present, we must find the source's venue
					// then get all the serial number in the venue. then send the same message to all the venues.
				}
				NextNotification = BroadcastQueue_.waitDequeueNotification();
			}
			Running_=false;
		}

		void Broadcast(const std::string &SourceSerial, const std::string &Data, uint64_t TimeStamp) {
			BroadcastQueue_.enqueueNotification(new VenueBroadcastNotification(SourceSerial,Data,TimeStamp));
		}

	  private:
		std::atomic_bool 				Running_=false;
		bool 							Enabled_=false;
		Poco::NotificationQueue			BroadcastQueue_;
		Poco::Thread					BroadcastManager_;

		std::map<OpenWifi::Types::UUID_t,OpenWifi::Types::UUIDvec_t>	Venues_;

		VenueBroadcaster() noexcept:
							   SubSystemServer("VenueBroadcaster", "VENUE-BCAST", "venue.broacast")
		{
		}
	};

	inline auto VenueBroadcaster() { return VenueBroadcaster::instance(); }
}