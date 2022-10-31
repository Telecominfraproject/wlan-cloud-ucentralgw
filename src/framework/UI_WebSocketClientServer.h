//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <map>
#include <string>

#include "Poco/Runnable.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/SocketNotification.h"

#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "framework/SubSystemServer.h"
#include "framework/UI_WebSocketClientNotifications.h"

namespace OpenWifi {

	class UI_WebSocketClient;

	class UI_WebSocketClientProcessor {
	  public:
		virtual void Processor(const Poco::JSON::Object::Ptr &O, std::string &Answer, bool &Done ) = 0;
	  private:
	};

	class UI_WebSocketClientServer : public SubSystemServer, Poco::Runnable {

	  public:
		static auto instance() {
			static auto instance_ = new UI_WebSocketClientServer;
			return instance_;
		}

		int Start() override;
		void Stop() override;
		void run() override;
		Poco::Net::SocketReactor & Reactor() { return Reactor_; }
		void NewClient(Poco::Net::WebSocket &WS, const std::string &Id, const std::string &UserName);
		void SetProcessor(UI_WebSocketClientProcessor *F);
		void UnRegister(const std::string &Id);
		void SetUser(const std::string &Id, const std::string &UserId);
		[[nodiscard]] inline bool GeoCodeEnabled() const { return GeoCodeEnabled_; }
		[[nodiscard]] inline std::string GoogleApiKey() const { return GoogleApiKey_; }
		[[nodiscard]] bool Send(const std::string &Id, const std::string &Payload);

		template <typename T> bool
		SendUserNotification(const std::string &userName, const WebSocketNotification<T> &Notification) {

			Poco::JSON::Object  Payload;
			Notification.to_json(Payload);
			Poco::JSON::Object  Msg;
			Msg.set("notification",Payload);
			std::ostringstream OO;
			Msg.stringify(OO);

			return SendToUser(userName,OO.str());
		}

		template <typename T> void SendNotification(const WebSocketNotification<T> &Notification) {
			Poco::JSON::Object  Payload;
			Notification.to_json(Payload);
			Poco::JSON::Object  Msg;
			Msg.set("notification",Payload);
			std::ostringstream OO;
			Msg.stringify(OO);
			SendToAll(OO.str());
		}

		[[nodiscard]] bool SendToUser(const std::string &userName, const std::string &Payload);
		void SendToAll(const std::string &Payload);

	  private:
		mutable std::atomic_bool Running_ = false;
		Poco::Thread 								Thr_;
		Poco::Net::SocketReactor					Reactor_;
		Poco::Thread								ReactorThread_;
		bool GeoCodeEnabled_ = false;
		std::string GoogleApiKey_;
		std::map<std::string, std::pair<std::unique_ptr<UI_WebSocketClient>, std::string>> Clients_;
		UI_WebSocketClientProcessor *Processor_ = nullptr;
		UI_WebSocketClientServer() noexcept;
	};

	inline auto UI_WebSocketClientServer() { return UI_WebSocketClientServer::instance(); }

	class UI_WebSocketClient {
	  public:
		explicit UI_WebSocketClient(Poco::Net::WebSocket &WS,
								 const std::string &Id,
								 const std::string &UserName,
								 Poco::Logger &L,
								 UI_WebSocketClientProcessor *Processor);
		virtual ~UI_WebSocketClient();
		[[nodiscard]] inline const std::string &Id();
		[[nodiscard]] Poco::Logger &Logger();
		bool Send(const std::string &Payload);
		void EndConnection();
	  private:
		std::unique_ptr<Poco::Net::WebSocket> WS_;
		Poco::Net::SocketReactor 	&Reactor_;
		std::string 				Id_;
		std::string					UserName_;
		Poco::Logger 				&Logger_;
		std::atomic_bool 			Authenticated_ = false;
		volatile bool				SocketRegistered_=false;
		SecurityObjects::UserInfoAndPolicy UserInfo_;
		UI_WebSocketClientProcessor *Processor_ = nullptr;
		void OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);
		void OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf);
	};

};

