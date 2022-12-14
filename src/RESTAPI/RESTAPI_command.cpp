//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_command.h"

#include "StorageService.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void RESTAPI_command::DoGet() {
		auto CommandUUID = GetBinding(RESTAPI::Protocol::COMMANDUUID, "");
		if(!Utils::ValidUUID(CommandUUID)) {
			return NotFound();
		}

		GWObjects::CommandDetails Command;
		if (StorageService()->GetCommand(CommandUUID, Command)) {
			return Object(Command);
		}
		return NotFound();
	}

	void RESTAPI_command::DoDelete() {
		auto CommandUUID = GetBinding(RESTAPI::Protocol::COMMANDUUID, "");
		if(CommandUUID.empty()) {
			return BadRequest(RESTAPI::Errors::MissingUUID);
		}

		if(!Utils::ValidUUID(CommandUUID)) {
			return NotFound();
		}

		GWObjects::CommandDetails	C;
		if(!StorageService()->GetCommand(CommandUUID, C)) {
			return NotFound();
		}

		if (StorageService()->DeleteCommand(CommandUUID)) {
			return OK();
		}
		return InternalError(RESTAPI::Errors::NoRecordsDeleted);
	}
}