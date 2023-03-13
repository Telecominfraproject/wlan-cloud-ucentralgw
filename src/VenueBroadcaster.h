//
// Created by stephane bourque on 2022-07-16.
//

#pragma once

#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"

#include "AP_WS_Server.h"
#include "sdks/sdk_prov.h"

#include "framework/MicroServiceFuncs.h"
#include "framework/SubSystemServer.h"
#include "framework/utils.h"

namespace OpenWifi {

	class VenueBroadcastNotification : public Poco::Notification {
	  public:
		VenueBroadcastNotification(const std::string &SourceSerialNumber, Poco::JSON::Object::Ptr Data,
								   uint64_t TimeStamp)
			: SourceSerialNumber_(SourceSerialNumber), Data_(Data), TimeStamp_(TimeStamp) {}
		std::string SourceSerialNumber_;
		Poco::JSON::Object::Ptr Data_;
		uint64_t TimeStamp_ = Utils::Now();
	};

	class VenueBroadcaster : public SubSystemServer, Poco::Runnable {
	  public:
		static auto instance() {
			static auto instance_ = new VenueBroadcaster;
			return instance_;
		}

		inline int Start() override {
			Enabled_ = MicroServiceConfigGetBool("venue_broadcast.enabled", true);
			if (Enabled_) {
				BroadcastManager_.start(*this);
			}
			return 0;
		}

		inline void Stop() override {
			poco_information(Logger(), "Stopping...");
			if (Enabled_ && Running_) {
				BroadcastQueue_.wakeUpAll();
				BroadcastManager_.wakeUp();
				BroadcastManager_.join();
			}
			poco_information(Logger(), "Stopped...");
		}

		inline void reinitialize([[maybe_unused]] Poco::Util::Application &self) override {
			poco_information(Logger(), "Reinitializing.");
		}

		struct VenueInfo {
			uint64_t timestamp = Utils::Now();
			Types::StringVec serialNumbers;
		};

		inline bool FindSerialNumberList(const std::string &Source,
										 OpenWifi::Types::StringVec &SerialNumbers) {
			//	Can we find our serial number in any of the lists so far...
			for (const auto &venue : Venues_) {
				auto entry = std::find(venue.second.serialNumbers.begin(),
									   venue.second.serialNumbers.end(), Source);
				if (entry != venue.second.serialNumbers.end() &&
					(Utils::Now() - venue.second.timestamp) < 600) {
					SerialNumbers = venue.second.serialNumbers;
					auto entry2 = std::find(SerialNumbers.begin(), SerialNumbers.end(), Source);
					SerialNumbers.erase(entry2);
					return true;
				}
			}

			//	get the venue from Prov and the serial numbers.
			Types::UUID_t Venue;
			Types::StringVec TmpSerialNumbers;
			if (OpenWifi::SDK::Prov::GetSerialNumbersForVenueOfSerialNumber(
					Source, Venue, TmpSerialNumbers, Logger())) {
				std::sort(TmpSerialNumbers.begin(), TmpSerialNumbers.end());
				VenueInfo V{.timestamp = Utils::Now(), .serialNumbers = TmpSerialNumbers};
				Venues_[Venue] = V;
				auto p = std::find(TmpSerialNumbers.begin(), TmpSerialNumbers.end(), Source);
				if (p != TmpSerialNumbers.end()) {
					TmpSerialNumbers.erase(p);
					SerialNumbers = TmpSerialNumbers;
					return true;
				}
			}

			return false;
		}

		inline void SendToDevice(const std::string &SerialNumber, const std::string &Payload) {
			AP_WS_Server()->SendFrame(SerialNumber, Payload);
		}

		inline void run() final {
			Running_ = true;
			Utils::SetThreadName("venue-bcast");
			Poco::AutoPtr<Poco::Notification> NextNotification(
				BroadcastQueue_.waitDequeueNotification());
			while (NextNotification && Running_) {
				auto Notification =
					dynamic_cast<VenueBroadcastNotification *>(NextNotification.get());
				if (Notification != nullptr) {
					Types::StringVec SerialNumbers;
					if (FindSerialNumberList(Notification->SourceSerialNumber_, SerialNumbers)) {
						Poco::JSON::Object Payload;
						Payload.set("jsonrpc", "2.0");
						Payload.set("method", "venue_broadcast");
						Poco::JSON::Object ParamBlock;
						ParamBlock.set("serial", Notification->SourceSerialNumber_);
						ParamBlock.set("timestamp", Notification->TimeStamp_);
						ParamBlock.set("data", Notification->Data_);
						Payload.set("params", ParamBlock);
						std::ostringstream o;
						Payload.stringify(o);
						for (const auto &Device : SerialNumbers) {
							SendToDevice(Device, o.str());
						}
					}
				}
				NextNotification = BroadcastQueue_.waitDequeueNotification();
			}
			Running_ = false;
		}

		inline void Broadcast(const std::string &SourceSerial, Poco::JSON::Object::Ptr Data,
							  uint64_t TimeStamp) {
			BroadcastQueue_.enqueueNotification(
				new VenueBroadcastNotification(SourceSerial, Data, TimeStamp));
		}

	  private:
		std::atomic_bool Running_ = false;
		bool Enabled_ = false;
		Poco::NotificationQueue BroadcastQueue_;
		Poco::Thread BroadcastManager_;

		std::map<OpenWifi::Types::UUID_t, VenueInfo> Venues_;

		VenueBroadcaster() noexcept
			: SubSystemServer("VenueBroadcaster", "VENUE-BCAST", "venue.broacast") {}
	};

	inline auto VenueBroadcaster() { return VenueBroadcaster::instance(); }
} // namespace OpenWifi