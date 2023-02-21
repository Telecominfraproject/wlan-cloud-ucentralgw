//
// Created by stephane bourque on 2022-01-18.
//

#pragma once

#include "Poco/JSON/Object.h"

namespace OpenWifi::StateUtils {
	bool ComputeAssociations(const Poco::JSON::Object::Ptr RawObject, uint64_t &Radios_2G,
							 uint64_t &Radios_5G, uint64_t &Radio_6G);
}