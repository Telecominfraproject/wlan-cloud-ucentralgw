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
		Utils::SetThreadName(ReactorThr_,"tel:reactor");
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
		Clients_[UUID] = nullptr;
		return true;
	}

	void TelemetryStream::run() {
		Utils::SetThreadName("tel:notifier");
		Poco::AutoPtr<Poco::Notification> NextNotification(MsgQueue_.waitDequeueNotification());
		while (NextNotification && Running_) {
			auto Notification = dynamic_cast<TelemetryNotification *>(NextNotification.get());
			if (Notification != nullptr) {
				std::lock_guard 	Lock(Mutex_);
				auto SerialNumberSetOfUUIDs = SerialNumbers_.find(Notification->SerialNumber_);
				if (SerialNumberSetOfUUIDs != SerialNumbers_.end()) {
					std::cout << "Found serial" << std::endl;
					for (auto &uuid : SerialNumberSetOfUUIDs->second) {
						std::cout << "Sending WS telemetry notification" << std::endl;
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
				} else {
					std::cout << "Cannot find serial: " << Utils::IntToSerialNumber(Notification->SerialNumber_) << std::endl;
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

		std::cout << "Removing client WS" << std::endl;
		auto Hint = Clients_.find(UUID);
		if(Hint!=Clients_.end()) {
			std::cout << "Removing client WS " << UUID << std::endl;
			Clients_.erase(Hint);
			for(const auto &i:SerialNumbers_) {
				std::cout << "Removing client WS serial" << std::endl;
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
		} else {
			std::cout << "Cannot derigister UUID " << UUID << std::endl;
		}
	}
}