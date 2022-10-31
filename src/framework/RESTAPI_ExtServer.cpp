//
// Created by stephane bourque on 2022-10-25.
//

#include "framework/RESTAPI_ExtServer.h"

namespace OpenWifi {

	Poco::Net::HTTPRequestHandler *ExtRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &Request) {
		try {
			Poco::URI uri(Request.getURI());
			auto TID = NextTransactionId_++;
			Utils::SetThreadName(fmt::format("x-rest:{}",TID).c_str());
			return RESTAPI_ExtServer()->CallServer(uri.getPath(), TID);
		} catch (...) {

		}
		return nullptr;
	}

	Poco::Net::HTTPRequestHandler *RESTAPI_ExtServer::CallServer(const std::string &Path, uint64_t Id) {
		RESTAPIHandler::BindingMap Bindings;
		Utils::SetThreadName(fmt::format("x-rest:{}",Id).c_str());
		return RESTAPI_ExtRouter(Path, Bindings, Logger(), Server_, Id);
	}

}
