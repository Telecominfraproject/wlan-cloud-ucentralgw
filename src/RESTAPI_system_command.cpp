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

namespace uCentral {
	void RESTAPI_system_command::handleRequest(Poco::Net::HTTPServerRequest &Request,
											   Poco::Net::HTTPServerResponse &Response) {

		if (!ContinueProcessing(Request, Response))
			return;

		if (!IsAuthorized(Request, Response))
			return;

		if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
			DoPost(Request, Response);
		else if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_GET)
			DoGet(Request, Response);
		else
            BadRequest(Request, Response, "Unsupported method.");
	}

	void RESTAPI_system_command::DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
		try {
			Poco::JSON::Parser parser;
			auto Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

			if (Obj->has(uCentral::RESTAPI::Protocol::COMMAND)) {
				auto Command = Poco::toLower(Obj->get(uCentral::RESTAPI::Protocol::COMMAND).toString());
				if (Command == uCentral::RESTAPI::Protocol::SETLOGLEVEL) {
					if (Obj->has(uCentral::RESTAPI::Protocol::PARAMETERS) &&
						Obj->isArray(uCentral::RESTAPI::Protocol::PARAMETERS)) {
						auto ParametersBlock = Obj->getArray(uCentral::RESTAPI::Protocol::PARAMETERS);
						for (const auto &i:*ParametersBlock) {
							Poco::JSON::Parser pp;
							auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
							if (InnerObj->has(uCentral::RESTAPI::Protocol::TAG) &&
								InnerObj->has(uCentral::RESTAPI::Protocol::VALUE)) {
								auto Name = GetS(uCentral::RESTAPI::Protocol::TAG, InnerObj);
								auto Value = GetS(uCentral::RESTAPI::Protocol::VALUE, InnerObj);
								Daemon()->SetSubsystemLogLevel(Name, Value);
								Logger_.information(Poco::format("Setting log level for %s at %s", Name, Value));
							}
						}
						OK(Request, Response);
						return;
					}
				} else if (Command == uCentral::RESTAPI::Protocol::GETLOGLEVELS) {
					auto CurrentLogLevels = Daemon()->GetLogLevels();
					Poco::JSON::Object	Result;
					Poco::JSON::Array	Array;
					for(auto &[Name,Level]:CurrentLogLevels) {
						Poco::JSON::Object	Pair;
						Pair.set( uCentral::RESTAPI::Protocol::TAG,Name);
						Pair.set(uCentral::RESTAPI::Protocol::VALUE,Level);
						Array.add(Pair);
					}
					Result.set(uCentral::RESTAPI::Protocol::TAGLIST,Array);
					ReturnObject(Request,Result,Response);
					return;
				} else if (Command == uCentral::RESTAPI::Protocol::GETLOGLEVELNAMES) {
					Poco::JSON::Object	Result;
					Poco::JSON::Array	LevelNamesArray;
					const Types::StringVec & LevelNames = Daemon()->GetLogLevelNames();
					for(const auto &i:LevelNames)
						LevelNamesArray.add(i);
					Result.set(uCentral::RESTAPI::Protocol::LIST,LevelNamesArray);
					ReturnObject(Request,Result,Response);
					return;
				} else if (Command == uCentral::RESTAPI::Protocol::GETSUBSYSTEMNAMES) {
					Poco::JSON::Object	Result;
					Poco::JSON::Array	LevelNamesArray;
					const Types::StringVec & SubSystemNames = Daemon()->GetSubSystems();
					for(const auto &i:SubSystemNames)
						LevelNamesArray.add(i);
					Result.set(uCentral::RESTAPI::Protocol::LIST,LevelNamesArray);
					ReturnObject(Request,Result,Response);
					return;
				} else if (Command == uCentral::RESTAPI::Protocol::STATS) {

				}
			}
		} catch(const Poco::Exception &E) {
			Logger_.log(E);
		}
		BadRequest(Request, Response, "Unsupported or missing parameters.");
	}

	void RESTAPI_system_command::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
		try {
			ParseParameters(Request);
			auto Command = GetParameter(RESTAPI::Protocol::COMMAND, "");
			if (!Poco::icompare(Command, RESTAPI::Protocol::VERSION)) {
				Poco::JSON::Object Answer;
				Answer.set(RESTAPI::Protocol::TAG, RESTAPI::Protocol::VERSION);
				Answer.set(RESTAPI::Protocol::VALUE, Daemon()->Version());
				ReturnObject(Request, Answer, Response);
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
				ReturnObject(Request, Answer, Response);
				return;
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		BadRequest(Request, Response, "Unsupported or missing parameters.");
	}

}