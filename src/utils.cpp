//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "utils.h"

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


}
