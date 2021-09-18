//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#include "RESTAPI_system_command.h"

#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"

#include "Daemon.h"
#include "RESTAPI_protocol.h"
#include "RESTAPI_errors.h"
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

namespace OpenWifi {
	void RESTAPI_system_command::DoPost() {
		auto Obj = ParseStream();
		if (Obj->has(RESTAPI::Protocol::COMMAND)) {
			auto Command = Poco::toLower(Obj->get(RESTAPI::Protocol::COMMAND).toString());
			if (Command == RESTAPI::Protocol::SETLOGLEVEL) {
				if (Obj->has(RESTAPI::Protocol::PARAMETERS) &&
					Obj->isArray(RESTAPI::Protocol::PARAMETERS)) {
					auto ParametersBlock = Obj->getArray(RESTAPI::Protocol::PARAMETERS);
					for (const auto &i : *ParametersBlock) {
						Poco::JSON::Parser pp;
						auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
						if (InnerObj->has(RESTAPI::Protocol::TAG) &&
							InnerObj->has(RESTAPI::Protocol::VALUE)) {
							auto Name = GetS(RESTAPI::Protocol::TAG, InnerObj);
							auto Value = GetS(RESTAPI::Protocol::VALUE, InnerObj);
							Daemon()->SetSubsystemLogLevel(Name, Value);
							Logger_.information(
								Poco::format("Setting log level for %s at %s", Name, Value));
						}
					}
					OK();
					return;
				}
			} else if (Command == RESTAPI::Protocol::GETLOGLEVELS) {
				auto CurrentLogLevels = Daemon()->GetLogLevels();
				Poco::JSON::Object Result;
				Poco::JSON::Array Array;
				for (auto &[Name, Level] : CurrentLogLevels) {
					Poco::JSON::Object Pair;
					Pair.set(RESTAPI::Protocol::TAG, Name);
					Pair.set(RESTAPI::Protocol::VALUE, Level);
					Array.add(Pair);
				}
				Result.set(RESTAPI::Protocol::TAGLIST, Array);
				ReturnObject(Result);
				return;
			} else if (Command == RESTAPI::Protocol::GETLOGLEVELNAMES) {
				Poco::JSON::Object Result;
				Poco::JSON::Array LevelNamesArray;
				const Types::StringVec &LevelNames = Daemon()->GetLogLevelNames();
				for (const auto &i : LevelNames)
					LevelNamesArray.add(i);
				Result.set(RESTAPI::Protocol::LIST, LevelNamesArray);
				ReturnObject(Result);
				return;
			} else if (Command == RESTAPI::Protocol::GETSUBSYSTEMNAMES) {
				Poco::JSON::Object Result;
				Poco::JSON::Array LevelNamesArray;
				const Types::StringVec &SubSystemNames = Daemon()->GetSubSystems();
				for (const auto &i : SubSystemNames)
					LevelNamesArray.add(i);
				Result.set(RESTAPI::Protocol::LIST, LevelNamesArray);
				ReturnObject(Result);
				return;
			} else if (Command == RESTAPI::Protocol::STATS) {

			} else if (Command == RESTAPI::Protocol::RELOAD) {
				if (Obj->has(RESTAPI::Protocol::SUBSYSTEMS) &&
					Obj->isArray(RESTAPI::Protocol::SUBSYSTEMS)) {
					auto SubSystems = Obj->getArray(RESTAPI::Protocol::SUBSYSTEMS);
					std::vector<std::string> Names;
					for (const auto &i : *SubSystems)
						Names.push_back(i.toString());
						std::thread	ReloadThread([Names](){
						std::this_thread::sleep_for(10000ms);
						for(const auto &i:Names) {
							Daemon()->Reload(i);
						}
					 });
					ReloadThread.detach();
				}
				OK();
				return;
			}
		} else {
			BadRequest("Unknown command.");
			return;
		}
		BadRequest("Missing command.");
	}

	void RESTAPI_system_command::DoGet() {
		std::string Arg;
		if(HasParameter("command",Arg) && Arg=="info") {
			Poco::JSON::Object Answer;
			Answer.set(RESTAPI::Protocol::VERSION, Daemon()->Version());
			Answer.set(RESTAPI::Protocol::UPTIME, Daemon()->uptime().totalSeconds());
			Answer.set(RESTAPI::Protocol::START, Daemon()->startTime().epochTime());
			Answer.set(RESTAPI::Protocol::OS, Poco::Environment::osName());
			Answer.set(RESTAPI::Protocol::PROCESSORS, Poco::Environment::processorCount());
			Answer.set(RESTAPI::Protocol::HOSTNAME, Poco::Environment::nodeName());
			ReturnObject(Answer);
			return;
		}
		BadRequest("Unknown command.");
	}
}