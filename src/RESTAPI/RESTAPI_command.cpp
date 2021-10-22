//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_command.h"

#include "StorageService.h"
#include "framework/RESTAPI_errors.h"
#include "framework/RESTAPI_protocol.h"

namespace OpenWifi {
	void RESTAPI_command::DoGet() {
		auto CommandUUID = GetBinding(RESTAPI::Protocol::COMMANDUUID, "");
		GWObjects::CommandDetails Command;
		if (StorageService()->GetCommand(CommandUUID, Command)) {
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
		if(!StorageService()->GetCommand(UUID, C)) {
			return NotFound();
		}

		if (StorageService()->DeleteCommand(UUID)) {
			return OK();
		}
		return InternalError();
	}
}