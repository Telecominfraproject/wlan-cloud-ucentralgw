//
// Created by stephane bourque on 2021-09-07.
//
#include <thread>

#include "framework/MicroService.h"

#include "Poco/JSON/Array.h"
#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/URI.h"

#include "RESTAPI/RESTAPI_telemetryWebSocket.h"
#include "TelemetryStream.h"

namespace OpenWifi {

	int TelemetryStream::Start() {
		Running_ = true;
		ReactorThr_.start(Reactor_);
		Utils::SetThreadName(ReactorThr_,"telemetry-svr");
		NotificationMgr_.start(*this);
		return 0;
	}

	void TelemetryStream::Stop() {
		poco_information(Logger(),"Stopping...");
		Running_ = false;
		Reactor_.stop();
		ReactorThr_.join();
		MsgQueue_.wakeUpAll();
		NotificationMgr_.wakeUp();
		NotificationMgr_.join();
		poco_information(Logger(),"Stopped...");
	}

	bool TelemetryStream::IsValidEndPoint(uint64_t SerialNumber, const std::string & UUID) {
		std::lock_guard	G(Mutex_);

		auto U = Clients_.find(UUID);
		if(U == Clients_.end() )
			return false;

		auto N = SerialNumbers_.find(SerialNumber);
		if(N == SerialNumbers_.end())
			return false;

		return (N->second.find(UUID) != N->second.end());
	}

	bool TelemetryStream::CreateEndpoint(uint64_t SerialNumber, std::string &EndPoint, const std::string &UUID) {
		std::lock_guard	G(Mutex_);

		Poco::URI	Public(MicroService::instance().ConfigGetString("openwifi.system.uri.public"));
		Poco::URI	U;
		U.setScheme("wss");
		U.setHost(Public.getHost());
		U.setPort(Public.getPort());
		auto RESTAPI_Path = std::string(*(RESTAPI_telemetryWebSocket::PathName().begin()));
		U.setPath(RESTAPI_Path);
		U.addQueryParameter("uuid", UUID);
		U.addQueryParameter("serialNumber", Utils::IntToSerialNumber(SerialNumber));
		EndPoint = U.toString();
		auto H = SerialNumbers_.find(SerialNumber);
		if(H == SerialNumbers_.end()) {
			std::set<std::string>	UUIDs{UUID};
			SerialNumbers_[SerialNumber] = UUIDs;
		} else {
			H->second.insert(UUID);
		}
		return true;
	}

	void TelemetryStream::run() {
		Utils::SetThreadName("tel:mgr");
		Poco::AutoPtr<Poco::Notification> NextNotification(MsgQueue_.waitDequeueNotification());
		while (NextNotification && Running_) {
			auto Notification = dynamic_cast<TelemetryNotification *>(NextNotification.get());
			if (Notification != nullptr) {
				std::lock_guard 	Lock(Mutex_);
				auto SerialNumberSetOfUUIDs = SerialNumbers_.find(Notification->SerialNumber_);
				if (SerialNumberSetOfUUIDs != SerialNumbers_.end()) {
					for (auto &uuid : SerialNumberSetOfUUIDs->second) {
						auto Client = Clients_.find(uuid);
						if (Client != Clients_.end() && Client->second != nullptr) {
							try {
								std::cout << "Sent WS telemetry notification" << std::endl;
								Client->second->Send(Notification->Payload_);
							} catch (...) {
								std::cout << "Cannot send WS telemetry notification" << std::endl;
							}
						}
					}
				}
			}
			NextNotification = MsgQueue_.waitDequeueNotification();
		}
	}


	bool TelemetryStream::NewClient(const std::string &UUID, uint64_t SerialNumber, std::unique_ptr<Poco::Net::WebSocket> Client) {
		std::lock_guard	G(Mutex_);
		try {
			Clients_[UUID] = std::make_unique<TelemetryClient>(
				UUID, SerialNumber, std::move(Client), NextReactor(), Logger());
			auto set = SerialNumbers_[SerialNumber];
			set.insert(UUID);
			SerialNumbers_[SerialNumber] = set;
			return true;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
			poco_warning(Logger(),fmt::format("Could not create a telemetry client for session {} and serial number {}", UUID, SerialNumber));
		}
		return false;
	}

	void TelemetryStream::DeRegisterClient(const std::string &UUID) {
		std::lock_guard		G(Mutex_);

		auto Hint = Clients_.find(UUID);
		if(Hint!=Clients_.end()) {
			Clients_.erase(Hint);
			for(const auto &i:SerialNumbers_) {
				auto S = i.second;
				S.erase(UUID);
			}

			//	remove empty slots
			for( auto i = SerialNumbers_.begin(); i!= SerialNumbers_.end();) {
				if(i->second.empty()) {
					i = SerialNumbers_.erase(i);
				} else {
					++i;
				}
			}
		}
	}
}