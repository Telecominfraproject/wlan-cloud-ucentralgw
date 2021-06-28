//
// Created by stephane bourque on 2021-05-23.
//

#include "StateProcessor.h"
#include "StorageService.h"

#include "Poco/JSON/Parser.h"

namespace uCentral {

	bool StateProcessor::Add(const Poco::JSON::Object::Ptr & O) {
		try {
			UpdatesSinceLastWrite_++;
			//	get the interfaces section
			if(O->has("interfaces")) {
				auto IFaces = O->get("interfaces");
				if(IFaces.isArray()) {
					auto IFaceObjs = IFaces.extract<Poco::JSON::Array::Ptr>();
					for (auto const &i : *IFaceObjs) {
						auto Interface = i.extract<Poco::JSON::Object::Ptr>();
						if (Interface->has("name") && Interface->has("counters")) {
							auto InterfaceName = Interface->get("name").toString();
							auto InterfaceMapEntry = Stats_.find(InterfaceName);
							if(InterfaceMapEntry == Stats_.end()) {
								std::map<std::string,uint64_t>	NewStatEntry;
								Stats_[InterfaceName] = NewStatEntry;
								InterfaceMapEntry = Stats_.find(InterfaceName);
							}

							auto CountersObj = Interface->get("counters").extract<Poco::JSON::Object::Ptr>();
							Poco::DynamicStruct CounterVars = *CountersObj;

							for (const auto &j : CounterVars) {
								auto Entry = InterfaceMapEntry->second.find(j.first);
								if(Entry==InterfaceMapEntry->second.end()) {
									InterfaceMapEntry->second[j.first] = j.second;
								} else {
									InterfaceMapEntry->second[j.first] += j.second;
								}
							}
						} else {
							return false;
						}
					}
					if(UpdatesSinceLastWrite_>10)
						Save();
					return true;
				}
			} else {
				std::cout << "No interfaces section" << std::endl;
			}
		} catch (const Poco::Exception &E ) {
			std::cout << "Exception0.. " <<  E.displayText() << " " << E.what() << std::endl;
		}
		return false;
	}

	bool StateProcessor::Add(const std::string &S) {
		try {
			Poco::JSON::Parser parser;
			auto ParsedMessage = parser.parse(S);
			const auto & Result = ParsedMessage.extract<Poco::JSON::Object::Ptr>();
			return Add(Result);
		} catch (const Poco::Exception &E) {
			std::cout << "Exception1.." << std::endl;
		}
		return false;
	}

	void StateProcessor::Print() const {
		for(const auto & Interface: Stats_) {
			std::cout << "Interface: " << Interface.first << std::endl;
			for(const auto &[Name,Value]:Interface.second) {
				std::cout << "     " << Name << ": " << Value << std::endl;
			}
		}
	}

	void StateProcessor::to_json(Poco::JSON::Object & Obj) const {
		/* interfaces: [
			name:
			counters: {

			}
		*/
		Poco::JSON::Array	Interfaces;
		for(const auto & Interface: Stats_) {
			Poco::JSON::Object InnerObj;
			Poco::JSON::Object	CountersObj;
			for(const auto &[Name,Value]:Interface.second) {
				CountersObj.set(Name,Value);
			}
			InnerObj.set("name",Interface.first);
			InnerObj.set("counters",CountersObj);
			Interfaces.add(InnerObj);
		}
		Obj.set("interfaces",Interfaces);
	}

	std::string StateProcessor::toString() const {
		try {
			Poco::JSON::Object Obj;
			to_json(Obj);
			std::ostringstream NewStats;
			Poco::JSON::Stringifier stringifier;
			stringifier.condense(Obj, NewStats);
			return NewStats.str();
		} catch( const Poco::Exception &E ) {

		}
		return "";
	};

	bool StateProcessor::Initialize(std::string &SerialNumber) {
		SerialNumber_ = SerialNumber;
		UpdatesSinceLastWrite_ = 0;
		Stats_.clear();
		std::string Stats;
		if(Storage()->GetLifetimeStats(SerialNumber,Stats)) {
			Add(Stats);
			return true;
		}
		return false;
	}

	bool StateProcessor::Save() {
		UpdatesSinceLastWrite_ = 0;
		std::string StatsToSave = toString();
		return Storage()->SetLifetimeStats(SerialNumber_, StatsToSave);
	}
}