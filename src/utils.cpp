//
// Created by stephane bourque on 2021-05-03.
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

			std::string ip = I.substr(9, PClosingBracket-9);
			std::string port = I.substr(PClosingBracket+1);
			return ip + port;
		}

		return I;
	}
}
