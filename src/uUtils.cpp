//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "uUtils.h"

namespace uCentral::Utils {

	[[nodiscard]] std::vector<std::string> Split(const std::string &List, char Delimiter ) {
		std::vector<std::string> ReturnList;

		unsigned long P=0;

		while(P<List.size())
		{
			unsigned long P2 = List.find_first_of(Delimiter, P);
			if(P2==std::string::npos) {
				ReturnList.push_back(List.substr(P));
				break;
			}
			else
				ReturnList.push_back(List.substr(P,P2-P));
			P=P2+1;
		}
		return ReturnList;
	}

	[[nodiscard]] std::string FormatIPv6(const std::string & I )
	{
		if(I.substr(0,8) == "[::ffff:")
		{
			unsigned long PClosingBracket = I.find_first_of(']');

			std::string ip = I.substr(8, PClosingBracket-8);
			std::string port = I.substr(PClosingBracket+1);
			return ip + port;
		}

		return I;
	}

	[[nodiscard]] std::string SerialToMAC(const std::string &Serial) {
		std::string R = Serial;

		if(R.size()<12)
			padTo(R,12,'0');
		else if (R.size()>12)
			R = R.substr(0,12);

		char buf[18];

		buf[0] = R[0]; buf[1] = R[1] ; buf[2] = ':' ;
		buf[3] = R[2] ; buf[4] = R[3]; buf[5] = ':' ;
		buf[6] = R[4]; buf[7] = R[5] ; buf[8] = ':' ;
		buf[9] = R[6] ; buf[10]= R[7]; buf[11] = ':';
		buf[12] = R[8] ; buf[13]= R[9]; buf[14] = ':';
		buf[15] = R[10] ; buf[16]= R[11];buf[17] = 0;

		return buf;
	}

	[[nodiscard]] std::string ToHex(const std::vector<unsigned char> & B) {
		std::string R;
		R.reserve(B.size()*2);

		static const char hex[] = "0123456789abcdef";

		for(const auto &i:B)
		{
			R += (hex[ (i & 0xf0) >> 4]);
			R += (hex[ (i & 0x0f) ]);
		}

		return R;
	}

	inline static const char kEncodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	inline static const char kPadCharacter = '=';

	std::string base64encode(const byte *input, unsigned long size) {
		std::string encoded;
		encoded.reserve(((size / 3) + (size % 3 > 0)) * 4);

		std::uint32_t temp;

		std::size_t i;

		int ee = (int)(size/3);

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
			temp = input[i] << 16;
			encoded.append(1, kEncodeLookup[(temp & 0x00FC0000) >> 18]);
			encoded.append(1, kEncodeLookup[(temp & 0x0003F000) >> 12]);
			encoded.append(2, kPadCharacter);
			break;
		case 2:
			temp = input[i++] << 16;
			temp += input[i] << 8;
			encoded.append(1, kEncodeLookup[(temp & 0x00FC0000) >> 18]);
			encoded.append(1, kEncodeLookup[(temp & 0x0003F000) >> 12]);
			encoded.append(1, kEncodeLookup[(temp & 0x00000FC0) >> 6]);
			encoded.append(1, kPadCharacter);
			break;
		}

		return encoded;
	}

	std::vector<byte> base64decode(const std::string& input)
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
}
