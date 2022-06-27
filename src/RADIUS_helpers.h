//
// Created by stephane bourque on 2022-06-20.
//

#pragma once

#include <list>
#include <string>
#include <iostream>
#include <iomanip>

namespace OpenWifi::RADIUS {

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

	//
	// From: https://github.com/Telecominfraproject/wlan-dictionary/blob/main/dictionary.tip
	//
	static const uint32_t TIP_vendor_id = 58888;
	static const unsigned char TIP_serial = 1;
	static const unsigned char TIP_AAAipaddr = 2;
	static const unsigned char TIP_AAAipv6addr = 3;


	using AttributeList = std::list<RadiusAttribute>;

	std::ostream &operator<<(std::ostream &os, AttributeList const &P) {
		for(const auto &attr:P) {
			os << "\tAttr: " << (uint16_t) attr.type << "  Size: " << (uint16_t) attr.len << std::endl;
		}
		return os;
	}

	bool ParseRadius(uint32_t offset, const unsigned char *Buffer, uint16_t Size, AttributeList &Attrs) {
		Attrs.clear();
		uint16_t pos=0;
		auto x=25;
		while(pos<Size && x) {
			RadiusAttribute Attr{ .type=Buffer[pos], .pos=(uint16_t )(pos+2+offset), .len=Buffer[pos+1]};
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
		explicit RadiusPacket(const unsigned char *buffer, uint16_t size) {
			memcpy((void *)&P_,buffer, size);
			Size_=size;
			Valid_ = ParseRadius(0,(unsigned char *)&P_.attributes[0],Size_-20,Attrs_);
		}

		explicit RadiusPacket(const std::string &p) {
			memcpy((void *)&P_,(const unsigned char*) p.c_str(), p.size());
			Size_=p.size();
			Valid_ = ParseRadius(0,(unsigned char *)&P_.attributes[0],Size_-20,Attrs_);
		}

		RadiusPacket() = default;

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
			os << std::dec << std::endl;
		}

		std::string ExtractSerialNumberTIP() {
			std::string     R;

			for(const auto &attribute:Attrs_) {
				if(attribute.type==26) {
					AttributeList   VendorAttributes;
					uint32_t VendorId = htonl( *(const uint32_t *)&(P_.attributes[attribute.pos]));
					// std::cout << VendorId << std::endl;
					if(VendorId==TIP_vendor_id) {
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
				if(attribute.type==33 && attribute.len>0) {
					std::string Attr33;
					Attr33.assign((const char *)(const char *)&P_.attributes[attribute.pos],attribute.len);
					auto Parts = Poco::StringTokenizer(Attr33,":");
					return Parts[1]+":"+Parts[2];
				}
			}
			return Result;
		}

		std::string ExtractCallingStationID() {
			std::string Result;
			for(const auto &attribute:Attrs_) {
				if(attribute.type==31 && attribute.len>0) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],attribute.len-1);
					return Result;
				}
			}
			return Result;
		}

		std::string ExtractCalledStationID() {
			std::string Result;
			for(const auto &attribute:Attrs_) {
				if(attribute.type==30 && attribute.len>0) {
					Result.assign((const char *)(const char *)&P_.attributes[attribute.pos],attribute.len-1);
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

	std::ostream &operator<<(std::ostream &os, RadiusPacket const &P) {
		os << P.Attrs_ ;
		return os;
	}
}