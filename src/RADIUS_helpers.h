//
// Created by stephane bourque on 2022-06-20.
//

#pragma once

#include <iomanip>
#include <iostream>
#include <list>
#include <string>

#include "Poco/HMACEngine.h"
#include "Poco/MD5Engine.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/StringTokenizer.h"

namespace OpenWifi::RADIUS {

#define RADCMD_ACCESS_REQ 1		  /* Access-Request      */
#define RADCMD_ACCESS_ACC 2		  /* Access-Accept       */
#define RADCMD_ACCESS_REJ 3		  /* Access-Reject       */
#define RADCMD_ACCOUN_REQ 4		  /* Accounting-Request  */
#define RADCMD_ACCOUN_RES 5		  /* Accounting-Response */
#define RADCMD_ACCOUN_STATUS 6	  /* Accounting-Status */
#define RADCMD_PASSWORD_REQUEST 7 /* Password-Request	[RFC3575] */
#define RADCMD_PASSWORD_ACK 8	  /* Password-Ack	[RFC3575] */
#define RADCMD_PASSWORD_REJECT 9  /* Password-Reject	[RFC3575] */
#define RADCMD_ACCOUN_MESSAGE 10  /* Accounting-Message	[RFC3575] */

#define RADCMD_RES_FREE_REQ 21		  /* Resource-Free-Request	[RFC3575] */
#define RADCMD_RES_FREE_RES 22		  /* Resource-Free-Response	[RFC3575] */
#define RADCMD_RES_QUERY_REQ 23		  /* Resource-Query-Request	[RFC3575] */
#define RADCMD_RES_QUERY_RES 24		  /* Resource-Query-Response	[RFC3575] */
#define RADCMD_RES_ALT_RECLAIM_REQ 25 /* Alternate-Resource-Reclaim-Request	[RFC3575] */

#define RADCMD_ACCESS_CHA 11 /* Access-Challenge    */
#define RADCMD_STATUS_SER 12 /* Status-Server       */
#define RADCMD_STATUS_CLI 13 /* Status-Client       */
#define RADCMD_DISCON_REQ 40 /* Disconnect-Request  */
#define RADCMD_DISCON_ACK 41 /* Disconnect-ACK      */
#define RADCMD_DISCON_NAK 42 /* Disconnect-NAK      */
#define RADCMD_COA_REQ 43	 /* CoA-Request         */
#define RADCMD_COA_ACK 44	 /* CoA-ACK             */
#define RADCMD_COA_NAK 45	 /* CoA-NAK             */
#define RADCMD_RESERVED 255	 /* Reserved            */

	/*
		21	Resource-Free-Request	[RFC3575]
		22	Resource-Free-Response	[RFC3575]
		23	Resource-Query-Request	[RFC3575]
		24	Resource-Query-Response	[RFC3575]
		25	Alternate-Resource-Reclaim-Request	[RFC3575]

		26	NAS-Reboot-Request	[RFC3575]
		27	NAS-Reboot-Response	[RFC3575]
		28	Reserved
		29	Next-Passcode	[RFC3575]
		30	New-Pin	[RFC3575]
		31	Terminate-Session	[RFC3575]
		32	Password-Expired	[RFC3575]
		33	Event-Request	[RFC3575]
		34	Event-Response	[RFC3575]
		35-39	Unassigned
		40	Disconnect-Request	[RFC3575][RFC5176]
		41	Disconnect-ACK	[RFC3575][RFC5176]
		42	Disconnect-NAK	[RFC3575][RFC5176]
		43	CoA-Request	[RFC3575][RFC5176]
		44	CoA-ACK	[RFC3575][RFC5176]
		45	CoA-NAK	[RFC3575][RFC5176]
		46-49	Unassigned
		50	IP-Address-Allocate	[RFC3575]
		51	IP-Address-Release	[RFC3575]
		52	Protocol-Error	[RFC7930]
		53-249	Unassigned
		250-253	Experimental Use	[RFC3575]
		254	Reserved	[RFC3575]
		255	Reserved	[RFC3575]
	*/

	struct tok {
		uint cmd;
		const char *name;
	};

	/*

	Radius commands

	 char const *fr_packet_codes[FR_MAX_PACKET_CODE] = {
	 "",					//!< 0
	 "Access-Request",
	 "Access-Accept",
	 "Access-Reject",
	 "Accounting-Request",
	 "Accounting-Response",
	 "Accounting-Status",
	 "Password-Request",
	 "Password-Accept",
	 "Password-Reject",
	 "Accounting-Message",			//!< 10
	 "Access-Challenge",
	 "Status-Server",
	 "Status-Client",
	 "14",
	 "15",
	 "16",
	 "17",
	 "18",
	 "19",
	 "20",					//!< 20
	 "Resource-Free-Request",
	 "Resource-Free-Response",
	 "Resource-Query-Request",
	 "Resource-Query-Response",
	 "Alternate-Resource-Reclaim-Request",
	 "NAS-Reboot-Request",
	 "NAS-Reboot-Response",
	 "28",
	 "Next-Passcode",
	 "New-Pin",				//!< 30
	 "Terminate-Session",
	 "Password-Expired",
	 "Event-Request",
	 "Event-Response",
	 "35",
	 "36",
	 "37",
	 "38",
	 "39",
	 "Disconnect-Request",			//!< 40
	 "Disconnect-ACK",
	 "Disconnect-NAK",
	 "CoA-Request",
	 "CoA-ACK",
	 "CoA-NAK",
	 "46",
	 "47",
	 "48",
	 "49",
	 "IP-Address-Allocate",
	 "IP-Address-Release",			//!< 50
	};


	 */

