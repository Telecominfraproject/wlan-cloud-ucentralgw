//
// Created by stephane bourque on 2022-01-18.
//

#include "StateUtils.h"
#include "Poco/JSON/Parser.h"

namespace OpenWifi::StateUtils {

	static int ChannelToBand(uint64_t C) {
		if(C>=1 && C<=16) return 2;
		return 5;
	}

	bool ComputeAssociations(const Poco::JSON::Object::Ptr & RawObject,
							 uint64_t &Radios_2G,
							 uint64_t &Radios_5G, uint64_t &Radios_6G) {
		Radios_2G = 0 ;
		Radios_5G = 0;
		Radios_6G = 0;

		if(RawObject->isArray("radios") && RawObject->isArray("interfaces")) {
			auto RA = RawObject->getArray("radios");
			// map of phy to 2g/5g
			std::map<std::string,int>   RadioPHYs;
			//  parse radios and get the phy out with the band
			for(auto const &i:*RA) {
				Poco::JSON::Parser p2;
				auto RadioObj = i.extract<Poco::JSON::Object::Ptr>();
				if(RadioObj->has("phy") && RadioObj->has("channel")) {
					if(RadioObj->isArray("channel")) {
						auto ChannelArray = RadioObj->getArray("channel");
						if(ChannelArray->size()) {
							RadioPHYs[RadioObj->get("phy").toString()] =
								ChannelToBand( ChannelArray->getElement<uint64_t>(0) );
						}
					} else {
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
							auto PHY = SSIDinfo->get("phy").toString();
							int Radio = 2;
							auto Rit = RadioPHYs.find(PHY);
							if(Rit!=RadioPHYs.end())
								Radio = Rit->second;
							auto AssocA = SSIDinfo->getArray("associations");
							if(Radio==2) {
								Radios_2G += AssocA->size();
							}
							else {
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