//
// Created by stephane bourque on 2022-11-21.
//

#include "RESTAPI_scripts_handler.h"
#include "RESTObjects/RESTAPI_GWobjects.h"
#include "StorageService.h"

namespace OpenWifi {

	void RESTAPI_scripts_handler::DoGet() {
		GWObjects::ScriptEntryList	L;

		StorageService()->ScriptDB().GetRecords(QB_.Offset,QB_.Limit,L.scripts);
		Poco::JSON::Object	Answer;
		L.to_json(Answer);
		return ReturnObject(Answer);
	}

} // namespace OpenWifi