	static const struct tok radius_command_values[] = {
		{RADCMD_ACCESS_REQ, "Access-Request"},
		{RADCMD_ACCESS_ACC, "Access-Accept"},
		{RADCMD_ACCESS_REJ, "Access-Reject"},
		{RADCMD_ACCOUN_REQ, "Accounting-Request"},
		{RADCMD_ACCOUN_RES, "Accounting-Response"},
		{RADCMD_ACCESS_CHA, "Access-Challenge"},
		{RADCMD_STATUS_SER, "Status-Server"},
		{RADCMD_STATUS_CLI, "Status-Client"},
		{RADCMD_DISCON_REQ, "Disconnect-Request"},
		{RADCMD_DISCON_ACK, "Disconnect-ACK"},
		{RADCMD_DISCON_NAK, "Disconnect-NAK"},
		{RADCMD_COA_REQ, "CoA-Request"},
		{RADCMD_COA_ACK, "CoA-ACK"},
		{RADCMD_COA_NAK, "CoA-NAK"},
		{RADCMD_RESERVED, "Reserved"},
		{RADCMD_ACCOUN_STATUS, "Accounting-Status"},
		{RADCMD_PASSWORD_REQUEST, "Password-Request"},
		{RADCMD_PASSWORD_ACK, "Password-Ack"},
		{RADCMD_PASSWORD_REJECT, "Password-Reject"},
		{RADCMD_ACCOUN_MESSAGE, "Accounting-Message"},
		{RADCMD_RES_FREE_REQ, "Resource-Free-Request"},
		{RADCMD_RES_FREE_RES, "Resource-Free-Response"},
		{RADCMD_RES_QUERY_REQ, "Resource-Query-Request"},
		{RADCMD_RES_QUERY_RES, "Resource-Query-Response"},
		{RADCMD_RES_ALT_RECLAIM_REQ, "Alternate-Resource-Reclaim-Request"},
		{0, nullptr}};

	constexpr std::uint8_t ACCT_STATUS_TYPE = 40;
	constexpr std::uint8_t ACCT_AUTHENTIC = 45;
	constexpr std::uint8_t CALLING_STATION_ID = 31;

	static const struct tok radius_attribute_names[] = {{1, "User-Name"},
														{2, "User-Password"},
														{3, "CHAP-Password"},
														{4, "NAS-IP Address"},
														{5, "NAS-Port"},
														{6, "Service-Type"},
														{7, "Framed-Protocol"},
														{8, "Framed-IP-Address"},
														{9, "Framed-IP-Netmask"},
														{10, "Framed-Routing"},
														{11, "Filter-Id"},
														{12, "Framed-MTU"},
														{13, "Framed-Compression"},
														{14, "Login-IP-Host"},
														{15, "Login-Service"},
														{16, "Login-TCP-Port"},
														{18, "Reply-Message"},
														{19, "Callback-Number"},
														{20, "Callback-ID"},
														{22, "Framed-Route"},
														{23, "Framed-IPX-Network"},
														{24, "State"},
														{25, "Class"},
														{26, "Vendor-Specific"},
														{27, "Session-Timeout"},
														{28, "Idle-Timeout"},
														{29, "Termination-Action"},
														{30, "Called-Station-Id"},
														{CALLING_STATION_ID, "Calling-Station-Id"},
														{32, "NAS-Identifier"},
														{33, "Proxy-State"},
														{34, "Login-LAT-Service"},
														{35, "Login-LAT-Node"},
														{36, "Login-LAT-Group"},
														{37, "Framed-AppleTalk-Link"},
														{38, "Framed-AppleTalk-Network"},
														{39, "Framed-AppleTalk-Zone"},
														{ACCT_STATUS_TYPE, "Acct-Status-Type"},
														{41, "Acct-Delay-Time"},
														{42, "Acct-Input-Octets"},
														{43, "Acct-Output-Octets"},
														{44, "Acct-Session-Id"},
														{ACCT_AUTHENTIC, "Acct-Authentic"},
														{46, "Acct-Session-Time"},
														{47, "Acct-Input-Packets"},
														{48, "Acct-Output-Packets"},
														{49, "Acct-Terminate-Cause"},
														{50, "Acct-Multi-Session-Id"},
														{51, "Acct-Link-Count"},
														{52, "Acct-Input-Gigawords"},
														{53, "Acct-Output-Gigawords"},
														{55, "Event-Timestamp"},
														{60, "CHAP-Challenge"},
														{61, "NAS-Port-Type"},
														{62, "Port-Limit"},
														{63, "Login-LAT-Port"},
														{64, "Tunnel-Type3"},
														{65, "Tunnel-Medium-Type1"},
														{66, "Tunnel-Client-Endpoint"},
														{67, "Tunnel-Server-Endpoint1"},
														{68, "Acct-Tunnel-Connection-ID"},
														{69, "Tunnel-Password1"},
														{70, "ARAP-Password"},
														{71, "ARAP-Features"},
														{72, "ARAP-Zone-Access"},
														{73, "ARAP-Security"},
														{74, "ARAP-Security-Data"},
														{75, "Password-Retry"},
														{76, "Prompt"},
														{77, "Connect-Info"},
														{78, "Configuration-Token"},
														{79, "EAP-Message"},
														{80, "Message-Authenticator"},
														{81, "Tunnel-Private-Group-ID"},
														{82, "Tunnel-Assignment-ID1"},
														{83, "Tunnel-Preference"},
														{84, "ARAP-Challenge-Response"},
														{85, "Acct-Interim-Interval"},
														{86, "Acct-Tunnel-Packets-Lost"},
														{87, "NAS-Port-ID"},
														{88, "Framed-Pool"},
														{90, "Tunnel-Client-Auth-ID"},
														{91, "Tunnel-Server-Auth-ID"},
														{0, nullptr}};

#pragma pack(push, 1)
	struct RadiusAttribute {
		unsigned char type{0};
		uint16_t pos{0};
		unsigned len{0};
	};
	struct RawRadiusPacket {
		unsigned char code{1};
		unsigned char identifier{0};
		uint16_t rawlen{0};
		unsigned char authenticator[16]{0};
		unsigned char attributes[4096]{0};
	};
#pragma pack(pop)

