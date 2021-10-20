//
// Created by stephane bourque on 2021-06-17.
//

#include "RESTAPI_ouis.h"
#include "OUIServer.h"
#include "framework/RESTAPI_protocol.h"
#include "framework/Utils.h"

namespace OpenWifi {
	void RESTAPI_ouis::DoGet() {
		Poco::JSON::Array Objects;
		auto Select = GetParameter("macList","");
		std::vector<std::string> Macs = Utils::Split(Select);
		for (auto &i : Macs) {
			Poco::JSON::Object O;
			auto Manufacturer = OUIServer()->GetManufacturer(i);
			O.set("tag", i);
			O.set("value", Manufacturer);
			Objects.add(O);
		}
		Poco::JSON::Object	RetObj;
		RetObj.set("tagList",Objects);
		ReturnObject(RetObj);
	}
}