//
// Created by stephane bourque on 2021-05-23.
//

#include "uStateProcessor.h"
#include "Poco/JSON/Parser.h"

#include "uUtils.h"

namespace uCentral {

	bool uStateProcessor::Add(const Poco::JSON::Object::Ptr & O) {
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

	bool uStateProcessor::Add(const std::string &S) {
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

	void uStateProcessor::Print() const {
		for(const auto & Interface: Stats_) {
			std::cout << "Interface: " << Interface.first << std::endl;
			for(const auto &[Name,Value]:Interface.second) {
				std::cout << "     " << Name << ": " << Value << std::endl;
			}
		}
	}

	void uStateProcessor::to_json(Poco::JSON::Object & Obj) const {
		for(const auto & Interface: Stats_) {
			Poco::JSON::Object InnerObj;
			for(const auto &[Name,Value]:Interface.second) {
				InnerObj.set(Name,Value);
			}
			Obj.set(Interface.first,InnerObj);
		}
	}

	bool uStateProcessor::Initialize(const std::string &SerialNumber) {
		SerialNumber_ = SerialNumber;
		UpdatesSinceLastWrite_ = 0;
		Stats_.clear();

		return false;
	}

	bool uStateProcessor::Save() {
		UpdatesSinceLastWrite_ = 0;

		return false;
	}
}