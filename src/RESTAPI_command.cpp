//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_command.h"

#include "RESTAPI_protocol.h"
#include "StorageService.h"
#include "RESTAPI_errors.h"

namespace OpenWifi {
	void RESTAPI_command::DoGet() {
		auto CommandUUID = GetBinding(RESTAPI::Protocol::COMMANDUUID, "");
		GWObjects::CommandDetails Command;
		if (Storage()->GetCommand(CommandUUID, Command)) {
			Poco::JSON::Object RetObj;
			Command.to_json(RetObj);
			return ReturnObject(RetObj);
		}
		return NotFound();
	}

	void RESTAPI_command::DoDelete() {
		auto UUID = GetBinding(RESTAPI::Protocol::COMMANDUUID, "");

		if(UUID.empty()) {
			return BadRequest(RESTAPI::Errors::MissingUUID);
		}

		GWObjects::CommandDetails	C;
		if(!Storage()->GetCommand(UUID, C)) {
			return NotFound();
		}

		if (Storage()->DeleteCommand(UUID)) {
			return OK();
		}
		return InternalError();
	}
}