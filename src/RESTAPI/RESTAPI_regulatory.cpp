//
// Created by stephane bourque on 2023-02-24.
//

#include "RESTAPI_regulatory.h"
#include "framework/ow_constants.h"
#include "RegulatoryInfo.h"

namespace OpenWifi {
	void RESTAPI_regulatory::DoGet() {

		auto CountryList = GetParameter("countries","");
		if(CountryList.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		std::vector<GWObjects::RegulatoryCountryInfo>	Answers;
		RegulatoryInfo()->Get(CountryList, Answers);
		return ReturnObject("countries", Answers);

	}

} // namespace OpenWifi