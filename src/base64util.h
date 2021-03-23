//
// Created by stephane bourque on 2021-03-22.
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
