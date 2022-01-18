//
// Created by stephane bourque on 2021-05-23.
//

#include "StateProcessor.h"
#include "StorageService.h"

#include "Poco/JSON/Parser.h"

namespace OpenWifi {

	bool StateProcessor::Add(const Poco::JSON::Object::Ptr & O) {
		try {

			std::cout << SerialNumber_ << std::endl;

			UpdatesSinceLastWrite_++;

			//	get the interfaces section
			if(O->has("interfaces") && O->isArray("interfaces")) {
				auto IFaces = O->getArray("interfaces");
				for (auto const &i : *IFaces) {
					auto Interface = i.extract<Poco::JSON::Object::Ptr>();

					if (Interface->has("name")) {
						_OWDEBUG_
						auto InterfaceName = Interface->get("name").toString();
						auto InterfaceMapEntry = Stats_.find(InterfaceName);
						if(InterfaceMapEntry == Stats_.end()) {
							std::map<std::string,uint64_t>	NewStatEntry;
							Stats_[InterfaceName] = NewStatEntry;
							InterfaceMapEntry = Stats_.find(InterfaceName);
						}
						_OWDEBUG_
						//	old stats version...
						if(Interface->has("counters")) {
							auto CountersObj = Interface->getObject("counters");
							for (const auto &j : *CountersObj) {
								_OWDEBUG_
								auto Entry = InterfaceMapEntry->second.find(j.first);
								if (Entry == InterfaceMapEntry->second.end()) {
									_OWDEBUG_
									InterfaceMapEntry->second[j.first] = j.second;
								} else {
									_OWDEBUG_
									InterfaceMapEntry->second[j.first] += j.second;
								}
							}
						} else {
							// 	must be the new version. So now we must get the ssids section, and iterate over each association
							//	and update the global counter with the new values. We can ignore the deltas since we really want the 	absolute values.
							try {
								_OWDEBUG_
								auto SSIDs = Interface->getArray("ssids");
								_OWDEBUG_
								uint64_t rx_bytes = 0, rx_packets=0, tx_bytes=0,tx_packets=0,tx_retries=0,tx_failed=0;
								for (const auto &SSID : *SSIDs) {
									_OWDEBUG_
									auto SSID_info = SSID.extract<Poco::JSON::Object::Ptr>();
									auto Associations = SSID_info->getArray("associations");
									for (const auto &k : *Associations) {
										auto A = k.extract<Poco::JSON::Object::Ptr>();
										_OWDEBUG_
										rx_bytes += A->getValue<uint64_t>("rx_bytes");
										rx_packets += A->getValue<uint64_t>("rx_packets");
										tx_bytes += A->getValue<uint64_t>("tx_bytes");
										tx_packets += A->getValue<uint64_t>("tx_packets");
										tx_retries += A->getValue<uint64_t>("tx_retries");
										tx_failed += A->getValue<uint64_t>("tx_failed");
										_OWDEBUG_
									}
								}
								InterfaceMapEntry->second["rx_bytes"] = rx_bytes;
								InterfaceMapEntry->second["rx_packets"] = rx_packets;
								InterfaceMapEntry->second["tx_bytes"] = tx_bytes;
								InterfaceMapEntry->second["tx_packets"] = tx_packets;
								InterfaceMapEntry->second["tx_retries"] = tx_retries;
								InterfaceMapEntry->second["tx_failed"] = tx_failed;
							} catch (const Poco::Exception &E) {
								_OWDEBUG_
								Logger().log(E);
							}
						}
					}
				}

				if(Conn_)
					GetAssociations(O,Conn_->Conn_.Associations_2G,Conn_->Conn_.Associations_5G);

				if(UpdatesSinceLastWrite_>10)
					Save();
				return true;
			} else {
				Logger().information(Poco::format("DEVICE(%s): State is missing interfaces",SerialNumber_));
			}
		} catch (const Poco::Exception &E ) {
			Logger().log(E);
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
			Logger().log(E);
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
		if(StorageService()->GetLifetimeStats(SerialNumber,Stats)) {
			Add(Stats);
			return true;
		}
		return false;
	}

	bool StateProcessor::Save() {
		UpdatesSinceLastWrite_ = 0;
		std::string StatsToSave = toString();
		return StorageService()->SetLifetimeStats(SerialNumber_, StatsToSave);
	}

	static int ChannelToBand(uint64_t C) {
		if(C>=1 && C<=16) return 2;
		return 5;
	}

	bool StateProcessor::GetAssociations(const Poco::JSON::Object::Ptr &RawObject, uint64_t &Radios_2G,
										 uint64_t &Radios_5G) {
		Radios_2G = 0 ;
		Radios_5G = 0;
		if(RawObject->isArray("radios") && RawObject->isArray("interfaces")) {
			auto RA = RawObject->getArray("radios");
			// map of phy to 2g/5g
			std::map<std::string,int>   RadioPHYs;
			//  parse radios and get the phy out with the band
			for(auto const &i:*RA) {
				Poco::JSON::Parser p2;
				auto RadioObj = i.extract<Poco::JSON::Object::Ptr>();
				if(RadioObj->has("phy") && RadioObj->has("channel")) {
					_OWDEBUG_
					if(RadioObj->isArray("channel")) {
						auto ChannelArray = RadioObj->getArray("channel");
						if(ChannelArray->size()) {
							_OWDEBUG_
							RadioPHYs[RadioObj->get("phy").toString()] =
								ChannelToBand( ChannelArray->getElement<uint64_t>(0) );
						}
					} else {
						_OWDEBUG_
						RadioPHYs[RadioObj->get("phy").toString()] =
							ChannelToBand(RadioObj->get("channel"));
					}
				}
			}

			auto IA = RawObject->getArray("interfaces");
			for(auto const &i:*IA) {
				auto InterfaceObj = i.extract<Poco::JSON::Object::Ptr>();
				if(InterfaceObj->isArray("ssids")) {
					auto SSIDA = InterfaceObj->getArray("ssids");
					for(const auto &s:*SSIDA) {
						auto SSIDinfo = s.extract<Poco::JSON::Object::Ptr>();
						if(SSIDinfo->isArray("associations") && SSIDinfo->has("phy")) {
							_OWDEBUG_
							auto PHY = SSIDinfo->get("phy").toString();
							int Radio = 2;
							auto Rit = RadioPHYs.find(PHY);
							if(Rit!=RadioPHYs.end())
								Radio = Rit->second;
							auto AssocA = SSIDinfo->getArray("associations");
							if(Radio==2) {
								_OWDEBUG_
								Radios_2G += AssocA->size();
							}
							else {
								_OWDEBUG_
								Radios_5G += AssocA->size();
							}
						}
					}
				}
			}
			return true;
		}
		return false;
	}
}