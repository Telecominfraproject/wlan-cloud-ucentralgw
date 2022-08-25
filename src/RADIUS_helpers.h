//
// Created by stephane bourque on 2022-06-20.
//

#pragma once

#include <list>
#include <string>
#include <iostream>
#include <iomanip>

#include "Poco/HMACEngine.h"
#include "Poco/MD5Engine.h"

namespace OpenWifi::RADIUS {

#define RADCMD_ACCESS_REQ   1 /* Access-Request      */
#define RADCMD_ACCESS_ACC   2 /* Access-Accept       */
#define RADCMD_ACCESS_REJ   3 /* Access-Reject       */
#define RADCMD_ACCOUN_REQ   4 /* Accounting-Request  */
#define RADCMD_ACCOUN_RES   5 /* Accounting-Response */
#define RADCMD_ACCESS_CHA  11 /* Access-Challenge    */
#define RADCMD_STATUS_SER  12 /* Status-Server       */
#define RADCMD_STATUS_CLI  13 /* Status-Client       */
#define RADCMD_DISCON_REQ  40 /* Disconnect-Request  */
#define RADCMD_DISCON_ACK  41 /* Disconnect-ACK      */
#define RADCMD_DISCON_NAK  42 /* Disconnect-NAK      */
#define RADCMD_COA_REQ     43 /* CoA-Request         */
#define RADCMD_COA_ACK     44 /* CoA-ACK             */
#define RADCMD_COA_NAK     45 /* CoA-NAK             */
#define RADCMD_RESERVED   255 /* Reserved            */

struct tok {
	uint 	cmd;
	const char * name;
};

static const struct tok radius_command_values[] = {
	{ RADCMD_ACCESS_REQ, "Access-Request" },
	{ RADCMD_ACCESS_ACC, "Access-Accept" },
	{ RADCMD_ACCESS_REJ, "Access-Reject" },
	{ RADCMD_ACCOUN_REQ, "Accounting-Request" },
	{ RADCMD_ACCOUN_RES, "Accounting-Response" },
	{ RADCMD_ACCESS_CHA, "Access-Challenge" },
	{ RADCMD_STATUS_SER, "Status-Server" },
	{ RADCMD_STATUS_CLI, "Status-Client" },
	{ RADCMD_DISCON_REQ, "Disconnect-Request" },
	{ RADCMD_DISCON_ACK, "Disconnect-ACK" },
	{ RADCMD_DISCON_NAK, "Disconnect-NAK" },
	{ RADCMD_COA_REQ,    "CoA-Request" },
	{ RADCMD_COA_ACK,    "CoA-ACK" },
	{ RADCMD_COA_NAK,    "CoA-NAK" },
	{ RADCMD_RESERVED,   "Reserved" },
	{ 0, nullptr}
};

static const struct tok radius_attribute_names[] = {
	{1,"User-Name"},
	{2,"User-Password"},
	{3,"CHAP-Password"},
	{4,"NAS-IP Address"},
	{5,"NAS-Port"},
	{6,"Service-Type"},
	{7,"Framed-Protocol"},
	{8,"Framed-IP-Address"},
	{9,"Framed-IP-Netmask"},
	{10,"Framed-Routing"},
	{11,"Filter-Id"},
	{12,"Framed-MTU"},
	{13,"Framed-Compression"},
	{14,"Login-IP-Host"},
	{15,"Login-Service"},
	{16,"Login-TCP-Port"},
	{18,"Reply-Message"},
	{19,"Callback-Number"},
	{20,"Callback-ID"},
	{22,"Framed-Route"},
	{23,"Framed-IPX-Network"},
	{24,"State"},
	{25,"Class"},
	{26,"Vendor-Specific"},
	{27,"Session-Timeout"},
	{28,"Idle-Timeout"},
	{29,"Termination-Action"},
	{30,"Called-Station-Id"},
	{31,"Calling-Station-Id"},
	{32,"NAS-Identifier"},
	{33,"Proxy-State"},
	{34,"Login-LAT-Service"},
	{35,"Login-LAT-Node"},
	{36,"Login-LAT-Group"},
	{37,"Framed-AppleTalk-Link"},
	{38,"Framed-AppleTalk-Network"},
	{39,"Framed-AppleTalk-Zone"},
	{40,"Acct-Status-Type"},
	{41,"Acct-Delay-Time"},
	{42,"Acct-Input-Octets"},
	{43,"Acct-Output-Octets"},
	{44,"Acct-Session-Id"},
	{45,"Acct-Authentic"},
	{46,"Acct-Session-Time"},
	{47,"Acct-Input-Packets"},
	{48,"Acct-Output-Packets"},
	{49,"Acct-Terminate-Cause"},
	{50,"Acct-Multi-Session-Id"},
	{51,"Acct-Link-Count"},
	{52,"Acct-Input-Gigawords"},
	{53,"Acct-Output-Gigawords"},
	{55,"Event-Timestamp"},
	{60,"CHAP-Challenge"},
	{61,"NAS-Port-Type"},
	{62,"Port-Limit"},
	{63,"Login-LAT-Port"},
	{64,"Tunnel-Type3"},
	{65,"Tunnel-Medium-Type1"},
	{66,"Tunnel-Client-Endpoint"},
	{67,"Tunnel-Server-Endpoint1"},
	{68,"Acct-Tunnel-Connection-ID"},
	{69,"Tunnel-Password1"},
	{70,"ARAP-Password"},
	{71,"ARAP-Features"},
	{72,"ARAP-Zone-Access"},
	{73,"ARAP-Security"},
	{74,"ARAP-Security-Data"},
	{75,"Password-Retry"},
	{76,"Prompt"},
	{77,"Connect-Info"},
	{78,"Configuration-Token"},
	{79,"EAP-Message"},
	{80,"Message-Authenticator"},
	{81,"Tunnel-Private-Group-ID"},
	{82,"Tunnel-Assignment-ID1"},
	{83,"Tunnel-Preference"},
	{84,"ARAP-Challenge-Response"},
	{85,"Acct-Interim-Interval"},
	{86,"Acct-Tunnel-Packets-Lost"},
	{87,"NAS-Port-ID"},
	{88,"Framed-Pool"},
	{90,"Tunnel-Client-Auth-ID"},
	{91,"Tunnel-Server-Auth-ID"},
	{0, nullptr}
};

#pragma pack(push,1)
	struct RadiusAttribute {
		unsigned char   type{0};
		uint16_t        pos{0};
		unsigned        len{0};
	};
	struct RawRadiusPacket {
		unsigned char   code{1};
		unsigned char   identifier{0};
		uint16_t        len{0};
		unsigned char   authenticator[16]{0};
		unsigned char   attributes[4096]{0};
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
	constexpr unsigned char Disconnect_ACK	 = 41;
	constexpr unsigned char Disconnect_NAK = 42;
	constexpr unsigned char CoA_Request = 43;
	constexpr unsigned char CoA_ACK = 44;
	constexpr unsigned char CoA_NAK = 45;

