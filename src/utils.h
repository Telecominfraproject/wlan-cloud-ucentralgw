//
// Created by stephane bourque on 2021-05-03.
//

#ifndef UCENTRALGW_UTILS_H
#define UCENTRALGW_UTILS_H

#include <vector>
#include <string>

namespace uCentral::Utils {

	[[nodiscard]] std::vector<std::string> Split(const std::string &List, char Delimiter=',');
	[[nodiscard]] std::string FormatIPv6(const std::string & I );
	inline void padTo(std::string& str, size_t num, char paddingChar = '\0') {
		str.append(num - str.length() % num, paddingChar);
	}

	[[nodiscard]] std::string SerialToMAC(const std::string &Serial);
	[[nodiscard]] std::string ToHex(const std::vector<unsigned char> & B);
}
#endif // UCENTRALGW_UTILS_H
