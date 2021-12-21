//
// Created by stephane bourque on 2021-12-20.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
	class SDKCalls {
	  public:
		static bool GetProvisioningConfiguration(const std::string & SerialNumber, std::string & Config);
	  private:
	};
}