	inline bool IsAuthentication(unsigned char t) {
		return (t == RADIUS::Access_Request ||
				t == RADIUS::Access_Accept ||
				t == RADIUS::Access_Challenge ||
				t == RADIUS::Access_Reject);
	}

	inline bool IsAccounting(unsigned char t) {
		return (t == RADIUS::Accounting_Request ||
				t == RADIUS::Accounting_Response ||
				t == RADIUS::Accounting_Status ||
				t == RADIUS::Accounting_Message);
	}

	inline bool IsAuthority(unsigned char t) {
		return (t == RADIUS::Disconnect_Request ||
				t == RADIUS::Disconnect_ACK ||
				t == RADIUS::Disconnect_NAK ||
				t == RADIUS::CoA_Request ||
				t == RADIUS::CoA_ACK ||
				t == RADIUS::CoA_NAK);
	}

	inline const char * CommandName(uint cmd) {
		auto cmds = radius_command_values;
		while(cmds->cmd && (cmds->cmd!=cmd))
			cmds++;
		if(cmds->cmd==cmd) return cmds->name;
		return "Unknown";
	}

	inline const char * AttributeName(uint cmd) {
		auto cmds = radius_attribute_names;
		while(cmds->cmd && (cmds->cmd!=cmd))
			cmds++;
		if(cmds->cmd==cmd) return cmds->name;
		return "Unknown";
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
		for(const auto &attr:P) {
			os << "\tAttr: " << (uint16_t) attr.type << "  Size: " << (uint16_t) attr.len << std::endl;
		}
		return os;
	}