	constexpr unsigned char Access_Request = 1;
	constexpr unsigned char Access_Accept = 2;
	constexpr unsigned char Access_Reject = 3;
	constexpr unsigned char Access_Challenge = 11;

	constexpr unsigned char Accounting_Request = 4;
	constexpr unsigned char Accounting_Response = 5;
	constexpr unsigned char Accounting_Status = 6;
	constexpr unsigned char Accounting_Message = 10;

	constexpr unsigned char Disconnect_Request = 40;
	constexpr unsigned char Disconnect_ACK = 41;
	constexpr unsigned char Disconnect_NAK = 42;
	constexpr unsigned char CoA_Request = 43;
	constexpr unsigned char CoA_ACK = 44;
	constexpr unsigned char CoA_NAK = 45;

	constexpr unsigned char ATTR_MessageAuthenticator = 80;

	constexpr std::uint8_t ACCT_STATUS_TYPE_START = 1;
	constexpr std::uint8_t ACCT_STATUS_TYPE_STOP = 2;
	constexpr std::uint8_t ACCT_STATUS_TYPE_INTERIM_UPDATE = 3;
	constexpr std::uint8_t ACCT_STATUS_TYPE_ACCOUNTING_ON = 7;
	constexpr std::uint8_t ACCT_STATUS_TYPE_ACCOUNTING_OFF = 8;
	constexpr std::uint8_t ACCT_STATUS_TYPE_FAILED = 15;

	constexpr std::uint8_t ACCT_AUTHENTIC_RADIUS = 1;
	constexpr std::uint8_t ACCT_AUTHENTIC_LOCAL = 2;
	constexpr std::uint8_t ACCT_AUTHENTIC_REMOTE = 3;

	inline bool IsAuthentication(unsigned char t) {
		return (t == RADIUS::Access_Request || t == RADIUS::Access_Accept ||
				t == RADIUS::Access_Challenge || t == RADIUS::Access_Reject);
	}

	inline bool IsAccounting(unsigned char t) {
		return (t == RADIUS::Accounting_Request || t == RADIUS::Accounting_Response ||
				t == RADIUS::Accounting_Status || t == RADIUS::Accounting_Message);
	}

	inline bool IsAuthority(unsigned char t) {
		return (t == RADIUS::Disconnect_Request || t == RADIUS::Disconnect_ACK ||
				t == RADIUS::Disconnect_NAK || t == RADIUS::CoA_Request || t == RADIUS::CoA_ACK ||
				t == RADIUS::CoA_NAK);
	}

	inline const char *CommandName(uint cmd) {
		auto cmds = radius_command_values;
		while (cmds->cmd && (cmds->cmd != cmd))
			cmds++;
		if (cmds->cmd == cmd)
			return cmds->name;
		return "Unknown";
	}

	inline const char *AttributeName(uint cmd) {
		auto cmds = radius_attribute_names;
		while (cmds->cmd && (cmds->cmd != cmd))
			cmds++;
		if (cmds->cmd == cmd)
			return cmds->name;
		return "Unknown";
	}

