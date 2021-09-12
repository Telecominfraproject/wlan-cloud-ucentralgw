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

namespace OpenWifi {
	void RESTAPI_command::DoGet() {
		auto CommandUUID = GetBinding(RESTAPI::Protocol::COMMANDUUID, "");
		GWObjects::CommandDetails Command;
		if (Storage()->GetCommand(CommandUUID, Command)) {
			Poco::JSON::Object RetObj;
			Command.to_json(RetObj);
			ReturnObject(RetObj);
		} else
			NotFound();
	}

	void RESTAPI_command::DoDelete() {
		auto CommandUUID = GetBinding(RESTAPI::Protocol::COMMANDUUID, "");
		if (Storage()->DeleteCommand(CommandUUID)) {
			OK();
		} else {
			NotFound();
		}
	}
}