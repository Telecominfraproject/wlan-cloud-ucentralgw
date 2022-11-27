//
// Created by stephane bourque on 2022-10-25.
//

#include "Poco/Path.h"

#include "framework/utils.h"
#include "framework/AppServiceRegistry.h"

namespace OpenWifi::Utils {

bool NormalizeMac(std::string & Mac) {
	Poco::replaceInPlace(Mac,":","");
	Poco::replaceInPlace(Mac,"-","");
	if(Mac.size()!=12)
		return false;
	for(const auto &i:Mac) {
		if(!std::isxdigit(i))
			return false;
	}
	Poco::toLowerInPlace(Mac);
	return true;
}

[[nodiscard]] bool ValidSerialNumber(const std::string &Serial) {
	return ((Serial.size() < uCentralProtocol::SERIAL_NUMBER_LENGTH) &&
			std::all_of(Serial.begin(),Serial.end(),[](auto i){return std::isxdigit(i);}));
}

[[nodiscard]] bool ValidUUID(const std::string &UUID) {
	if(UUID.size()>36)
		return false;
	uint dashes=0;
	return (std::all_of(UUID.begin(),UUID.end(),[&](auto i){ if(i=='-') dashes++; return i=='-' || std::isxdigit(i);})) && (dashes>0);
}

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

void padTo(std::string& str, size_t num, char paddingChar) {
	str.append(num - str.length() % num, paddingChar);
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

uint64_t MACToInt(const std::string &MAC) {
	uint64_t Result = 0 ;
	for(const auto &c:MAC) {
		if(c==':')
			continue;
		Result <<= 4;
		if(c>='0' && c<='9') {
			Result += (c - '0');
		} else if (c>='a' && c<='f') {
			Result += (c-'a'+10);
		} else if (c>='A' && c<='F') {
			Result += (c-'A'+10);
		}
	}
	return Result;
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

using byte = std::uint8_t;

[[nodiscard]] std::string base64encode(const byte *input, uint32_t size) {
	std::string encoded;
	encoded.reserve(((size / 3) + (size % 3 > 0)) * 4);

	std::uint32_t temp,i,ee;
	ee = (size/3);

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

[[nodiscard]] std::vector<byte> base64decode(const std::string& input)
{
	if(input.length() % 4)
		throw std::runtime_error("Invalid base64 length!");

	std::size_t padding=0;

	if(input.length())
	{
		if(input[input.length() - 1] == kPadCharacter) padding++;
		if(input[input.length() - 2] == kPadCharacter) padding++;
	}

	std::vector<byte> decoded;
	decoded.reserve(((input.length() / 4) * 3) - padding);

	std::uint32_t temp=0;
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

bool ParseTime(const std::string &Time, int & Hours, int & Minutes, int & Seconds) {
	Poco::StringTokenizer	TimeTokens(Time,":",Poco::StringTokenizer::TOK_TRIM);

	Hours =  Minutes = Seconds = 0 ;
	if(TimeTokens.count()==1) {
		Hours 	= std::atoi(TimeTokens[0].c_str());
	} else if(TimeTokens.count()==2) {
		Hours 	= std::atoi(TimeTokens[0].c_str());
		Minutes = std::atoi(TimeTokens[1].c_str());
	} else if(TimeTokens.count()==3) {
		Hours 	= std::atoi(TimeTokens[0].c_str());
		Minutes = std::atoi(TimeTokens[1].c_str());
		Seconds = std::atoi(TimeTokens[2].c_str());
	} else
		return false;
	return true;
}


bool ParseDate(const std::string &Time, int & Year, int & Month, int & Day) {
	Poco::StringTokenizer	DateTokens(Time,"-",Poco::StringTokenizer::TOK_TRIM);

	Year =  Month = Day = 0 ;
	if(DateTokens.count()==3) {
		Year 	= std::atoi(DateTokens[0].c_str());
		Month 	= std::atoi(DateTokens[1].c_str());
		Day 	= std::atoi(DateTokens[2].c_str());
	} else
		return false;
	return true;
}

bool CompareTime( int H1, int H2, int M1, int M2, int S1, int S2) {
	if(H1<H2)
		return true;
	if(H1>H2)
		return false;
	if(M1<M2)
		return true;
	if(M2>M1)
		return false;
	if(S1<=S2)
		return true;
	return false;
}

[[nodiscard]] std::string LogLevelToString(int Level) {
	switch(Level) {
	case Poco::Message::PRIO_DEBUG: return "debug";
	case Poco::Message::PRIO_INFORMATION: return "information";
	case Poco::Message::PRIO_FATAL: return "fatal";
	case Poco::Message::PRIO_WARNING: return "warning";
	case Poco::Message::PRIO_NOTICE: return "notice";
	case Poco::Message::PRIO_CRITICAL: return "critical";
	case Poco::Message::PRIO_ERROR: return "error";
	case Poco::Message::PRIO_TRACE: return "trace";
	default: return "none";
	}
}

[[nodiscard]] uint64_t SerialNumberToInt(const std::string & S) {
	return std::stoull(S,nullptr,16);
}

[[nodiscard]] std::string IntToSerialNumber(uint64_t S) {
	char b[16];
	for(int i=0;i<12;++i) {
		int B = (S & 0x0f);
		if(B<10)
			b[11-i] = B+'0';
		else
			b[11-i] = B - 10 + 'a';
		S >>= 4 ;
	}
	b[12]=0;
	return b;
}


[[nodiscard]] bool SerialNumberMatch(const std::string &S1, const std::string &S2, int Bits) {
	auto S1_i = SerialNumberToInt(S1);
	auto S2_i = SerialNumberToInt(S2);
	return ((S1_i>>Bits)==(S2_i>>Bits));
}

[[nodiscard]] uint64_t SerialNumberToOUI(const std::string & S) {
	uint64_t Result = 0 ;
	int Digits=0;

	for(const auto &i:S) {
		if(std::isxdigit(i)) {
			if(i>='0' && i<='9') {
				Result <<=4;
				Result += i-'0';
			} else if(i>='A' && i<='F') {
				Result <<=4;
				Result += i-'A'+10;
			} else if(i>='a' && i<='f') {
				Result <<=4;
				Result += i-'a'+10;
			}
			Digits++;
			if(Digits==6)
				break;
		}
	}
	return Result;
}

[[nodiscard]] uint64_t GetDefaultMacAsInt64() {
	uint64_t Result=0;
	auto IFaceList = Poco::Net::NetworkInterface::list();

	for(const auto &iface:IFaceList) {
		if(iface.isRunning() && !iface.isLoopback()) {
			auto MAC = iface.macAddress();
			for (auto const &i : MAC) {
				Result <<= 8;
				Result += (uint8_t)i;
			}
			if (Result != 0)
				break;
		}
	}
	return Result;
}

[[nodiscard]] uint64_t InitializeSystemId() {
	std::random_device	RDev;
	std::srand(RDev());
	std::chrono::high_resolution_clock	Clock;
	auto Now = Clock.now().time_since_epoch().count();
	auto S = (GetDefaultMacAsInt64() + std::rand() + Now)  ;
	OpenWifi::AppServiceRegistry().Set("systemid",S);
	return S;
}

[[nodiscard]] uint64_t GetSystemId() {
	uint64_t ID=0;
	if(!AppServiceRegistry().Get("systemid",ID)) {
		return InitializeSystemId();
	}
	return ID;
}

[[nodiscard]] bool ValidEMailAddress(const std::string &email) {
	// define a regular expression
	static const std::regex pattern
		("[_a-z0-9-]+(\\.[_a-z0-9-]+)*(\\+[a-z0-9-]+)?@[a-z0-9-]+(\\.[a-z0-9-]+)*");
	// try to match the string with the regular expression
	return std::regex_match(email, pattern);
}

[[nodiscard]] std::string LoadFile( const Poco::File & F) {
	std::string Result;
	try {
		std::ostringstream OS;
		std::ifstream   IF(F.path());
		Poco::StreamCopier::copyStream(IF, OS);
		Result = OS.str();
	} catch (...) {

	}
	return Result;
}

void ReplaceVariables( std::string & Content , const Types::StringPairVec & P) {
	for(const auto &[Variable,Value]:P) {
		Poco::replaceInPlace(Content,"${" + Variable + "}", Value);
	}
}

[[nodiscard]] MediaTypeEncoding FindMediaType(const Poco::File &F) {
	const auto E = Poco::Path(F.path()).getExtension();
	if(E=="png")
		return MediaTypeEncoding{   .Encoding = BINARY,
								 .ContentType = "image/png" };
	if(E=="gif")
		return MediaTypeEncoding{   .Encoding = BINARY,
								 .ContentType = "image/gif" };
	if(E=="jpeg" || E=="jpg")
		return MediaTypeEncoding{   .Encoding = BINARY,
								 .ContentType = "image/jpeg" };
	if(E=="svg" || E=="svgz")
		return MediaTypeEncoding{   .Encoding = PLAIN,
								 .ContentType = "image/svg+xml" };
	if(E=="html")
		return MediaTypeEncoding{   .Encoding = PLAIN,
								 .ContentType = "text/html" };
	if(E=="css")
		return MediaTypeEncoding{   .Encoding = PLAIN,
								 .ContentType = "text/css" };
	if(E=="js")
		return MediaTypeEncoding{   .Encoding = PLAIN,
								 .ContentType = "application/javascript" };
	return MediaTypeEncoding{       .Encoding = BINARY,
							 .ContentType = "application/octet-stream" };
}

[[nodiscard]] std::string BinaryFileToHexString(const Poco::File &F) {
	static const char hex[] = "0123456789abcdef";
	std::string Result;
	try {
		std::ifstream IF(F.path());

		int Count = 0;
		while (IF.good()) {
			if (Count)
				Result += ", ";
			if ((Count % 32) == 0)
				Result += "\r\n";
			Count++;
			unsigned char C = IF.get();
			Result += "0x";
			Result += (char) (hex[(C & 0xf0) >> 4]);
			Result += (char) (hex[(C & 0x0f)]);
		}
	} catch(...) {

	}
	return Result;
}

[[nodiscard]] std::string SecondsToNiceText(uint64_t Seconds) {
	std::string Result;
	int Days = Seconds / (24*60*60);
	Seconds -= Days * (24*60*60);
	int Hours= Seconds / (60*60);
	Seconds -= Hours * (60*60);
	int Minutes = Seconds / 60;
	Seconds -= Minutes * 60;
	Result = std::to_string(Days) +" days, " + std::to_string(Hours) + ":" + std::to_string(Minutes) + ":" + std::to_string(Seconds);
	return Result;
}

[[nodiscard]] bool wgets(const std::string &URL, std::string &Response) {
	try {
		Poco::URI uri(URL);
		Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

		// prepare path
		std::string path(uri.getPathAndQuery());
		if (path.empty()) {
			path = "/";
		}

		// send request
		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);
		session.sendRequest(req);

		Poco::Net::HTTPResponse res;
		std::istream &is = session.receiveResponse(res);
		std::ostringstream os;

		Poco::StreamCopier::copyStream(is,os);
		Response = os.str();

		return true;
	} catch (...) {

	}
	return false;
}

[[nodiscard]] bool wgetfile(const Poco::URI &uri, const std::string &FileName) {
	try {
		Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());

		// send request
		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, uri.getPath(), Poco::Net::HTTPMessage::HTTP_1_1);
		session.sendRequest(req);

		Poco::Net::HTTPResponse res;
		std::istream &is = session.receiveResponse(res);
		std::fstream os(FileName,std::ios_base::trunc | std::ios_base::binary | std::ios_base::out);
		Poco::StreamCopier::copyStream(is,os);
		return true;
	} catch (...) {

	}
	return false;
}

bool ExtractBase64CompressedData(const std::string &CompressedData,
										std::string &UnCompressedData, uint64_t compress_sz ) {
	std::istringstream ifs(CompressedData);
	Poco::Base64Decoder b64in(ifs);
	std::ostringstream ofs;
	Poco::StreamCopier::copyStream(b64in, ofs);

	int factor = 20;
	unsigned long MaxSize = compress_sz ? (unsigned long) (compress_sz + 5000) : (unsigned long) (ofs.str().size() * factor);
	while(true) {
		std::vector<uint8_t> UncompressedBuffer(MaxSize);
		unsigned long FinalSize = MaxSize;
		auto status = uncompress((uint8_t *)&UncompressedBuffer[0], &FinalSize,
								 (uint8_t *)ofs.str().c_str(), ofs.str().size());
		if(status==Z_OK) {
			UncompressedBuffer[FinalSize] = 0;
			UnCompressedData = (char *)&UncompressedBuffer[0];
			return true;
		}
		if(status==Z_BUF_ERROR) {
			if(factor<300) {
				factor+=10;
				MaxSize = ofs.str().size() * factor;
				continue;
			} else {
				return false;
			}
		}
		return false;
	}
	return false;
}

    bool IsAlphaNumeric(const std::string &s) {
        return std::all_of(s.begin(),s.end(),[](char c) -> bool { return isalnum(c); });
    }

    std::string SanitizeToken(const std::string &Token) {
        if(Token.size()>8) {
            return Token.substr(0,4) + "****" + Token.substr(Token.size()-4,4);
        }
        return "*******";
    }

	[[nodiscard]] bool ValidateURI(const std::string &uri) {
		try {
			Poco::URI	u(uri);
			return true;
		} catch (...) {

		}
		return false;
	}

}
