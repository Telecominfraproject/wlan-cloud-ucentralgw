//
// Created by stephane bourque on 2022-07-16.
//

#pragma once

#include "RESTObjects/RESTAPI_ProvObjects.h"

#include "framework/OpenAPIRequests.h"

namespace OpenWifi::SDK::Prov {

	inline bool GetSerialNumbersForVenueOfSerialNumber( const std::string & SerialNumber, Types::UUID_t &Venue, Types::StringVec & AdjacentSerialNumbers , Poco::Logger &Logger ) {
		OpenAPIRequestGet	GetInventoryForSerialNumber( uSERVICE_PROVISIONING, "/api/v1/inventory/" + SerialNumber , {} , 30000);

		auto CallResponse1 = Poco::makeShared<Poco::JSON::Object>();
		if(!GetInventoryForSerialNumber.Do(CallResponse1,"")) {
			Logger.error(fmt::format("{}: Cannot find serial number in inventory.", SerialNumber));
			return false;
		}

		ProvObjects::InventoryTag	Device;
		if(!Device.from_json(CallResponse1)) {
			Logger.error(fmt::format("{}: Invalid Inventory response.", SerialNumber));
			return false;
		}

		Venue = Device.venue;

		OpenAPIRequestGet	GetInventoryForVenue( uSERVICE_PROVISIONING, "/api/v1/inventory" ,
											   {
												   {"serialOnly","true"},
												   {"venue", Venue}
											   }, 30000);

		auto CallResponse2 = Poco::makeShared<Poco::JSON::Object>();
		if(!GetInventoryForVenue.Do(CallResponse2,"")) {
			Logger.error(fmt::format("{}: Cannot get inventory for venue.", SerialNumber));
			return false;
		}

		try {
			OpenWifi::RESTAPI_utils::field_from_json(CallResponse2, "serialNumbers",
													 AdjacentSerialNumbers);
		} catch(...) {
			Logger.error(fmt::format("{}: Cannot parse inventory list", SerialNumber));
			return false;
		}

		return true;
	}

}