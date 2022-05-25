//
// Created by stephane bourque on 2022-05-25.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {

	inline bool ParseWifiScan(Poco::JSON::Object::Ptr &Obj, std::stringstream & Result) {
		if(Obj->has("status")) {
			auto Status = Obj->get("status").extract<Poco::JSON::Object::Ptr>();
			if(Status->has("scan") && Status->isArray("scan")) {
				auto ScanArray = Status->getArray("scan");
				Poco::JSON::Array	ParsedScan;
				for(auto &scan_entry:*ScanArray) {
					auto Entry = scan_entry.extract<Poco::JSON::Object::Ptr>();
					if(Entry->has("ies") && Entry->isArray("ies")) {
						auto ies = Entry->getArray("ies");
						Poco::JSON::Array	new_ies;
						for(auto &ie:*ies) {
							auto ie_obj = ie.extract<Poco::JSON::Object::Ptr>();
							std::cout << "IE:" << (uint64_t) ie_obj->get("type") << std::endl;
						}
						Entry->set("ies",new_ies);
						ParsedScan.add(Entry);
					} else {
						ParsedScan.add(scan_entry);
					}
				}
				Status->set("scan",ParsedScan);
				Obj->set("status", Status);
			}
		}
		Obj->stringify(Result);
		return false;
	}

} // namespace OpenWifi

