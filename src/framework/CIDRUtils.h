//
// Created by stephane bourque on 2021-09-29.
//

#ifndef OWPROV_CIDRUTILS_H
#define OWPROV_CIDRUTILS_H

#include "framework/OpenWifiTypes.h"

namespace OpenWifi::CIDR {
    [[nodiscard]] bool IpInRanges(const std::string &IP, const Types::StringVec &R);
    [[nodiscard]] bool ValidateIpRanges(const Types::StringVec & Ranges);
}


#endif //OWPROV_CIDRUTILS_H