	inline bool ParseRadius(uint32_t offset, const unsigned char *Buffer, uint16_t Size, AttributeList &Attrs) {
		Attrs.clear();
		uint16_t pos=0;
		auto x=25;
		while(pos<Size && x) {
			RadiusAttribute Attr{ .type=Buffer[pos], .pos=(uint16_t)(pos+2+offset), .len=(unsigned int)(Buffer[pos+1]-2)};
			// std::cout << "POS: " << pos << "  P:" << (uint32_t) Attr.pos << "  T:" << (uint32_t) Attr.type << "  L:" << (uint32_t) Attr.len << "  S:" << (uint32_t) Size << std::endl;
			if(pos+Attr.len<=Size) {
				Attrs.emplace_back(Attr);
			} else {
				std::cout << "Bad parse1: " << (uint32_t) (pos+Attr.len) << "  S:" << Size << std::endl;
				return false;
			}
			if(Buffer[pos+1]==0) {
				std::cout << "Bad parse2: " << (uint32_t) (pos+Attr.len) << "  S:" << Size << std::endl;
				return false;
			}
			pos+=Buffer[pos+1];
			x--;
		}
		// std::cout << "Good parse" << std::endl;
		return true;
	}

	class RadiusPacket {
	  public:
		explicit RadiusPacket(const Poco::Buffer<char> & Buf) {
			if(Buf.size() >= sizeof(RawRadiusPacket)) {
				Valid_ = false;
				return;
			}
			memcpy((void *)&P_,Buf.begin(), Buf.size());
			P_.len = htons(P_.len);
			Size_=Buf.size();
			Valid_ = (Size_==P_.len);
			if(Valid_)
				Valid_ = ParseRadius(0,(unsigned char *)&P_.attributes[0],Size_-20,Attrs_);
		}

		explicit RadiusPacket(const unsigned char *buffer, uint16_t size) {
			if(size >= sizeof(RawRadiusPacket)) {
				Valid_ = false;
				return;
			}
			memcpy((void *)&P_,buffer, size);
			P_.len = htons(P_.len);
			Size_=size;
			Valid_ = (Size_==P_.len);
			if(Valid_)
				Valid_ = ParseRadius(0,(unsigned char *)&P_.attributes[0],Size_-20,Attrs_);
		}

		explicit RadiusPacket(const std::string &p) {
			if(p.size() >= sizeof(RawRadiusPacket)) {
				Valid_ = false;
				return;
			}
			memcpy((void *)&P_,(const unsigned char*) p.c_str(), p.size());
			P_.len = htons(P_.len);
			Size_=p.size();
			Valid_ = (Size_==P_.len);
			if(Valid_)
				Valid_ = ParseRadius(0,(unsigned char *)&P_.attributes[0],Size_-20,Attrs_);
		}

		explicit RadiusPacket(const RadiusPacket &P) {
			Valid_ = P.Valid_;
			Size_ = P.Size_;
			P_ = P.P_;
			Attrs_ = P.Attrs_;
		}

		explicit RadiusPacket() = default;

		unsigned char * Buffer() { return (unsigned char *)&P_; }
		[[nodiscard]] uint16_t BufferLen() const { return sizeof(P_);}

		void Evaluate(uint16_t size) {
			Size_ = size;
			Valid_ = ParseRadius(0,(unsigned char *)&P_.attributes[0],Size_-20,Attrs_);
		}

		[[nodiscard]]  uint16_t Len() const { return htons(P_.len); }
		[[nodiscard]]  uint16_t Size() const { return Size_; }

		friend std::ostream &operator<<(std::ostream &os, RadiusPacket const &P);

		void Log(std::ostream &os) {
			uint16_t p = 0;

			while(p<Size_) {
				os << std::setfill('0') << std::setw(4) << p << ":  ";
				uint16_t v=0;
				while(v<16 && p+v<Size_) {
					os << std::setfill('0') << std::setw(2) << std::right << std::hex << (uint16_t )((const unsigned char *)&P_)[p+v] << " ";
					v++;
				}
				os << std::endl;
				p+=16;
			}
			os << std::dec << std::endl << std::endl;
			Print(os);
		}

		void ComputeMessageAuthenticator(const std::string &secret) {
			RawRadiusPacket		P = P_;

			unsigned char OldAuthenticator[16]{0};
			for(const auto &attr:Attrs_) {
				if(attr.type==80) {
					memcpy(OldAuthenticator,&P_.attributes[attr.pos],16);
					memset(&P.attributes[attr.pos],0,16);
				}
			}

			unsigned char NewAuthenticator[16]{0};
			Poco::HMACEngine<Poco::MD5Engine>	H(secret+ "111");
			H.update((const unsigned char *)&P,P.len);
			auto digest = H.digest();
			int p =0;
			for(const auto &i:digest)
				NewAuthenticator[p++]=i;

			if(memcmp(OldAuthenticator,NewAuthenticator,16)==0) {
				std::cout << "Authenticator match..." << std::endl;
			} else {
				std::cout << "Authenticator MIS-match..." << std::endl;
				for(const auto &attr:Attrs_) {
					if(attr.type==80) {
						memcpy(&P_.attributes[attr.pos],NewAuthenticator,16);
					}
				}
			}
		}

