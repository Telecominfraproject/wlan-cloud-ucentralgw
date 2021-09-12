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

namespace OpenWifi {
	void RESTAPI_system_command::DoPost() {
		auto Obj = ParseStream();
		if (Obj->has(RESTAPI::Protocol::COMMAND)) {
			auto Command = Poco::toLower(Obj->get(RESTAPI::Protocol::COMMAND).toString());
			if (Command == RESTAPI::Protocol::SETLOGLEVEL) {
				if (Obj->has(RESTAPI::Protocol::PARAMETERS) &&
					Obj->isArray(RESTAPI::Protocol::PARAMETERS)) {
					auto ParametersBlock = Obj->getArray(RESTAPI::Protocol::PARAMETERS);
					for (const auto &i:*ParametersBlock) {
						Poco::JSON::Parser pp;
						auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
						if (InnerObj->has(RESTAPI::Protocol::TAG) &&
							InnerObj->has(RESTAPI::Protocol::VALUE)) {
							auto Name = GetS(RESTAPI::Protocol::TAG, InnerObj);
							auto Value = GetS(RESTAPI::Protocol::VALUE, InnerObj);
							Daemon()->SetSubsystemLogLevel(Name, Value);
							Logger_.information(Poco::format("Setting log level for %s at %s", Name, Value));
						}
					}
					OK();
					return;
				}
			} else if (Command == RESTAPI::Protocol::GETLOGLEVELS) {
				auto CurrentLogLevels = Daemon()->GetLogLevels();
				Poco::JSON::Object	Result;
				Poco::JSON::Array	Array;
				for(auto &[Name,Level]:CurrentLogLevels) {
					Poco::JSON::Object	Pair;
					Pair.set( RESTAPI::Protocol::TAG,Name);
					Pair.set(RESTAPI::Protocol::VALUE,Level);
					Array.add(Pair);
				}
				Result.set(RESTAPI::Protocol::TAGLIST,Array);
				ReturnObject(Result);
				return;
			} else if (Command == RESTAPI::Protocol::GETLOGLEVELNAMES) {
				Poco::JSON::Object	Result;
				Poco::JSON::Array	LevelNamesArray;
				const Types::StringVec & LevelNames = Daemon()->GetLogLevelNames();
				for(const auto &i:LevelNames)
					LevelNamesArray.add(i);
				Result.set(RESTAPI::Protocol::LIST,LevelNamesArray);
				ReturnObject(Result);
				return;
			} else if (Command == RESTAPI::Protocol::GETSUBSYSTEMNAMES) {
				Poco::JSON::Object	Result;
				Poco::JSON::Array	LevelNamesArray;
				const Types::StringVec & SubSystemNames = Daemon()->GetSubSystems();
				for(const auto &i:SubSystemNames)
					LevelNamesArray.add(i);
				Result.set(RESTAPI::Protocol::LIST,LevelNamesArray);
				ReturnObject(Result);
				return;
			} else if (Command == RESTAPI::Protocol::STATS) {

			} else {
				BadRequest("Unknown command.");
			}
		} else {
			BadRequest("Missing command.");
		}
	}

	void RESTAPI_system_command::DoGet() {
		auto Command = GetParameter(RESTAPI::Protocol::COMMAND, "");
		if (!Poco::icompare(Command, RESTAPI::Protocol::VERSION)) {
			Poco::JSON::Object Answer;
			Answer.set(RESTAPI::Protocol::TAG, RESTAPI::Protocol::VERSION);
			Answer.set(RESTAPI::Protocol::VALUE, Daemon()->Version());
			ReturnObject(Answer);
			return;
		}
		if (!Poco::icompare(Command, RESTAPI::Protocol::TIMES)) {
			Poco::JSON::Array	Array;
			Poco::JSON::Object 	Answer;
			Poco::JSON::Object	UpTimeObj;
			UpTimeObj.set(RESTAPI::Protocol::TAG,RESTAPI::Protocol::UPTIME);
			UpTimeObj.set(RESTAPI::Protocol::VALUE, Daemon()->uptime().totalSeconds());
			Poco::JSON::Object	StartObj;
			StartObj.set(RESTAPI::Protocol::TAG,RESTAPI::Protocol::START);
			StartObj.set(RESTAPI::Protocol::VALUE, Daemon()->startTime().epochTime());
			Array.add(UpTimeObj);
			Array.add(StartObj);
			Answer.set(RESTAPI::Protocol::TIMES, Array);
			ReturnObject(Answer);
			return;
		}
		BadRequest("Unsupported or missing parameters.");
	}
}