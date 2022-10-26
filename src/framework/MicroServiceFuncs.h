//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>

#include "framework/OpenWifiTypes.h"

namespace OpenWifi {
	const std::string & MicroServiceDataDirectory();
	Types::MicroServiceMetaVec MicroServiceGetServices(const std::string & Type);
	std::string MicroServicePublicEndPoint();
	std::string MicroServiceConfigGetString(const std::string &Key, const std::string &DefaultValue);
}
