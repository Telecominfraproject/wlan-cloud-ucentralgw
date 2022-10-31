//
// Created by stephane bourque on 2022-04-28.
//

#include "GwWebSocketClient.h"
#include "SerialNumberCache.h"

namespace OpenWifi {

	GwWebSocketClient::GwWebSocketClient(Poco::Logger &Logger):
		 Logger_(Logger){
		UI_WebSocketClientServer()->SetProcessor(this);
	}

	GwWebSocketClient::~GwWebSocketClient() {
		UI_WebSocketClientServer()->SetProcessor(nullptr);
	}

	void GwWebSocketClient::Processor(const Poco::JSON::Object::Ptr &O, std::string &Answer, bool &Done ) {
		try {
			if (O->has("command")) {
				auto Command = O->get("command").toString();
				if (Command == "serial_number_search" && O->has("serial_prefix")) {
					ws_command_serial_number_search(O,Done,Answer);
				} else if (Command=="exit") {
					ws_command_exit(O,Done,Answer);
				} else {
					ws_command_invalid(O, Done, Answer);
				}
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
	}

	void GwWebSocketClient::ws_command_serial_number_search(const Poco::JSON::Object::Ptr &O,
															  bool &Done, std::string &Answer) {
		Done = false;
		auto Prefix = O->get("serial_prefix").toString();
		if (!Prefix.empty() && Prefix.length() < 13) {
			std::vector<uint64_t> Numbers;
			SerialNumberCache()->FindNumbers(Prefix, 50, Numbers);
			Poco::JSON::Array Arr;
			for (const auto &i : Numbers)
				Arr.add(Utils::int_to_hex(i));
			Poco::JSON::Object RetObj;
			RetObj.set("serialNumbers", Arr);
			std::ostringstream SS;
			Poco::JSON::Stringifier::stringify(RetObj, SS);
			Answer = SS.str();
		}
	}

	void GwWebSocketClient::ws_command_exit([[maybe_unused]] const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer) {
		Done = true;
		Answer = R"lit({ "closing" : "Goodbye! Aurevoir! Hasta la vista!" })lit";
	}

	void GwWebSocketClient::ws_command_invalid([[maybe_unused]] const Poco::JSON::Object::Ptr &O, bool &Done, std::string &Answer) {
		Done = false;
		Answer = std::string{R"lit({ "error" : "invalid command" })lit"};
	}
}