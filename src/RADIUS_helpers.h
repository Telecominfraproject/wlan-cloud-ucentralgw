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

#include <framework/utils.h>

namespace OpenWifi::RADIUS {

	//	Packet types
	constexpr std::uint8_t Access_Request = 1;
	constexpr std::uint8_t Access_Accept = 2;
	constexpr std::uint8_t Access_Reject = 3;
	constexpr std::uint8_t Accounting_Request = 4;
	constexpr std::uint8_t Accounting_Response = 5;
	constexpr std::uint8_t Accounting_Status = 6;
	constexpr std::uint8_t Password_Request = 7;
	constexpr std::uint8_t Password_Ack = 8;
	constexpr std::uint8_t Password_Reject = 9;
	constexpr std::uint8_t Accounting_Message = 10;
	constexpr std::uint8_t Access_Challenge = 11;
	constexpr std::uint8_t Status_Server = 12; /* Status-Server       */
	constexpr std::uint8_t Status_Client = 13; /* Status-Client       */
	constexpr std::uint8_t Disconnect_Request = 40;
	constexpr std::uint8_t Disconnect_ACK = 41;
	constexpr std::uint8_t Disconnect_NAK = 42;
	constexpr std::uint8_t CoA_Request = 43;
	constexpr std::uint8_t CoA_ACK = 44;
	constexpr std::uint8_t CoA_NAK = 45;
	constexpr std::uint8_t Resource_Free_Request = 21;
	constexpr std::uint8_t Resource_Free_Response = 22;
	constexpr std::uint8_t Resource_Query_Request = 23;
	constexpr std::uint8_t Resource_Query_Response = 24;
	constexpr std::uint8_t Alternate_Resource_Reclaim_Request = 25 ;
	constexpr std::uint8_t Reserved_Cmd = 255;	 /* Reserved            */

	//	Some attribute values
	namespace Attributes {
		constexpr std::uint8_t AUTH_USERNAME = 1;
		constexpr std::uint8_t USER_PASSWORD = 2;
		constexpr std::uint8_t CHAP_PASSWORD = 3;
		constexpr std::uint8_t NAS_IP = 4;
		constexpr std::uint8_t CALLED_STATION_ID = 30;
		constexpr std::uint8_t CALLING_STATION_ID = 31;
		constexpr std::uint8_t NAS_IDENTIFIER = 32;
		constexpr std::uint8_t PROXY_STATE = 33;
		constexpr std::uint8_t ACCT_STATUS_TYPE = 40;
		constexpr std::uint8_t ACCT_INPUT_OCTETS = 42;
		constexpr std::uint8_t ACCT_OUTPUT_OCTETS = 43;
		constexpr std::uint8_t ACCT_SESSION_ID = 44;
		constexpr std::uint8_t ACCT_AUTHENTIC = 45;
		constexpr std::uint8_t ACCT_SESSION_TIME = 46;
		constexpr std::uint8_t ACCT_INPUT_PACKETS = 47;
		constexpr std::uint8_t ACCT_OUTPUT_PACKETS = 48;
		constexpr std::uint8_t ACCT_TERMINATE_CAUSE = 49;
		constexpr std::uint8_t ACCT_MULTI_SESSION_ID = 50;
		constexpr std::uint8_t ACCT_INPUT_GIGAWORDS = 52;
		constexpr std::uint8_t ACCT_OUTPUT_GIGAWORDS = 53;
		constexpr std::uint8_t EVENT_TIMESTAMP = 55;
		constexpr std::uint8_t TUNNEL_PRIVATE_GROUP_ID = 81;
		constexpr std::uint8_t MESSAGE_AUTHENTICATOR = 80;
		constexpr std::uint8_t CHARGEABLE_USER_IDENTITY = 89;
	};

