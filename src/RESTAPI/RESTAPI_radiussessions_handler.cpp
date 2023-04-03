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

		GWObjects::RADIUSSessionList	L;
		RADIUSSessionTracker()->GetAPSessions(SerialNumber,L);
		return ReturnObject("sessions",L.Sessions);
	}

} // namespace OpenWifi