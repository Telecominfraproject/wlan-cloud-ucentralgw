//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "framework/RESTAPI_Handler.h"

#include "Poco/Environment.h"

using namespace std::chrono_literals;

namespace OpenWifi {

	class RESTAPI_system_command : public RESTAPIHandler {
	  public:
		RESTAPI_system_command(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServerAccounting & Server, uint64_t TransactionId, bool Internal)
			: RESTAPIHandler(bindings, L,
							 std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
													  Poco::Net::HTTPRequest::HTTP_GET,
													  Poco::Net::HTTPRequest::HTTP_OPTIONS},
							 Server,
							 TransactionId,
							 Internal) {}
		static auto PathName() { return std::list<std::string>{"/api/v1/system"};}

		inline void DoGet() {
			std::string Arg;
			if(HasParameter("command",Arg) && Arg=="info") {
				Poco::JSON::Object Answer;
				Answer.set(RESTAPI::Protocol::VERSION, MicroServiceVersion());
				Answer.set(RESTAPI::Protocol::UPTIME, MicroServiceUptimeTotalSeconds());
				Answer.set(RESTAPI::Protocol::START, MicroServiceStartTimeEpochTime());
				Answer.set(RESTAPI::Protocol::OS, Poco::Environment::osName());
				Answer.set(RESTAPI::Protocol::PROCESSORS, Poco::Environment::processorCount());
				Answer.set(RESTAPI::Protocol::HOSTNAME, Poco::Environment::nodeName());
				Answer.set(RESTAPI::Protocol::UI, MicroServiceGetUIURI());

				Poco::JSON::Array   Certificates;
				auto SubSystems = MicroServiceGetFullSubSystems();
				std::set<std::string>   CertNames;

				for(const auto &i:SubSystems) {
					auto Hosts=i->HostSize();
					for(uint64_t j=0;j<Hosts;++j) {
						auto CertFileName = i->Host(j).CertFile();
						if(!CertFileName.empty()) {
							Poco::File  F1(CertFileName);
							if(F1.exists()) {
								auto InsertResult = CertNames.insert(CertFileName);
								if(InsertResult.second) {
									Poco::JSON::Object Inner;
									Poco::Path F(CertFileName);
									Inner.set("filename", F.getFileName());
									Poco::Crypto::X509Certificate C(CertFileName);
									auto ExpiresOn = C.expiresOn();
									Inner.set("expiresOn", ExpiresOn.timestamp().epochTime());
									Certificates.add(Inner);
								}
							}
						}
					}
				}
				Answer.set("certificates", Certificates);
				return ReturnObject(Answer);
			}
			if(GetBoolParameter("extraConfiguration")) {
				Poco::JSON::Object  Answer;
				MicroServiceGetExtraConfiguration(Answer);
				return ReturnObject(Answer);
			}
			BadRequest(RESTAPI::Errors::InvalidCommand);
		}

		inline void DoPost() final {
			const auto & Obj = ParsedBody_;
			if (Obj->has(RESTAPI::Protocol::COMMAND)) {
				auto Command = Poco::toLower(Obj->get(RESTAPI::Protocol::COMMAND).toString());
				if (Command == RESTAPI::Protocol::SETLOGLEVEL) {
					if (Obj->has(RESTAPI::Protocol::SUBSYSTEMS) &&
						Obj->isArray(RESTAPI::Protocol::SUBSYSTEMS)) {
						auto ParametersBlock = Obj->getArray(RESTAPI::Protocol::SUBSYSTEMS);
						for (const auto &i : *ParametersBlock) {
							Poco::JSON::Parser pp;
							auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
							if (InnerObj->has(RESTAPI::Protocol::TAG) &&
								InnerObj->has(RESTAPI::Protocol::VALUE)) {
								auto Name = GetS(RESTAPI::Protocol::TAG, InnerObj);
								auto Value = GetS(RESTAPI::Protocol::VALUE, InnerObj);
								MicroServiceSetSubsystemLogLevel(Name, Value);
								poco_information(Logger_,
									fmt::format("Setting log level for {} at {}", Name, Value));
							}
						}
						return OK();
					}
				} else if (Command == RESTAPI::Protocol::GETLOGLEVELS) {
					auto CurrentLogLevels = MicroServiceGetLogLevels();
					Poco::JSON::Object Result;
					Poco::JSON::Array Array;
					for (auto &[Name, Level] : CurrentLogLevels) {
						Poco::JSON::Object Pair;
						Pair.set(RESTAPI::Protocol::TAG, Name);
						Pair.set(RESTAPI::Protocol::VALUE, Level);
						Array.add(Pair);
					}
					Result.set(RESTAPI::Protocol::TAGLIST, Array);
					return ReturnObject(Result);
				} else if (Command == RESTAPI::Protocol::GETLOGLEVELNAMES) {
					Poco::JSON::Object Result;
					Poco::JSON::Array LevelNamesArray;
					const Types::StringVec &LevelNames =  MicroServiceGetLogLevelNames();
					for (const auto &i : LevelNames)
						LevelNamesArray.add(i);
					Result.set(RESTAPI::Protocol::LIST, LevelNamesArray);
					return ReturnObject(Result);
				} else if (Command == RESTAPI::Protocol::GETSUBSYSTEMNAMES) {
					Poco::JSON::Object Result;
					Poco::JSON::Array LevelNamesArray;
					const Types::StringVec &SubSystemNames =  MicroServiceGetSubSystems();
					for (const auto &i : SubSystemNames)
						LevelNamesArray.add(i);
					Result.set(RESTAPI::Protocol::LIST, LevelNamesArray);
					return ReturnObject(Result);
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
								if(i=="daemon")
									MicroServiceReload();
								else
									MicroServiceReload(i);
							}
						});
						ReloadThread.detach();
					}
					return OK();
				}
			} else {
				return BadRequest(RESTAPI::Errors::InvalidCommand);
			}
			BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		void DoPut() final {};
		void DoDelete() final {};
	};

}