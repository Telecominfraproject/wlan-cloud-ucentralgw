//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "framework/SubSystemServer.h"
#include "framework/UI_WebSocketClientServer.h"
#include "framework/UI_WebSocketClientNotifications.h"


namespace OpenWifi {

	class WebSocketLogger : public Poco::Channel {
	  public:

		WebSocketLogger() {
		}

		~WebSocketLogger() {
		}

		std::string getProperty( [[maybe_unused]] const std::string &p ) const {
			std::cout << "WS getProperty" << std::endl;
			return "";
		}

		void close() final {
		}

		void open() final {
		}

		static std::string to_string(Poco::Message::Priority p) {
			switch(p) {
			case Poco::Message::PRIO_INFORMATION: return "information";
			case Poco::Message::PRIO_CRITICAL: return "critical";
			case Poco::Message::PRIO_DEBUG: return "debug";
			case Poco::Message::PRIO_ERROR: return "error";
			case Poco::Message::PRIO_FATAL: return "fatal";
			case Poco::Message::PRIO_NOTICE: return "notice";
			case Poco::Message::PRIO_TRACE: return "trace";
			case Poco::Message::PRIO_WARNING: return "warning";
			default: return "none";
			}
		}

		struct NotificationLogMessage {
			std::string 		msg;
			std::string 		level;
			std::uint64_t 		timestamp;
			std::string 		source;
			std::string 		thread_name;
			std::uint64_t 		thread_id=0;

			inline void to_json(Poco::JSON::Object &Obj) const {
				RESTAPI_utils::field_to_json(Obj,"msg", msg);
				RESTAPI_utils::field_to_json(Obj,"level", level);
				RESTAPI_utils::field_to_json(Obj,"timestamp", timestamp);
				RESTAPI_utils::field_to_json(Obj,"source", source);
				RESTAPI_utils::field_to_json(Obj,"thread_name", thread_name);
				RESTAPI_utils::field_to_json(Obj,"thread_id", thread_id);
			}

			inline bool from_json(const Poco::JSON::Object::Ptr &Obj) {
				try {
					RESTAPI_utils::field_from_json(Obj, "msg", msg);
					RESTAPI_utils::field_from_json(Obj, "level", level);
					RESTAPI_utils::field_from_json(Obj, "timestamp", timestamp);
					RESTAPI_utils::field_from_json(Obj, "source", source);
					RESTAPI_utils::field_from_json(Obj, "thread_name", thread_name);
					RESTAPI_utils::field_from_json(Obj, "thread_id", thread_id);
					return true;
				} catch(...) {

				}
				return false;
			}
		};

		typedef WebSocketNotification<NotificationLogMessage> WebSocketClientNotificationLogMessage_t;

		void log(const Poco::Message &m) final {
			if(UI_WebSocketClientServer()->IsAnyoneConnected()) {
				WebSocketClientNotificationLogMessage_t		Msg;
				Msg.content.msg = m.getText();
				Msg.content.level = WebSocketLogger::to_string(m.getPriority());
				Msg.content.timestamp = m.getTime().epochTime();
				Msg.content.source = m.getSource();
				Msg.content.thread_name = m.getThread();
				Msg.content.thread_id = m.getTid();
				Msg.type_id = 1;
				UI_WebSocketClientServer()->SendNotification(Msg);
			}
		}

		void setProperty([[maybe_unused]] const std::string &name, [[maybe_unused]] const std::string &value) {
			std::cout << "WS setProperty" << std::endl;
		}

	  private:
		std::recursive_mutex	Mutex_;
	};

//	inline auto WebSocketLogger() { return WebSocketLogger::instance(); }

}