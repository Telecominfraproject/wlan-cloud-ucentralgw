//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#ifndef UCENTRAL_BASE64UTIL_H
#define UCENTRAL_BASE64UTIL_H

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>

namespace base64 {
    using byte = std::uint8_t;
    std::string encode(const byte *input, unsigned long size);
    std::vector<byte> decode(const std::string& input);
};


#endif //UCENTRAL_BASE64UTIL_H
