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

	static int BandToInt(const std::string &band) {
		if(band=="2G") return 2;
		if(band=="5G") return 5;
		if(band=="6G") return 6;
		return 2;
	}

	bool ComputeAssociations(const Poco::JSON::Object::Ptr RawObject,
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
			bool UseBandInfo = false;
			for(auto const &i:*RA) {
//				Poco::JSON::Parser p2;
				auto RadioObj = i.extract<Poco::JSON::Object::Ptr>();
				if(RadioObj->has("band")) {
					// std::cout << "Use band info" << std::endl;
					UseBandInfo = true ;
				} else if(RadioObj->has("phy") && RadioObj->has("channel")) {
					if(RadioObj->isArray("channel")) {
						auto ChannelArray = RadioObj->getArray("channel");
						if(ChannelArray->size()) {
							RadioPHYs[RadioObj->get("phy")] =
								ChannelToBand( ChannelArray->getElement<uint64_t>(0) );
						}
					} else {
						RadioPHYs[RadioObj->get("phy")] =
							ChannelToBand(RadioObj->get("channel"));
					}
				}
			}

			auto InterfaceArray = RawObject->getArray("interfaces");
			for(auto const &interface:*InterfaceArray) {
				auto InterfaceObj = interface.extract<Poco::JSON::Object::Ptr>();
				if(InterfaceObj->isArray("ssids")) {
					auto SSIDArray = InterfaceObj->getArray("ssids");
					for(const auto &ssid:*SSIDArray) {
						auto SSID_info = ssid.extract<Poco::JSON::Object::Ptr>();
						if(SSID_info->isArray("associations") && SSID_info->has("phy")) {
							int Radio = 2;
							if(UseBandInfo) {
								Radio = BandToInt(SSID_info->get("band"));
							} else {
								auto PHY = SSID_info->get("phy");
								auto Rit = RadioPHYs.find(PHY);
								if (Rit != RadioPHYs.end())
									Radio = Rit->second;
							}
							auto AssocA = SSID_info->getArray("associations");
							switch(Radio) {
								case 2: Radios_2G += AssocA->size(); break;
								case 5: Radios_5G += AssocA->size(); break;
								case 6: Radios_6G += AssocA->size(); break;
								default: Radios_2G += AssocA->size(); break;
							}
						}
					}
				}
			}
//			std::cout << Radios_2G << " " << Radios_5G << " " << Radios_6G << std::endl;
			return true;
		}
		return false;
	}
}