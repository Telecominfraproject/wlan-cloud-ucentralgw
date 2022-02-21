//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_commands.h"
#include "StorageService.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void RESTAPI_commands::DoGet() {
		auto SerialNumber = GetParameter(RESTAPI::Protocol::SERIALNUMBER, "");
		std::vector<GWObjects::CommandDetails> Commands;
		if (QB_.Newest) {
			StorageService()->GetNewestCommands(SerialNumber, QB_.Limit, Commands);
		} else {
			StorageService()->GetCommands(SerialNumber, QB_.StartDate, QB_.EndDate, QB_.Offset, QB_.Limit,
								   Commands);
		}
		Poco::JSON::Array ArrayObj;
		for (const auto &i : Commands) {
			Poco::JSON::Object Obj;
			i.to_json(Obj);
			ArrayObj.add(Obj);
		}
		Poco::JSON::Object RetObj;
		RetObj.set(RESTAPI::Protocol::COMMANDS, ArrayObj);
		ReturnObject(RetObj);
	}

	void RESTAPI_commands::DoDelete() {
		auto SerialNumber = GetParameter(RESTAPI::Protocol::SERIALNUMBER, "");

		if(SerialNumber.empty()) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}
		if (StorageService()->DeleteCommands(SerialNumber, QB_.StartDate, QB_.EndDate)) {
			return OK();
		}
		InternalError();
	}
}