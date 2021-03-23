//
// Created by stephane bourque on 2021-03-22.
//

#include "base64util.h"

namespace base64 {

    inline static const char kEncodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    inline static const char kPadCharacter = '=';

    std::string encode(const byte *input, unsigned long size) {
        std::string encoded;
        encoded.reserve(((size / 3) + (size % 3 > 0)) * 4);

        std::uint32_t temp{};

        std::size_t i;

        int ee = size/3;

        for (i = 0; i < 3*ee; ++i) {
            temp = input[i++] << 16;
            temp += input[i++] << 8;
            temp += input[i];
            encoded.append(1, kEncodeLookup[(temp & 0x00FC0000) >> 18]);
            encoded.append(1, kEncodeLookup[(temp & 0x0003F000) >> 12]);
            encoded.append(1, kEncodeLookup[(temp & 0x00000FC0) >> 6]);
            encoded.append(1, kEncodeLookup[(temp & 0x0000003F)]);
        }

        switch (size % 3) {
            case 1:
                temp = input[i++] << 16;
                encoded.append(1, kEncodeLookup[(temp & 0x00FC0000) >> 18]);
                encoded.append(1, kEncodeLookup[(temp & 0x0003F000) >> 12]);
                encoded.append(2, kPadCharacter);
                break;
            case 2:
                temp = input[i++] << 16;
                temp += input[i++] << 8;
                encoded.append(1, kEncodeLookup[(temp & 0x00FC0000) >> 18]);
                encoded.append(1, kEncodeLookup[(temp & 0x0003F000) >> 12]);
                encoded.append(1, kEncodeLookup[(temp & 0x00000FC0) >> 6]);
                encoded.append(1, kPadCharacter);
                break;
        }

        return encoded;
    }

    std::vector<byte> decode(const std::string& input)
    {
        if(input.length() % 4)
            throw std::runtime_error("Invalid base64 length!");

        std::size_t padding{};

        if(input.length())
        {
            if(input[input.length() - 1] == kPadCharacter) padding++;
            if(input[input.length() - 2] == kPadCharacter) padding++;
        }

        std::vector<byte> decoded;
        decoded.reserve(((input.length() / 4) * 3) - padding);

        std::uint32_t temp{};
        auto it = input.begin();

        while(it < input.end())
        {
            for(std::size_t i = 0; i < 4; ++i)
            {
                temp <<= 6;
                if     (*it >= 0x41 && *it <= 0x5A) temp |= *it - 0x41;
                else if(*it >= 0x61 && *it <= 0x7A) temp |= *it - 0x47;
                else if(*it >= 0x30 && *it <= 0x39) temp |= *it + 0x04;
                else if(*it == 0x2B)                temp |= 0x3E;
                else if(*it == 0x2F)                temp |= 0x3F;
                else if(*it == kPadCharacter)
                {
                    switch(input.end() - it)
                    {
                        case 1:
                            decoded.push_back((temp >> 16) & 0x000000FF);
                            decoded.push_back((temp >> 8 ) & 0x000000FF);
                            return decoded;
                        case 2:
                            decoded.push_back((temp >> 10) & 0x000000FF);
                            return decoded;
                        default:
                            throw std::runtime_error("Invalid padding in base64!");
                    }
                }
                else throw std::runtime_error("Invalid character in base64!");

                ++it;
            }

            decoded.push_back((temp >> 16) & 0x000000FF);
            decoded.push_back((temp >> 8 ) & 0x000000FF);
            decoded.push_back((temp      ) & 0x000000FF);
        }

        return decoded;
    }

};
