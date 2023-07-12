//
// Created by stephane bourque on 2023-07-11.
//

#include "RESTAPI_default_firmwares.h"

#include "Poco/Array.h"

#include "RESTAPI_default_firmwares.h"
#include "StorageService.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void RESTAPI_default_firmwares::DoGet() {

		if (QB_.CountOnly) {
			auto Count = StorageService()->GetDefaultFirmwaresCount();
			return ReturnCountOnly(Count);
		}

		std::vector<GWObjects::DefaultFirmware> Firmwares;
		StorageService()->GetDefaultFirmwares(QB_.Offset, QB_.Limit, Firmwares);
		return Object(RESTAPI::Protocol::FIRMWARES, Firmwares);
	}
} // namespace OpenWifi