	inline void MakeRadiusAuthenticator(unsigned char *authenticator) {
		for (int i = 0; i < 16; i++)
			authenticator[i] = std::rand() & 0xff;
	}

	//
	// From: https://github.com/Telecominfraproject/wlan-dictionary/blob/main/dictionary.tip
	//
	static const uint32_t TIP_vendor_id = 58888;
	static const unsigned char TIP_serial = 1;
	static const unsigned char TIP_AAAipaddr = 2;
	static const unsigned char TIP_AAAipv6addr = 3;

	using AttributeList = std::list<RadiusAttribute>;

	inline std::ostream &operator<<(std::ostream &os, AttributeList const &P) {
		for (const auto &attr : P) {
			os << "\tAttr: " << (uint16_t)attr.type << "  Size: " << (uint16_t)attr.len
			   << std::endl;
		}
		return os;
	}

	inline bool ParseRadius(uint32_t offset, const unsigned char *Buffer, uint16_t Size,
							AttributeList &Attrs) {
		Attrs.clear();
		uint16_t pos = 0;
		auto x = 25;
		while (pos < Size && x) {
			RadiusAttribute Attr{.type = Buffer[pos],
								 .pos = (uint16_t)(pos + 2 + offset),
								 .len = (unsigned int)(Buffer[pos + 1] - 2)};
			if (pos + Attr.len <= Size) {
				Attrs.emplace_back(Attr);
			} else {
				std::cout << "Bad parse1: " << (uint32_t)(pos + Attr.len) << "  S:" << Size
						  << std::endl;
				return false;
			}
			if (Buffer[pos + 1] == 0) {
				std::cout << "Bad parse2: " << (uint32_t)(pos + Attr.len) << "  S:" << Size
						  << std::endl;
				return false;
			}
			pos += Buffer[pos + 1];
			x--;
		}
		return true;
	}

	class RadiusPacket {
	  public:
		explicit RadiusPacket(const Poco::Buffer<char> &Buf) {
			if (Buf.size() >= sizeof(RawRadiusPacket)) {
				Valid_ = false;
				return;
			}
			memcpy((void *)&P_, Buf.begin(), Buf.size());
			Size_ = Buf.size();
			Valid_ = (Size_ == ntohs(P_.rawlen));
			if (Valid_)
				Valid_ = ParseRadius(0, (unsigned char *)&P_.attributes[0], Size_ - 20, Attrs_);
		}

		explicit RadiusPacket(const unsigned char *buffer, uint16_t size) {
			if (size >= sizeof(RawRadiusPacket)) {
				Valid_ = false;
				return;
			}
			memcpy((void *)&P_, buffer, size);
			Size_ = size;
			Valid_ = (Size_ == ntohs(P_.rawlen));
			if (Valid_)
				Valid_ = ParseRadius(0, (unsigned char *)&P_.attributes[0], Size_ - 20, Attrs_);
		}

		explicit RadiusPacket(const std::string &p) {
			if (p.size() >= sizeof(RawRadiusPacket)) {
				Valid_ = false;
				return;
			}
			memcpy((void *)&P_, (const unsigned char *)p.c_str(), p.size());
			Size_ = p.size();
			Valid_ = (Size_ == ntohs(P_.rawlen));
			if (Valid_)
				Valid_ = ParseRadius(0, (unsigned char *)&P_.attributes[0], Size_ - 20, Attrs_);
		}

		RadiusPacket(const RadiusPacket &P) {
			Valid_ = P.Valid_;
			Size_ = P.Size_;
			P_ = P.P_;
			Attrs_ = P.Attrs_;
		}

        void ReParse() {
            P_.rawlen = htons(Size_);
            Valid_ = ParseRadius(0, (unsigned char *)&P_.attributes[0], Size_ - 20, Attrs_);
        }

		inline RadiusPacket& operator=(const RadiusPacket& other) {
			Valid_ = other.Valid_;
			Size_ = other.Size_;
			P_ = other.P_;
			Attrs_ = other.Attrs_;
			return *this;
		}

		explicit RadiusPacket() = default;

		unsigned char *Buffer() { return (unsigned char *)&P_; }
		[[nodiscard]] uint16_t BufferLen() const { return sizeof(P_); }

		void Evaluate(uint16_t size) {
			Size_ = size;
			Valid_ = ParseRadius(0, (unsigned char *)&P_.attributes[0], Size_ - 20, Attrs_);
		}

		[[nodiscard]] uint16_t Len() const { return ntohs(P_.rawlen); }
		[[nodiscard]] uint16_t Size() const { return Size_; }

		friend std::ostream &operator<<(std::ostream &os, RadiusPacket const &P);

		inline bool IsAuthentication() {
			return (P_.code == RADIUS::Access_Request || P_.code == RADIUS::Access_Accept ||
					P_.code == RADIUS::Access_Challenge || P_.code == RADIUS::Access_Reject ||
					P_.code == RADCMD_RES_FREE_REQ || P_.code == RADCMD_RES_FREE_RES ||
					P_.code == RADCMD_RES_QUERY_REQ || P_.code == RADCMD_RES_QUERY_RES ||
					P_.code == RADCMD_RES_ALT_RECLAIM_REQ);
		}

