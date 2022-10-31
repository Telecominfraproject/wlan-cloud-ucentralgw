//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>

#include "Poco/Net/IPAddress.h"
#include "Poco/StringTokenizer.h"

#include "framework/OpenWifiTypes.h"

namespace OpenWifi::CIDR {

	static bool cidr_match(const in_addr &addr, const in_addr &net, uint8_t bits) {
		if (bits == 0) {
			return true;
		}
		return !((addr.s_addr ^ net.s_addr) & htonl(0xFFFFFFFFu << (32 - bits)));
	}

	static bool cidr6_match(const in6_addr &address, const in6_addr &network, uint8_t bits) {
	#ifdef __linux__
		const uint32_t *a = address.s6_addr32;
		const uint32_t *n = network.s6_addr32;
	#else
		const uint32_t *a = address.__u6_addr.__u6_addr32;
		const uint32_t *n = network.__u6_addr.__u6_addr32;
	#endif
		int bits_whole, bits_incomplete;
		bits_whole = bits >> 5;		   // number of whole u32
		bits_incomplete = bits & 0x1F; // number of bits in incomplete u32
		if (bits_whole) {
			if (memcmp(a, n, bits_whole << 2) != 0) {
				return false;
			}
		}
		if (bits_incomplete) {
			uint32_t mask = htonl((0xFFFFFFFFu) << (32 - bits_incomplete));
			if ((a[bits_whole] ^ n[bits_whole]) & mask) {
				return false;
			}
		}
		return true;
	}

	static bool ConvertStringToLong(const char *S, unsigned long &L) {
		char *end;
		L = std::strtol(S, &end, 10);
		return end != S;
	}

	static bool CidrIPinRange(const Poco::Net::IPAddress &IP, const std::string &Range) {
		Poco::StringTokenizer TimeTokens(Range, "/", Poco::StringTokenizer::TOK_TRIM);

		Poco::Net::IPAddress RangeIP;
		if (Poco::Net::IPAddress::tryParse(TimeTokens[0], RangeIP)) {
			if (TimeTokens.count() == 2) {
				if (RangeIP.family() == Poco::Net::IPAddress::IPv4) {
					unsigned long MaskLength;
					if (ConvertStringToLong(TimeTokens[1].c_str(), MaskLength)) {
						return cidr_match(*static_cast<const in_addr *>(RangeIP.addr()),
										  *static_cast<const in_addr *>(IP.addr()), MaskLength);
					}
				} else if (RangeIP.family() == Poco::Net::IPAddress::IPv6) {
					unsigned long MaskLength;
					if (ConvertStringToLong(TimeTokens[1].c_str(), MaskLength)) {
						return cidr6_match(*static_cast<const in6_addr *>(RangeIP.addr()),
										   *static_cast<const in6_addr *>(IP.addr()), MaskLength);
					}
				}
			}
			return false;
		}
		return false;
	}

	//
	//  Ranges can be a single IP, of IP1-IP2, of A set of IPs: IP1,IP2,IP3, or a cidr IP/24
	//  These can work for IPv6 too...
	//
	static bool ValidateRange(const std::string &R) {

		auto Tokens = Poco::StringTokenizer(R, "-");
		if (Tokens.count() == 2) {
			Poco::Net::IPAddress a, b;
			if (!Poco::Net::IPAddress::tryParse(Tokens[0], a) &&
				Poco::Net::IPAddress::tryParse(Tokens[1], b))
				return false;
			return a.family() == b.family();
		}

		Tokens = Poco::StringTokenizer(R, ",");
		if (Tokens.count() > 1) {
			return std::all_of(Tokens.begin(), Tokens.end(), [](const std::string &A) {
				Poco::Net::IPAddress a;
				return Poco::Net::IPAddress::tryParse(A, a);
			});
		}

		Tokens = Poco::StringTokenizer(R, "/");
		if (Tokens.count() == 2) {
			Poco::Net::IPAddress a;
			if (!Poco::Net::IPAddress::tryParse(Tokens[0], a))
				return false;
			if (std::atoi(Tokens[1].c_str()) == 0)
				return false;
			return true;
		}

		Poco::Net::IPAddress a;
		return Poco::Net::IPAddress::tryParse(R, a);
	}

	static bool IpInRange(const Poco::Net::IPAddress &target, const std::string &R) {

		auto Tokens = Poco::StringTokenizer(R, "-");
		if (Tokens.count() == 2) {
			auto a = Poco::Net::IPAddress::parse(Tokens[0]);
			auto b = Poco::Net::IPAddress::parse(Tokens[1]);
			if (target.family() != a.family())
				return false;
			return (a <= target && b >= target);
		}

		Tokens = Poco::StringTokenizer(R, ",");
		if (Tokens.count() > 1) {
			return std::any_of(Tokens.begin(), Tokens.end(), [target](const std::string &Element) {
				return Poco::Net::IPAddress::parse(Element) == target;
			});
		}

		Tokens = Poco::StringTokenizer(R, "/");
		if (Tokens.count() == 2) {
			return CidrIPinRange(target, R);
		}

		return Poco::Net::IPAddress::parse(R) == target;
	}

	[[nodiscard]] inline bool IpInRanges(const std::string &IP, const Types::StringVec &R) {
		Poco::Net::IPAddress Target;

		if (!Poco::Net::IPAddress::tryParse(IP, Target))
			return false;

		return std::any_of(cbegin(R), cend(R),
						   [Target](const std::string &i) { return IpInRange(Target, i); });
	}

	[[nodiscard]] inline bool ValidateIpRanges(const Types::StringVec &Ranges) {
		return std::all_of(cbegin(Ranges), cend(Ranges), ValidateRange);
	}
}