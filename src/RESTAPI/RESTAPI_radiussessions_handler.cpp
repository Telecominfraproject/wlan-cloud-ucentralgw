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

		auto userName = GetParameter("userName","");
		if(!userName.empty()) {
			GWObjects::RADIUSSessionList	L;
			Poco::toLowerInPlace(userName);
			RADIUSSessionTracker()->GetUserNameAPSessions(userName,L);
			return ReturnObject("sessions",L.sessions);
		}

		auto mac = GetParameter("mac","");
		Poco::toUpperInPlace(mac);
		Poco::replaceInPlace(mac,":","-");
		if(!userName.empty()) {
			GWObjects::RADIUSSessionList	L;
			Poco::toLowerInPlace(userName);
			RADIUSSessionTracker()->GetMACAPSessions(mac,L);
			return ReturnObject("sessions",L.sessions);
		}

		auto SerialNumber = GetBinding("serialNumber","");
		if(SerialNumber.empty() || !Utils::ValidSerialNumber(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::RADIUSSessionList	L;
		RADIUSSessionTracker()->GetAPSessions(SerialNumber,L);
		return ReturnObject("sessions",L.sessions);
	}

	void RESTAPI_radiussessions_handler::DoPut() {
		auto SerialNumber = GetBinding("serialNumber","");
		if(SerialNumber.empty() || !Utils::ValidSerialNumber(SerialNumber)) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::RadiusCoADMParameters	Parameters;
		if(!Parameters.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if(Parameters.callingStationId.empty() || Parameters.accountingSessionId.empty() || Parameters.accountingMultiSessionId.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		auto Command = GetParameter("operation","");

		if(Command=="coadm") {
			if(RADIUSSessionTracker()->SendCoADM(SerialNumber, Parameters.accountingSessionId)) {
				return OK();
			}
			return BadRequest(RESTAPI::Errors::CouldNotPerformCommand);
		}

		return BadRequest(RESTAPI::Errors::InvalidCommand);
	}

} // namespace OpenWifi