	namespace AccountingPacketTypes {
		constexpr std::uint8_t ACCT_STATUS_TYPE_START = 1;
		constexpr std::uint8_t ACCT_STATUS_TYPE_STOP = 2;
		constexpr std::uint8_t ACCT_STATUS_TYPE_INTERIM_UPDATE = 3;
		constexpr std::uint8_t ACCT_STATUS_TYPE_ACCOUNTING_ON = 7;
		constexpr std::uint8_t ACCT_STATUS_TYPE_ACCOUNTING_OFF = 8;
		constexpr std::uint8_t ACCT_STATUS_TYPE_FAILED = 15;
	}

	namespace AccountingTerminationReasons {
		constexpr std::uint8_t ACCT_TERMINATE_USER_REQUEST = 1;
		constexpr std::uint8_t ACCT_TERMINATE_LOST_CARRIER = 2;
		constexpr std::uint8_t ACCT_TERMINATE_LOST_SERVICE = 3;
		constexpr std::uint8_t ACCT_TERMINATE_IDLE_TIMEOUT = 4;
		constexpr std::uint8_t ACCT_TERMINATE_SESSION_TIMEOUT = 5;
		constexpr std::uint8_t ACCT_TERMINATE_ADMIN_RESET = 6;
		constexpr std::uint8_t ACCT_TERMINATE_ADMIN_REBOOT = 7;
		constexpr std::uint8_t ACCT_TERMINATE_PORT_ERROR = 8;
		constexpr std::uint8_t ACCT_TERMINATE_NAS_ERROR = 9;
		constexpr std::uint8_t ACCT_TERMINATE_NAS_REQUEST = 10;
		constexpr std::uint8_t ACCT_TERMINATE_PORT_REBOOT = 11;
		constexpr std::uint8_t ACCT_TERMINATE_PORT_UNNEEDED = 12;
		constexpr std::uint8_t ACCT_TERMINATE_PORT_PREEMPTED = 13;
		constexpr std::uint8_t ACCT_TERMINATE_PORT_SUSPEND = 14;
		constexpr std::uint8_t ACCT_TERMINATE_SERVICE_UNAVAILABLE = 15;
		constexpr std::uint8_t ACCT_TERMINATE_CALLBACK = 16;
		constexpr std::uint8_t ACCT_TERMINATE_USER_ERROR = 17;
		constexpr std::uint8_t ACCT_TERMINATE_HOST_REQUEST = 18;
	};

	namespace AuthenticationTypes {
		constexpr std::uint8_t ACCT_AUTHENTIC_RADIUS = 1;
		constexpr std::uint8_t ACCT_AUTHENTIC_LOCAL = 2;
		constexpr std::uint8_t ACCT_AUTHENTIC_REMOTE = 3;
	};

	constexpr std::uint32_t 	TIP_vendor_id = 58888;
	constexpr std::uint8_t 		TIP_serial = 1;
	constexpr std::uint8_t 		TIP_AAAipaddr = 2;
	constexpr std::uint8_t 		TIP_AAAipv6addr = 3;

	struct tok {
		uint cmd;
		const char *name;
	};

	static const struct tok radius_command_values[] = {
		{Access_Request, "Access-Request"},
		{Access_Accept, "Access-Accept"},
		{Access_Reject, "Access-Reject"},
		{Accounting_Request, "Accounting-Request"},
		{Accounting_Response, "Accounting-Response"},
		{Access_Challenge, "Access-Challenge"},
		{Status_Server, "Status-Server"},
		{Status_Client, "Status-Client"},
		{Disconnect_Request, "Disconnect-Request"},
		{Disconnect_ACK, "Disconnect-ACK"},
		{Disconnect_NAK, "Disconnect-NAK"},
		{CoA_Request, "CoA-Request"},
		{CoA_ACK, "CoA-ACK"},
		{CoA_NAK, "CoA-NAK"},
		{Reserved_Cmd, "Reserved"},
		{Accounting_Status, "Accounting-Status"},
		{Password_Request, "Password-Request"},
		{Password_Ack, "Password-Ack"},
		{Password_Reject, "Password-Reject"},
		{Accounting_Message, "Accounting-Message"},
		{Resource_Free_Request, "Resource-Free-Request"},
		{Resource_Free_Response, "Resource-Free-Response"},
		{Resource_Query_Request, "Resource-Query-Request"},
		{Resource_Query_Response, "Resource-Query-Response"},
		{Alternate_Resource_Reclaim_Request, "Alternate-Resource-Reclaim-Request"},
		{0, nullptr}
	};

