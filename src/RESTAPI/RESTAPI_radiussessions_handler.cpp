//
// Created by stephane bourque on 2023-04-02.
//

#include "RESTAPI_radiussessions_handler.h"
#include <RESTObjects/RESTAPI_GWobjects.h>
#include <RADIUSSessionTracker.h>

namespace OpenWifi {

	void RESTAPI_radiussessions_handler::DoGet() {

		if(GetBoolParameter("serialNumberOnly")) {
			std::vector<std::string>	L;
			RADIUSSessionTracker()->GetAPList(L);
			return ReturnObject("serialNumbers",L);
		}

		auto SerialNumber = GetBinding("serialNumber","");
		if(SerialNumber.empty() || Utils::ValidSerialNumber(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::RADIUSSessionList	L;
		RADIUSSessionTracker()->GetAPSessions(SerialNumber,L);
		return ReturnObject("sessions",L.sessions);
	}

	void RESTAPI_radiussessions_handler::DoPut() {
		auto SerialNumber = GetBinding("serialNumber","");
		if(SerialNumber.empty() || Utils::ValidSerialNumber(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}


	}

} // namespace OpenWifi