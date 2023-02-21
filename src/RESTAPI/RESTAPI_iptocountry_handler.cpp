//
// Created by stephane bourque on 2022-02-05.
//

#include "RESTAPI_iptocountry_handler.h"
#include "FindCountry.h"

namespace OpenWifi {

	void RESTAPI_iptocountry_handler::DoGet() {
		auto IPList = GetParameter("iplist", "");

		if (IPList.empty()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		auto IPAddresses = Poco::StringTokenizer(IPList, ",");
		Poco::JSON::Object Answer;

		Answer.set("enabled", FindCountryFromIP()->Enabled());
		Poco::JSON::Array Countries;

		for (const auto &i : IPAddresses) {
			Countries.add(FindCountryFromIP()->Get(i));
		}
		Answer.set("countryCodes", Countries);

		return ReturnObject(Answer);
	}

} // namespace OpenWifi