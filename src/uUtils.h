//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_UUTILS_H
#define UCENTRALGW_UUTILS_H

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

	using byte = std::uint8_t;

	[[nodiscard]] std::string base64encode(const byte *input, unsigned long size);
	std::vector<byte> base64decode(const std::string& input);

//	[[nodiscard]] std::string to_RFC3339(uint64_t t);
//	[[nodiscard]] uint64_t from_RFC3339(const std::string &t);

	bool ParseTime(const std::string &Time, int & Hours, int & Minutes, int & Seconds);
	bool ParseDate(const std::string &Time, int & Year, int & Month, int & Day);
	bool CompareTime( int H1, int H2, int M1, int M2, int S1, int S2);

	[[nodiscard]] bool ValidSerialNumber(const std::string &Serial);
}
#endif // UCENTRALGW_UUTILS_H
