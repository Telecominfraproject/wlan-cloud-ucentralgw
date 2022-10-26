//
// Created by stephane bourque on 2022-10-25.
//

#include "framework/MicroService.h"
#include "framework/MicroServiceFuncs.h"


namespace OpenWifi {
	const std::string & MicroServiceDataDirectory() {
		return MicroService::instance().DataDir();
	}

	Types::MicroServiceMetaVec MicroServiceGetServices(const std::string &Type) {
		return MicroService::instance().GetServices(Type);
	}

	std::string MicroServicePublicEndPoint() {
		return MicroService::instance().PublicEndPoint();
	}

	std::string MicroServiceConfigGetString(const std::string &Key, const std::string &DefaultValue) {
		return MicroService::instance().ConfigGetString(Key, DefaultValue);
	}
}