	static const struct tok radius_attribute_names[] = {
		{ Attributes::AUTH_USERNAME, "User-Name"},
		{ Attributes::USER_PASSWORD, "User-Password"},
		{ Attributes::CHAP_PASSWORD, "CHAP-Password"},
		{ Attributes::NAS_IP, "NAS-IP Address"},
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
		{ Attributes::CALLED_STATION_ID, "Called-Station-Id"},
		{ Attributes::CALLING_STATION_ID, "Calling-Station-Id"},
		{ Attributes::NAS_IDENTIFIER, "NAS-Identifier"},
		{ Attributes::PROXY_STATE, "Proxy-State"},
		{34, "Login-LAT-Service"},
		{35, "Login-LAT-Node"},
		{36, "Login-LAT-Group"},
		{37, "Framed-AppleTalk-Link"},
		{38, "Framed-AppleTalk-Network"},
		{39, "Framed-AppleTalk-Zone"},
		{ Attributes::ACCT_STATUS_TYPE, "Acct-Status-Type"},
		{41, "Acct-Delay-Time"},
		{ Attributes::ACCT_INPUT_OCTETS, "Acct-Input-Octets"},
		{ Attributes::ACCT_OUTPUT_OCTETS, "Acct-Output-Octets"},
		{ Attributes::ACCT_SESSION_ID, "Acct-Session-Id"},
		{ Attributes::ACCT_AUTHENTIC, "Acct-Authentic"},
		{ Attributes::ACCT_SESSION_TIME, "Acct-Session-Time"},
		{ Attributes::ACCT_INPUT_PACKETS, "Acct-Input-Packets"},
		{ Attributes::ACCT_OUTPUT_PACKETS, "Acct-Output-Packets"},
		{ Attributes::ACCT_TERMINATE_CAUSE, "Acct-Terminate-Cause"},
		{ Attributes::ACCT_MULTI_SESSION_ID, "Acct-Multi-Session-Id"},
		{51, "Acct-Link-Count"},
		{ Attributes::ACCT_INPUT_GIGAWORDS, "Acct-Input-Gigawords"},
		{ Attributes::ACCT_OUTPUT_GIGAWORDS, "Acct-Output-Gigawords"},
		{ Attributes::EVENT_TIMESTAMP, "Event-Timestamp"},
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
		{ Attributes::MESSAGE_AUTHENTICATOR, "Message-Authenticator"},
		{81, "Tunnel-Private-Group-ID"},
		{82, "Tunnel-Assignment-ID1"},
		{83, "Tunnel-Preference"},
		{84, "ARAP-Challenge-Response"},
		{85, "Acct-Interim-Interval"},
		{86, "Acct-Tunnel-Packets-Lost"},
		{87, "NAS-Port-ID"},
		{88, "Framed-Pool"},
		{ Attributes::CHARGEABLE_USER_IDENTITY, "Chargeable-User-Identity"},
		{90, "Tunnel-Client-Auth-ID"},
		{91, "Tunnel-Server-Auth-ID"},
		{0, nullptr}
	};

	constexpr std::uint32_t AttributeOffset = 20;

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

		explicit RadiusPacket() :
 			P_{0}, Size_(AttributeOffset)
		{
		};

		unsigned char *Buffer() { return (unsigned char *)&P_; }
		[[nodiscard]] uint16_t BufferLen() const { return sizeof(P_); }

		void Evaluate(uint16_t size) {
			Size_ = size;
			Valid_ = ParseRadius(0, (unsigned char *)&P_.attributes[0], Size_ - 20, Attrs_);
		}