		inline bool IsAccounting() {
			return (P_.code == RADIUS::Accounting_Request ||
					P_.code == RADIUS::Accounting_Response ||
					P_.code == RADIUS::Accounting_Status || P_.code == RADIUS::Accounting_Message);
		}

		inline bool IsAuthority() {
			return (P_.code == RADIUS::Disconnect_Request || P_.code == RADIUS::Disconnect_ACK ||
					P_.code == RADIUS::Disconnect_NAK || P_.code == RADIUS::CoA_Request ||
					P_.code == RADIUS::CoA_ACK || P_.code == RADIUS::CoA_NAK);
		}

		void Log(std::ostream &os) {
			uint16_t p = 0;

			while (p < Size_) {
				os << std::setfill('0') << std::setw(4) << p << ":  ";
				uint16_t v = 0;
				while (v < 16 && p + v < Size_) {
					os << std::setfill('0') << std::setw(2) << std::right << std::hex
					   << (uint16_t)((const unsigned char *)&P_)[p + v] << " ";
					v++;
				}
				os << std::endl;
				p += 16;
			}
			os << std::dec << std::endl << std::endl;
			Print(os);
		}

		inline const char *PacketType() { return CommandName(P_.code); }

		inline std::uint8_t PacketTypeInt() { return P_.code; }

		void ComputeMessageAuthenticator(const std::string &secret) {
			RawRadiusPacket P = P_;

			if (P_.code == 1) {
				unsigned char OldAuthenticator[16]{0};
				for (const auto &attr : Attrs_) {
					if (attr.type == 80) {
						memcpy(OldAuthenticator, &P_.attributes[attr.pos], 16);
						memset(&P.attributes[attr.pos], 0, 16);
					}
				}

				unsigned char NewAuthenticator[16]{0};
				Poco::HMACEngine<Poco::MD5Engine> H(secret);
				H.update((const unsigned char *)&P, Size_);
				auto digest = H.digest();
				int p = 0;
				for (const auto &i : digest)
					NewAuthenticator[p++] = i;

				if (memcmp(OldAuthenticator, NewAuthenticator, 16) == 0) {
					// std::cout << "Authenticator match..." << std::endl;
				} else {
					// std::cout << "Authenticator MIS-match..." << std::endl;
					for (const auto &attr : Attrs_) {
						if (attr.type == 80) {
							memcpy(&P_.attributes[attr.pos], NewAuthenticator, 16);
						}
					}
				}
			}
		}

		void RecomputeAuthenticator(const std::string &secret) {
			memset(P_.authenticator,0,sizeof(P_.authenticator));
			Poco::MD5Engine md5;
			md5.update((const unsigned char *)&P_, Size_);
			md5.update(secret.c_str(), secret.size());
			auto digest = md5.digest();
			int p = 0;
			for (const auto &i : digest)
				P_.authenticator[p++] = i;
		}

		bool VerifyMessageAuthenticator(const std::string &secret) {
			RawRadiusPacket P = P_;
			if (P_.code == 1) {
				unsigned char OldAuthenticator[16]{0};
				for (const auto &attr : Attrs_) {
					if (attr.type == 80) {
						memcpy(OldAuthenticator, &P_.attributes[attr.pos], 16);
						memset(&P.attributes[attr.pos], 0, 16);
					}
				}
				unsigned char NewAuthenticator[16]{0};
				Poco::HMACEngine<Poco::MD5Engine> H(secret);
				H.update((const unsigned char *)&P, Size_);
				auto digest = H.digest();
				int p = 0;
				for (const auto &i : digest)
					NewAuthenticator[p++] = i;
				return memcmp(OldAuthenticator, NewAuthenticator, 16) == 0;
			}
			return true;
		}

		static void BufLog(std::ostream &os, const char *pre, const unsigned char *b, uint s) {
			uint16_t p = 0;
			while (p < s) {
				os << pre << std::setfill('0') << std::setw(4) << p << ":  ";
				uint16_t v = 0;
				while (v < 16 && p + v < s) {
					os << std::setfill('0') << std::setw(2) << std::right << std::hex
					   << (uint16_t)b[p + v] << " ";
					v++;
				}
				os << std::endl;
				p += 16;
			}
			os << std::dec;
		}

		void PrintAccount_StatusType(std::ostream &os, const std::string &spaces, const unsigned char *buf, std::uint8_t len) {
			os << spaces ;
			if (buf[3]==ACCT_STATUS_TYPE_START)
				os << "Start" << std::endl;
			else if (buf[3]==ACCT_STATUS_TYPE_STOP)
				os << "Stop" << std::endl;
			else if (buf[3]==ACCT_STATUS_TYPE_INTERIM_UPDATE)
				os << "Interim-Update" << std::endl;
			else if (buf[3]==ACCT_STATUS_TYPE_ACCOUNTING_ON)
				os << "Accounting-On" << std::endl;
			else if (buf[3]==ACCT_STATUS_TYPE_ACCOUNTING_OFF)
				os << "Accounting-Off" << std::endl;
			else if (buf[3]==ACCT_STATUS_TYPE_FAILED)
				os << "Failed" << std::endl;
			else
				BufLog(os,"",buf,len);
		}

