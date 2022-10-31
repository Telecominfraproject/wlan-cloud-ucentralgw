//
// Created by stephane bourque on 2022-10-25.
//

#include "RESTAPI_IntServer.h"

namespace OpenWifi {

	Poco::Net::HTTPRequestHandler *
	IntRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &Request) {
		auto TID = NextTransactionId_++;
		Utils::SetThreadName(fmt::format("i-rest:{}", TID).c_str());
		Poco::URI uri(Request.getURI());
		return RESTAPI_IntServer()->CallServer(uri.getPath(), TID);
	}

} // namespace OpenWifi