		[[nodiscard]] uint16_t Len() const { return ntohs(P_.rawlen); }
		[[nodiscard]] uint16_t Size() const { return Size_; }

		friend std::ostream &operator<<(std::ostream &os, RadiusPacket const &P);

		[[nodiscard]] inline std::string PacketTypeToString() const {

			for(auto const &Name:radius_command_values) {
				if(Name.cmd == P_.code)
					return Name.name;
			}
			return "Unknown";
		}

		inline bool IsAuthentication() {
			return (P_.code == RADIUS::Access_Request || P_.code == RADIUS::Access_Accept ||
					P_.code == RADIUS::Access_Challenge || P_.code == RADIUS::Access_Reject ||
					P_.code == RADIUS::Resource_Free_Request || P_.code == RADIUS::Resource_Free_Response ||
					P_.code == RADIUS::Resource_Query_Request || P_.code == RADIUS::Resource_Query_Response ||
					P_.code == RADIUS::Alternate_Resource_Reclaim_Request);
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

		inline bool IsStatusMessageReply(std::string &ReplySource) {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == RADIUS::Attributes::PROXY_STATE) {
					std::string Attr33;
					// format is statis:server name
					Attr33.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len);
					auto Parts = Poco::StringTokenizer(Attr33, ":");
					if(Parts.count() == 2 && Parts[0] == "status") {
						ReplySource = Parts[1];
						return true;
					}
					return false;
				}
			}
			DBGLINE
			return false;
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
		inline void PacketType(std::uint8_t T) { P_.code = T; }

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

		inline std::uint8_t Identifier(std::uint8_t id) { P_.identifier = id; return id; }
		inline std::uint8_t Identifier() const { return P_.identifier; }

		void RecomputeAuthenticator(const std::string &secret) {
			memset(P_.authenticator,0,sizeof(P_.authenticator));
			Poco::MD5Engine md5;
			P_.rawlen = htons(Size_);
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
			if (buf[3]==AccountingPacketTypes::ACCT_STATUS_TYPE_START)
				os << "Start" << std::endl;
			else if (buf[3]==AccountingPacketTypes::ACCT_STATUS_TYPE_STOP)
				os << "Stop" << std::endl;
			else if (buf[3]==AccountingPacketTypes::ACCT_STATUS_TYPE_INTERIM_UPDATE)
				os << "Interim-Update" << std::endl;
			else if (buf[3]==AccountingPacketTypes::ACCT_STATUS_TYPE_ACCOUNTING_ON)
				os << "Accounting-On" << std::endl;
			else if (buf[3]==AccountingPacketTypes::ACCT_STATUS_TYPE_ACCOUNTING_OFF)
				os << "Accounting-Off" << std::endl;
			else if (buf[3]==AccountingPacketTypes::ACCT_STATUS_TYPE_FAILED)
				os << "Failed" << std::endl;
			else
				BufLog(os,"",buf,len);
		}

		void PrintAccount_AcctAuthentic(std::ostream &os, const std::string &spaces, const unsigned char *buf, std::uint8_t len) {
			os << spaces ;
			if (buf[3]==AuthenticationTypes::ACCT_AUTHENTIC_RADIUS)
				os << "RADIUS" << std::endl;
			else if (buf[3]==AuthenticationTypes::ACCT_AUTHENTIC_LOCAL)
				os << "Local" << std::endl;
			else if (buf[3]==AuthenticationTypes::ACCT_AUTHENTIC_REMOTE)
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
				case Attributes::ACCT_STATUS_TYPE: PrintAccount_StatusType(os, attr_offset, &P_.attributes[attr.pos], attr.len); break;
				case Attributes::ACCT_AUTHENTIC: PrintAccount_AcctAuthentic(os, attr_offset, &P_.attributes[attr.pos], attr.len); break;
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

		std::string ExtractSerialNumberFromProxyState() const {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == RADIUS::Attributes::PROXY_STATE) {
					std::string Attr33;
					// format is serial:IP:port:interface
					Attr33.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len);
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

		std::string ExtractProxyStateDestination() const {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == RADIUS::Attributes::PROXY_STATE && attribute.len > 2) {
					std::string Attr33;
					// format is

					Attr33.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len);
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

		std::uint32_t ExtractProxyStateDestinationIPint() const {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == RADIUS::Attributes::PROXY_STATE && attribute.len > 2) {
					std::string Attr33;
					// format is

					Attr33.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len);
					auto Parts = Poco::StringTokenizer(Attr33, "|");
					if (Parts.count() == 4) {
						return Utils::IPtoInt(Parts[1]);
					}
					Parts = Poco::StringTokenizer(Attr33, ":");
					if (Parts.count() == 4) {
						return Utils::IPtoInt(Parts[1]);
					}
					return 0;
				}
			}
			return 0;
		}

		std::string ExtractCallingStationID() const {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == RADIUS::Attributes::CALLING_STATION_ID && attribute.len > 0) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len);
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractAccountingSessionID() const {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == RADIUS::Attributes::ACCT_SESSION_ID && attribute.len > 0) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len );
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractAccountingMultiSessionID() const {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == RADIUS::Attributes::ACCT_MULTI_SESSION_ID && attribute.len > 0) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len );
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractCalledStationID() const {
			std::string Result;
			for (const auto &attribute : Attrs_) {
				if (attribute.type == RADIUS::Attributes::CALLED_STATION_ID && attribute.len > 0) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],
								  attribute.len);
					return Result;
				}
			}
			return Result;
		}

		[[nodiscard]] std::string UserName() const {
			for (const auto &attr : Attrs_) {
				if (attr.type == RADIUS::Attributes::AUTH_USERNAME) {
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

		void ReplaceAttribute(std::uint8_t attribute, std::uint16_t value) {
			for (const auto &attr : Attrs_) {
				if(attr.type==attribute) {
					P_.attributes[attr.pos+0] = value >> 8;
					P_.attributes[attr.pos+1] = value & 0x00ff;
					return;
				}
			}
		}

		void ReplaceAttribute(std::uint8_t attribute, std::uint32_t value) {
			for (const auto &attr : Attrs_) {
				if(attr.type==attribute) {
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
			if(Size_<AttributeOffset) Size_ = AttributeOffset;
			auto Pos = Size_ - AttributeOffset;
			P_.attributes[Pos+0] = attribute;
			P_.attributes[Pos+1] = 1+2;
			P_.attributes[Pos+2] = value;
			Size_+= 3;
			ReParse();
		}

		void AppendAttribute(std::uint8_t attribute, std::uint16_t value) {
			if(Size_<AttributeOffset) Size_ = AttributeOffset;
			auto Pos = Size_ - AttributeOffset;
			P_.attributes[Pos+0] = attribute;
			P_.attributes[Pos+1] = 2+2;
			P_.attributes[Pos+2] = (value & 0xff00) >> 8;
			P_.attributes[Pos+3] = (value & 0x00ff) >> 0;
			Size_+= 4;
			ReParse();
		}

		void AppendAttribute(std::uint8_t attribute, std::uint32_t value) {
			if(Size_<AttributeOffset) Size_ = AttributeOffset;
			auto Pos = Size_ - AttributeOffset;
			P_.attributes[Pos+0] = attribute;
			P_.attributes[Pos+1] = 4+2;
			P_.attributes[Pos+2] = (value & 0xff000000) >> 24;
			P_.attributes[Pos+3] = (value & 0x00ff0000) >> 16;
			P_.attributes[Pos+4] = (value & 0x0000ff00) >> 8;
			P_.attributes[Pos+5] = (value & 0x000000ff) >> 0;
			Size_+= 6;
			ReParse();
		}

		void AppendAttribute(std::uint8_t attribute, const char *attribute_value, std::uint8_t attribute_len) {
			if(Size_<AttributeOffset) Size_ = AttributeOffset;
			auto Pos = Size_ - AttributeOffset;
			P_.attributes[Pos+0] = attribute;
			P_.attributes[Pos+1] = attribute_len+2;
			memcpy(&P_.attributes[Pos+2],attribute_value,attribute_len);
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

		void AddAttribute(std::uint8_t location, std::uint8_t attribute, std::uint16_t value) {
			for (const auto &attr : Attrs_) {
				if(attr.type==location) {
					int Augment = 2;
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

		void AddAttribute(std::uint8_t location, std::uint8_t attribute, std::uint32_t value) {
			for (const auto &attr: Attrs_) {
				if (attr.type == location) {
					int Augment = 4;
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

		bool HasAttribute(std::uint8_t attribute) const {
			return std::any_of(Attrs_.begin(),Attrs_.end(),[attribute](const RadiusAttribute &Attr) { return Attr.type ==attribute; });
		}

		void ReplaceOrAdd(std::uint8_t attribute, std::uint8_t attribute_value) {
			if(HasAttribute(attribute))
				ReplaceAttribute(attribute, attribute_value);
			else
				AppendAttribute(attribute, attribute_value);
		}

		void ReplaceOrAdd(std::uint8_t attribute, std::uint16_t attribute_value) {
			if(HasAttribute(attribute))
				ReplaceAttribute(attribute, attribute_value);
			else
				AppendAttribute(attribute, attribute_value);
		}

		void ReplaceOrAdd(std::uint8_t attribute, std::uint32_t attribute_value) {
			if(HasAttribute(attribute))
				ReplaceAttribute(attribute, attribute_value);
			else
				AppendAttribute(attribute, attribute_value);
		}

		void ReplaceOrAdd(std::uint8_t attribute, const char *attribute_value, std::uint8_t attribute_len) {
			if(HasAttribute(attribute))
				ReplaceAttribute(attribute, attribute_value, attribute_len);
			else
				AppendAttribute(attribute, attribute_value, attribute_len);
		}

		void ReplaceOrAdd(std::uint8_t attribute, const std::string & attribute_value) {
			if(HasAttribute(attribute))
				ReplaceAttribute(attribute, attribute_value.c_str(), attribute_value.size());
			else
				AppendAttribute(attribute, attribute_value.c_str(), attribute_value.size());

		}

		AttributeList Attrs_;
		RawRadiusPacket P_;
		uint16_t Size_{0};
		bool Valid_ = false;
	};

	class RadiusOutputPacket {
	  public:
		explicit RadiusOutputPacket(const std::string &Secret) : Secret_(Secret) {}

		inline void MakeStatusMessage(const std::string &Source) {
			P_.code = RADIUS::Status_Server;
			P_.identifier = std::rand() & 0x00ff;
			MakeRadiusAuthenticator(P_.authenticator);
			unsigned char MessageAuthenticator[16]{0};
			std::string FullSource = "status:" + Source;
			AddAttribute(RADIUS::Attributes::PROXY_STATE, FullSource.size(), (const unsigned char *)FullSource.c_str());
			AddAttribute(RADIUS::Attributes::MESSAGE_AUTHENTICATOR, sizeof(MessageAuthenticator),
						 MessageAuthenticator);
            // int PktLen = 1 + 1 + 2 + 16 + 1 + 1 + 16 ;
			int PktLen = 1 + 1 + 2 + 16 + AttributesLen_;
			P_.rawlen = htons(PktLen);

			Poco::HMACEngine<Poco::MD5Engine> H(Secret_);
			H.update((const unsigned char *)&P_, PktLen);
			auto digest = H.digest();
			int p = 0, offset = (int)FullSource.size() + 2 ;
			for (const auto &i : digest)
				P_.attributes[offset + 1 + 1 + p++] = i;
		}

		inline void AddAttribute(unsigned char attr, uint8_t len, const unsigned char *data) {
			P_.attributes[AttributesLen_++] = attr;
			P_.attributes[AttributesLen_++] = len+2;
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