		void PrintAccount_AcctAuthentic(std::ostream &os, const std::string &spaces, const unsigned char *buf, std::uint8_t len) {
			os << spaces ;
			if (buf[3]==ACCT_AUTHENTIC_RADIUS)
				os << "RADIUS" << std::endl;
			else if (buf[3]==ACCT_AUTHENTIC_LOCAL)
				os << "Local" << std::endl;
			else if (buf[3]==ACCT_AUTHENTIC_REMOTE)
				os << "Remote" << std::endl;
			else
				BufLog(os,"",buf,len);
		}

		inline void Print(std::ostream &os) {
			os << "Packet type: (" << (uint)P_.code << ") " << CommandName(P_.code) << std::endl;
			os << "  Identifier: " << (uint)P_.identifier << std::endl;
			os << "  Length: " << Size_ << std::endl;
			os << "  Authenticator: ";
			BufLog(os, "", P_.authenticator, sizeof(P_.authenticator));
			os << "  Attributes: " << std::endl;
			for (const auto &attr : Attrs_) {
				os << "    " << std::setfill(' ') << "(" << std::setw(4) << (uint)attr.type << ") "
				   << AttributeName(attr.type) << "   Len:" << attr.len << std::endl;
				std::string attr_offset = "           ";
				switch(attr.type) {
				case ACCT_STATUS_TYPE: PrintAccount_StatusType(os, attr_offset, &P_.attributes[attr.pos], attr.len); break;
				case ACCT_AUTHENTIC: PrintAccount_AcctAuthentic(os, attr_offset, &P_.attributes[attr.pos], attr.len); break;
				default:
					BufLog(os, attr_offset.c_str(), &P_.attributes[attr.pos], attr.len);
				}
			}
			os << std::dec << std::endl << std::endl;
		}

		std::string ExtractSerialNumberTIP() {
			std::string R;

			for (const auto &attribute : Attrs_) {
				if (attribute.type == 26) {
					AttributeList VendorAttributes;
					uint32_t VendorId = htonl(*(const uint32_t *)&(P_.attributes[attribute.pos]));
					// std::cout << VendorId << std::endl;
					if (VendorId == TIP_vendor_id && attribute.len > (4 + 2)) {
						if (ParseRadius(attribute.pos + 4, &P_.attributes[attribute.pos + 4],
										attribute.len - 4 - 2, VendorAttributes)) {
							// std::cout << VendorAttributes << std::endl;
							for (const auto &vendorAttr : VendorAttributes) {
								if (vendorAttr.type == TIP_serial) {
									for (uint16_t i = 0; i < vendorAttr.len; i++) {
										if (P_.attributes[vendorAttr.pos + i] == '-')
											continue;
										R += (char)P_.attributes[vendorAttr.pos + i];
									}
									return R;
								}
							}
						}
					}
				}
			}
			return R;
		}

