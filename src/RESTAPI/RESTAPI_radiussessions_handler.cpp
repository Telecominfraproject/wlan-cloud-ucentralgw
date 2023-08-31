//
// Created by stephane bourque on 2023-04-02.
//

#include "RESTAPI_radiussessions_handler.h"
#include <RESTObjects/RESTAPI_GWobjects.h>
#include <RADIUSSessionTracker.h>

namespace OpenWifi {

	bool MayBeAMAC(const std::string &mac) {
		return std::all_of(mac.begin(),mac.end(),[](char c)->bool {
			if ((c>='0' && c<='9') 	||
				(c>='a' && c<='f') 	||
				(c>='A' && c<='F')	||
				(c==':') ||
				(c=='-') ||
				(c=='*')) return true;
			return false;
			});
	}

	std::string InsertDelimiters(const std::string &mac, int first=1, char delimiter=':') {
		std::string res;
		std::size_t index=0;
		for(auto c:mac) {
			res += c;
			index++;
			if(index<mac.size()) {
				if (!first)
					res += delimiter;
			}
			first = 1-first;
		}
		return res;
	}

	std::string StripDelimiters(const std::string &V) {
		std::string Res;
		std::for_each(V.begin(),V.end(),[&](char c){ if(c!=':' && c!='-') { Res += c; }});
		return Res;
	}

	static std::string ConvertToMac(const std::string & V) {
		auto res = V;
		Poco::toUpperInPlace(res);
		res = StripDelimiters(res);
		if(res.size()==12) {
			res = InsertDelimiters(res);
		} else {
			if(res.find_first_of('*')==std::string::npos) {
				return "";
			}
			if(res[0]=='*') {
				res = InsertDelimiters(res, 1 - (res.size() % 2) );
			} else {
				res = InsertDelimiters(res);
			}
		}
		return res;
	}

	void RESTAPI_radiussessions_handler::DoGet() {

		if(GetBoolParameter("serialNumberOnly")) {
			std::vector<std::string>	L;
			RADIUSSessionTracker()->GetAPList(L);
			return ReturnObject("serialNumbers",L);
		}

		auto mac = GetParameter("mac","");
		auto userName = GetParameter("userName","");
		if(!userName.empty()) {
			GWObjects::RADIUSSessionList	L;
			Poco::toLowerInPlace(userName);
			RADIUSSessionTracker()->GetUserNameAPSessions(userName,L);
			if(L.sessions.empty() && MayBeAMAC(userName)) {
				mac = ConvertToMac(userName);
			} else {
				return ReturnObject("sessions", L.sessions);
			}
		}

		if(!mac.empty()) {
			Poco::toUpperInPlace(mac);
			Poco::replaceInPlace(mac,":","-");
			GWObjects::RADIUSSessionList	L;
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
			auto Index = Parameters.accountingSessionId + Parameters.accountingMultiSessionId;
			poco_information(Logger(), fmt::format("Disconnecting session {},{}", Parameters.accountingSessionId, Parameters.accountingMultiSessionId ));
			if(RADIUSSessionTracker()->SendCoADM(SerialNumber, Index)) {
				return OK();
			}
			return BadRequest(RESTAPI::Errors::CouldNotPerformCommand);
		}

		if(Command=="disconnectUser" && !Parameters.userName.empty()) {
			poco_information(Logger(), fmt::format("Disconnecting sessions for user: {}", Parameters.userName ));
			if(RADIUSSessionTracker()->DisconnectUser(Parameters.userName)) {
				return OK();
			}
			return BadRequest(RESTAPI::Errors::CouldNotPerformCommand);
		}

		return BadRequest(RESTAPI::Errors::InvalidCommand);
	}

} // namespace OpenWifi