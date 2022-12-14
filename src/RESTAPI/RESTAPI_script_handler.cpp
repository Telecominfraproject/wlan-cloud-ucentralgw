//
// Created by stephane bourque on 2022-11-21.
//

#include "RESTAPI_script_handler.h"

namespace OpenWifi {

	void RESTAPI_script_handler::DoGet() {
		std::string 	UUID = GetBinding("uuid","");

		if(UUID.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::ScriptEntry	SE;
		if(DB_.GetRecord("id",UUID, SE)) {
			return Object(SE);
		}
		return NotFound();
	}

	void RESTAPI_script_handler::DoDelete() {
		std::string 	UUID = GetBinding("uuid","");

		if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
			return BadRequest(RESTAPI::Errors::ACCESS_DENIED);
		}

		if(UUID.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if(DB_.DeleteRecord("id",UUID)) {
			return OK();
		}
		return NotFound();
	}

	void RESTAPI_script_handler::DoPost() {
		std::string 	UUID = GetBinding("uuid","");

		if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
			return BadRequest(RESTAPI::Errors::ACCESS_DENIED);
		}

		if(UUID.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::ScriptEntry	SE;
		if(!SE.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if(	SE.name.empty() ||
			SE.author.empty()	||
			(SE.type!="bundle" && SE.type!="shell") ||
			SE.content.empty() ||
			SE.version.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if(!SE.restricted.empty()) {
			for(const auto &role:SE.restricted) {
				if(SecurityObjects::UserTypeFromString(role)==SecurityObjects::UNKNOWN) {
					return BadRequest(RESTAPI::Errors::InvalidUserRole);
				}
			}
			std::sort(SE.restricted.begin(),SE.restricted.end());
		}

		if(!SE.uri.empty() && !Utils::ValidateURI(SE.uri)) {
			return BadRequest(RESTAPI::Errors::InvalidURI);
		}

		if(!SE.defaultUploadURI.empty() && !Utils::ValidateURI(SE.defaultUploadURI)) {
			return BadRequest(RESTAPI::Errors::InvalidURI);
		}

		SE.id = MicroServiceCreateUUID();
		SE.created = SE.modified = Utils::Now();
		if(DB_.CreateRecord(SE)) {
			return Object(SE);
		}
		return BadRequest(RESTAPI::Errors::RecordNotCreated);
	}

	void RESTAPI_script_handler::DoPut() {
		std::string 	UUID = GetBinding("uuid","");

		if(UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
			return BadRequest(RESTAPI::Errors::ACCESS_DENIED);
		}

		if(UUID.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::ScriptEntry	SE;
		if(!SE.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if(!SE.restricted.empty()) {
			for(const auto &role:SE.restricted) {
				if(SecurityObjects::UserTypeFromString(role)==SecurityObjects::UNKNOWN) {
					return BadRequest(RESTAPI::Errors::InvalidUserRole);
				}
			}
			std::sort(SE.restricted.begin(),SE.restricted.end());
		}

		GWObjects::ScriptEntry	Existing;
		if(!DB_.GetRecord("id", UUID, Existing)) {
			return NotFound();
		}

		if(ParsedBody_->has("name") && SE.name.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if(ParsedBody_->has("content") && SE.content.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if(ParsedBody_->has("version") && SE.version.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if(!SE.uri.empty() && !Utils::ValidateURI(SE.uri)) {
			return BadRequest(RESTAPI::Errors::InvalidURI);
		}

		if(!SE.defaultUploadURI.empty() && !Utils::ValidateURI(SE.defaultUploadURI)) {
			return BadRequest(RESTAPI::Errors::InvalidURI);
		}

		if(ParsedBody_->has("restricted")) {
			Existing.restricted = SE.restricted;
		}

		AssignIfPresent(ParsedBody_, "name", Existing.name);
		AssignIfPresent(ParsedBody_, "description", Existing.description);
		AssignIfPresent(ParsedBody_, "uri", Existing.uri);
		AssignIfPresent(ParsedBody_, "content", Existing.content);
		AssignIfPresent(ParsedBody_, "version", Existing.version);
		AssignIfPresent(ParsedBody_, "deferred", Existing.deferred);
		AssignIfPresent(ParsedBody_, "timeout", Existing.timeout);
		AssignIfPresent(ParsedBody_, "defaultUploadURI", Existing.defaultUploadURI);

		Existing.modified = Utils::Now();

		if(DB_.UpdateRecord("id", UUID, Existing)) {
			return Object(Existing);
		}
		return BadRequest(RESTAPI::Errors::RecordNotUpdated);
	}

} // namespace OpenWifi