		std::string ExtractSerialNumberFromProxyState() {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == 33) {
					std::string Attr33;
					// format is serial:IP:port:interface
					Attr33.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len - 2);
					auto Parts = Poco::StringTokenizer(Attr33, "|");
					if (Parts.count() == 4) {
						return Parts[0];
					}
					Parts = Poco::StringTokenizer(Attr33, ":");
					if (Parts.count() == 4) {
						return Parts[0];
					}
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractProxyStateDestination() {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == 33 && attribute.len > 2) {
					std::string Attr33;
					// format is serial:IP:port:interface
					Attr33.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len - 2);
					auto Parts = Poco::StringTokenizer(Attr33, "|");
					if (Parts.count() == 4) {
						Poco::Net::SocketAddress D(Parts[1], Parts[2]);
						return D.toString();
					}
					Parts = Poco::StringTokenizer(Attr33, ":");
					if (Parts.count() == 4) {
						Poco::Net::SocketAddress D(Parts[1], Parts[2]);
						return D.toString();
					}
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractCallingStationID() {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == 31 && attribute.len > 2) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len - 2);
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractCalledStationID() {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == 30 && attribute.len > 2) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len - 2);
					return Result;
				}
			}
			return Result;
		}

		[[nodiscard]] std::string UserName() const {
			for (const auto &attr : Attrs_) {
				if (attr.type == 1) {
					std::string user_name{(const char *)&P_.attributes[attr.pos], attr.len};
					return user_name;
				}
			}
			return "";
		}

		void ReplaceAttribute(std::uint8_t attribute, std::uint8_t value) {
			for (const auto &attr : Attrs_) {
				if(attr.type==attribute) {
					P_.attributes[attr.pos] = value;
					return;
				}
			}
		}

		void ReplaceAttribute(std::uint8_t attribute, std::uint16_t t_value) {
			for (const auto &attr : Attrs_) {
				if(attr.type==attribute) {
					auto value = htons(t_value);
					P_.attributes[attr.pos+0] = value >> 8;
					P_.attributes[attr.pos+1] = value & 0x00ff;
					return;
				}
			}
		}

		void ReplaceAttribute(std::uint8_t attribute, std::uint32_t t_value) {
			for (const auto &attr : Attrs_) {
				if(attr.type==attribute) {
					auto value = htonl(t_value);
					P_.attributes[attr.pos+0] = (std::uint8_t ) ((value & 0xff000000) >> 24);
					P_.attributes[attr.pos+1] = (std::uint8_t ) ((value & 0x00ff0000) >> 16);
					P_.attributes[attr.pos+2] = (std::uint8_t ) ((value & 0x0000ff00) >> 8);
					P_.attributes[attr.pos+3] = (std::uint8_t ) ((value & 0x000000ff) >> 0);
					return;
				}
			}
		}

		void ReplaceAttribute(std::uint8_t attribute, const char *attribute_value, std::uint8_t attribute_len) {
			for (const auto &attr : Attrs_) {
				if(attr.type==attribute) {
					if(attr.len==attribute_len) {
						memcpy(&P_.attributes[attr.pos], attribute_value, attribute_len);
					} else if(attribute_len<attr.len){
						memcpy(&P_.attributes[attr.pos], attribute_value, attribute_len);
                        P_.attributes[attr.pos-1] = attribute_len + 2;
                        auto Shrink = attr.len - attribute_len;
						memmove(&P_.attributes[attr.pos+attribute_len], &P_.attributes[attr.pos+attr.len], Size_ - Shrink);
						Size_ -= Shrink;
						ReParse();
					} else {
						auto Augment = (attribute_len - attr.len);
						memmove(&P_.attributes[attr.pos+attribute_len], &P_.attributes[attr.pos+attr.len], Size_ + Augment);
						memcpy(&P_.attributes[attr.pos], attribute_value, attribute_len);
						P_.attributes[attr.pos-1] = attribute_len+2;
						Size_ += Augment;
						ReParse();
					}
					return;
				}
			}
		}

		void ReplaceAttribute(std::uint8_t attribute, const std::string &attribute_value) {
			ReplaceAttribute(attribute,attribute_value.c_str(),attribute_value.size());
		}

		void RemoveAttribute(std::uint8_t attribute) {
			for (const auto &attr : Attrs_) {
				if(attr.type==attribute) {
					auto Shrink = attr.len+2;
					memmove(&P_.attributes[attr.pos-2], &P_.attributes[attr.pos+attr.len], Size_ - Shrink);
					Size_ -= Shrink;
					ReParse();
					return;
				}
			}
		}

		void AppendAttribute(std::uint8_t attribute, std::uint8_t value) {
			P_.attributes[Size_+0] = attribute;
			P_.attributes[Size_+1] = 1+2;
			P_.attributes[Size_+2] = value;
			Size_+= 3;
			ReParse();
		}

		void AppendAttribute(std::uint8_t attribute, std::uint16_t t_value) {
			auto value = htons(t_value);
			P_.attributes[Size_+0] = attribute;
			P_.attributes[Size_+1] = 2+2;
			P_.attributes[Size_+2] = (value & 0xff00) >> 8;
			P_.attributes[Size_+3] = (value & 0x00ff) >> 0;
			Size_+= 4;
			ReParse();
		}

		void AppendAttribute(std::uint8_t attribute, std::uint32_t t_value) {
			auto value = htonl(t_value);
			P_.attributes[Size_+0] = attribute;
			P_.attributes[Size_+1] = 4+2;
			P_.attributes[Size_+2] = (value & 0xff000000) >> 24;
			P_.attributes[Size_+3] = (value & 0x00ff0000) >> 16;
			P_.attributes[Size_+4] = (value & 0x0000ff00) >> 8;
			P_.attributes[Size_+5] = (value & 0x000000ff) >> 0;
			Size_+= 6;
			ReParse();
		}

		void AppendAttribute(std::uint8_t attribute, const char *attribute_value, std::uint8_t attribute_len) {
			P_.attributes[Size_+0] = attribute;
			P_.attributes[Size_+1] = attribute_len+2;
			memcpy(&P_.attributes[Size_+2],attribute_value,attribute_len);
			Size_+= 2 + attribute_len;
			ReParse();
		}

		void AppendAttribute(std::uint8_t attribute, const std::string &attribute_value) {
			AppendAttribute(attribute, attribute_value.c_str(), attribute_value.size());
		}
		void AddAttribute(std::uint8_t location, std::uint8_t attribute, std::uint8_t value) {
			for (const auto &attr : Attrs_) {
				if(attr.type==location) {
					int Augment = 1;
					memmove(&P_.attributes[attr.pos+attr.len+1+1+Augment], &P_.attributes[attr.pos+attr.len], Size_-(attr.pos+attr.len));
					P_.attributes[attr.pos+attr.len+0] = attribute;
					P_.attributes[attr.pos+attr.len+1] = Augment+2;
					P_.attributes[attr.pos+attr.len+2] = value;
					Size_+=2+Augment;
					ReParse();
					return;
				}
			}
		}

		void AddAttribute(std::uint8_t location, std::uint8_t attribute, std::uint16_t t_value) {
			for (const auto &attr : Attrs_) {
				if(attr.type==location) {
					int Augment = 2;
					auto value = htons(t_value);
					memmove(&P_.attributes[attr.pos+attr.len+1+1+Augment], &P_.attributes[attr.pos+attr.len], Size_-(attr.pos+attr.len));
					P_.attributes[attr.pos+attr.len+0] = attribute;
					P_.attributes[attr.pos+attr.len+1] = Augment+2;
					P_.attributes[attr.pos+attr.len+2] = (value & 0xff00) >> 8;
					P_.attributes[attr.pos+attr.len+3] = (value & 0x00ff) >> 0;
					Size_+=2+Augment;
					ReParse();
					return;
				}
			}
		}

		void AddAttribute(std::uint8_t location, std::uint8_t attribute, std::uint32_t t_value) {
			for (const auto &attr: Attrs_) {
				if (attr.type == location) {
					int Augment = 4;
					auto value = htonl(t_value);
					memmove(&P_.attributes[attr.pos + attr.len + 1 + 1 + Augment], &P_.attributes[attr.pos + attr.len],
							Size_ - (attr.pos + attr.len));
					P_.attributes[attr.pos + attr.len + 0] = attribute;
					P_.attributes[attr.pos + attr.len + 1] = Augment+2;
					P_.attributes[attr.pos + attr.len + 2] = (value & 0xff000000) >> 24;
					P_.attributes[attr.pos + attr.len + 3] = (value & 0x00ff0000) >> 16;
					P_.attributes[attr.pos + attr.len + 4] = (value & 0x0000ff00) >> 8;
					P_.attributes[attr.pos + attr.len + 5] = (value & 0x000000ff) >> 0;
					Size_ += 2 + Augment;
					ReParse();
					return;
				}
			}
		}


		void AddAttribute(std::uint8_t location, std::uint8_t attribute, const char *attribute_value, std::uint8_t attribute_len) {
			for (const auto &attr: Attrs_) {
				if (attr.type == location) {
					int Augment = attribute_len;
					memmove(&P_.attributes[attr.pos + attr.len + 1 + 1 + Augment], &P_.attributes[attr.pos + attr.len],
							Size_ - (attr.pos + attr.len));
					P_.attributes[attr.pos + attr.len + 0] = attribute;
					P_.attributes[attr.pos + attr.len + 1] = Augment+2;
					memcpy(&P_.attributes[attr.pos + attr.len + 2], attribute_value, attribute_len);
					Size_ += 2 + Augment;
					ReParse();
					return;
				}
			}
		}

		void AddAttribute(std::uint8_t location, std::uint8_t attribute, const std::string &attribute_value) {
			AddAttribute(location, attribute, attribute_value.c_str(), attribute_value.size());
		}


		AttributeList Attrs_;
		RawRadiusPacket P_;
		uint16_t Size_{0};
		bool Valid_ = false;
	};

	class RadiusOutputPacket {
	  public:
		explicit RadiusOutputPacket(const std::string &Secret) : Secret_(Secret) {}

		inline void MakeStatusMessage() {
			P_.code = RADCMD_STATUS_SER;
			P_.identifier = std::rand() & 0x00ff;
			MakeRadiusAuthenticator(P_.authenticator);
			unsigned char MessageAuthenticator[16]{0};
			AddAttribute(ATTR_MessageAuthenticator, sizeof(MessageAuthenticator),
						 MessageAuthenticator);
            int PktLen = 1 + 1 + 2 + 16 + 1 + 1 + 16;
			P_.rawlen = htons(PktLen);

			Poco::HMACEngine<Poco::MD5Engine> H(Secret_);
			H.update((const unsigned char *)&P_, PktLen);
			auto digest = H.digest();
			int p = 0;
			for (const auto &i : digest)
				P_.attributes[1 + 1 + p++] = i;
		}

		inline void AddAttribute(unsigned char attr, uint8_t len, const unsigned char *data) {
			P_.attributes[AttributesLen_++] = attr;
			P_.attributes[AttributesLen_++] = len;
			memcpy(&P_.attributes[AttributesLen_], data, len);
			AttributesLen_ += len;
		}

		[[nodiscard]] inline const unsigned char *Data() const {
			return (const unsigned char *)&P_;
		}
		[[nodiscard]] inline std::uint16_t Len() const { return ntohs(P_.rawlen); }

	  private:
		RawRadiusPacket P_;
		uint16_t AttributesLen_ = 0;
		std::string Secret_;
	};

	inline std::ostream &operator<<(std::ostream &os, RadiusPacket const &P) {
		os << P.Attrs_;
		return os;
	}
} // namespace OpenWifi::RADIUS