		bool VerifyMessageAuthenticator(const std::string &secret) {
			RawRadiusPacket		P = P_;
			unsigned char OldAuthenticator[16]{0};
			for(const auto &attr:Attrs_) {
				if(attr.type==80) {
					memcpy(OldAuthenticator,&P_.attributes[attr.pos],16);
					memset(&P.attributes[attr.pos],0,16);
				}
			}
			unsigned char NewAuthenticator[16]{0};
			Poco::HMACEngine<Poco::MD5Engine>	H(secret);
			H.update((const unsigned char *)&P,P.len);
			auto digest = H.digest();
			int p =0;
			for(const auto &i:digest)
				NewAuthenticator[p++]=i;
			return memcmp(OldAuthenticator,NewAuthenticator,16)==0;
		}

		static void BufLog(std::ostream & os, const char * pre, const unsigned char *b, uint s) {
			uint16_t p = 0;
			while(p<s) {
				os << pre << std::setfill('0') << std::setw(4) << p << ":  ";
				uint16_t v=0;
				while(v<16 && p+v<s) {
					os << std::setfill('0') << std::setw(2) << std::right << std::hex << (uint16_t )b[p+v] << " ";
					v++;
				}
				os << std::endl;
				p+=16;
			}
			os << std::dec ;
		}

		inline void Print(std::ostream &os) {
			os << "Packet type: (" << (uint) P_.code << ") " << CommandName(P_.code) << std::endl;
			os << "  Identifier: " << (uint) P_.identifier << std::endl;
			os << "  Length: " << P_.len << std::endl;
			os << "  Authenticator: " ;
			BufLog(os, "", P_.authenticator, sizeof(P_.authenticator));
			os << "  Attributes: " << std::endl;
			for(const auto &attr:Attrs_) {
				os << "    " << std::setfill(' ') << "(" << std::setw(4) << (uint) attr.type << ") " << AttributeName(attr.type) << "   Len:" << attr.len << std::endl;
				BufLog(os, "           " , &P_.attributes[attr.pos], attr.len);
			}
			os << std::dec << std::endl << std::endl;
 		}

		std::string ExtractSerialNumberTIP() {
			std::string     R;

			for(const auto &attribute:Attrs_) {
				if(attribute.type==26) {
					AttributeList   VendorAttributes;
					uint32_t VendorId = htonl( *(const uint32_t *)&(P_.attributes[attribute.pos]));
					// std::cout << VendorId << std::endl;
					if(VendorId==TIP_vendor_id && attribute.len>(4+2)) {
						if (ParseRadius(attribute.pos + 4, &P_.attributes[attribute.pos + 4], attribute.len - 4 - 2,
										VendorAttributes)) {
							// std::cout << VendorAttributes << std::endl;
							for (const auto &vendorAttr: VendorAttributes) {
								if (vendorAttr.type == TIP_serial) {
									for (uint16_t i = 0; i < vendorAttr.len; i++) {
										if (P_.attributes[vendorAttr.pos + i] == '-')
											continue;
										R += (char) P_.attributes[vendorAttr.pos + i];
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
			for(const auto &attribute:Attrs_) {
				if(attribute.type==33) {
					const char * SN = (const char *)&P_.attributes[attribute.pos];
					auto i=0;
					while(*SN!=':' && i<12) {
						Result+=*SN++;
						i++;
					}
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractProxyStateDestination() {
			std::string Result;
			for(const auto &attribute:Attrs_) {
				if(attribute.type==33 && attribute.len>2) {
					std::string Attr33;
					// format is serial:IP:port:interface
					Attr33.assign((const char *)(const char *)&P_.attributes[attribute.pos],attribute.len-2);
					auto Parts = Poco::StringTokenizer(Attr33,":");
					if(Parts.count()==4)
						return Parts[1]+":"+Parts[2];
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractCallingStationID() {
			std::string Result;
			for(const auto &attribute:Attrs_) {
				if(attribute.type==31 && attribute.len>2) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],attribute.len-2);
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractCalledStationID() {
			std::string Result;
			for(const auto &attribute:Attrs_) {
				if(attribute.type==30 && attribute.len>2) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],attribute.len-2);
					return Result;
				}
			}
			return Result;
		}

	  private:
		RawRadiusPacket     P_;
		uint16_t            Size_{0};
		AttributeList       Attrs_;
		bool                Valid_=false;
	};

	inline std::ostream &operator<<(std::ostream &os, RadiusPacket const &P) {
		os << P.Attrs_ ;
		return os;
	}
}