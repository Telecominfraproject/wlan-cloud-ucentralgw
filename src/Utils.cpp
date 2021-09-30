//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#include <stdexcept>
#include <fstream>
#include <cstdlib>
#include <regex>
#include <random>
#include <chrono>

#include "Utils.h"

#include "Poco/Exception.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeParser.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Message.h"
#include "Poco/File.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPResponse.h"

#include "uCentralProtocol.h"
#include "Daemon.h"

namespace OpenWifi::Utils {

	[[nodiscard]] bool ValidSerialNumber(const std::string &Serial) {
		return ((Serial.size() < uCentralProtocol::SERIAL_NUMBER_LENGTH) &&
				std::all_of(Serial.begin(),Serial.end(),[](auto i){return std::isxdigit(i);}));
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

	inline static const char kEncodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	inline static const char kPadCharacter = '=';

	std::string base64encode(const byte *input, unsigned long size) {
		std::string encoded;
		encoded.reserve(((size / 3) + (size % 3 > 0)) * 4);

		std::uint32_t temp;

		std::size_t i;

		int ee = (int)(size/3);

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

	std::vector<byte> base64decode(const std::string& input)
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

	std::string to_RFC3339(uint64_t t)
	{
		if(t==0)
			return "";
		return Poco::DateTimeFormatter::format(Poco::DateTime(Poco::Timestamp::fromEpochTime(t)), Poco::DateTimeFormat::ISO8601_FORMAT);
	}

	uint64_t from_RFC3339(const std::string &TimeString)
	{
		if(TimeString.empty() || TimeString=="0")
			return 0;

		try {
			int             TZ;
			Poco::DateTime  DT = Poco::DateTimeParser::parse(Poco::DateTimeFormat::ISO8601_FORMAT,TimeString,TZ);
			return DT.timestamp().epochTime();
		}
		catch( const Poco::Exception & E )
		{

		}
		return 0;
	}

	bool ParseTime(const std::string &Time, int & Hours, int & Minutes, int & Seconds) {
		Poco::StringTokenizer	TimeTokens(Time,":",Poco::StringTokenizer::TOK_TRIM);

		Hours =  Minutes = Hours = 0 ;
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

	std::string LogLevelToString(int Level) {
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

	bool SerialNumberMatch(const std::string &S1, const std::string &S2, int Bits) {
		auto S1_i = SerialNumberToInt(S1);
		auto S2_i = SerialNumberToInt(S2);
		return ((S1_i>>Bits)==(S2_i>>Bits));
	}

	uint64_t SerialNumberToInt(const std::string & S) {
		uint64_t R=0;

		for(const auto &i:S)
			if(i>='0' && i<='9') {
				R <<= 4;
				R += (i-'0');
			} else if(i>='a' && i<='f') {
				R <<= 4;
				R += (i-'a') + 10 ;
			} else if(i>='A' && i<='F') {
				R <<= 4;
				R += (i-'A') + 10 ;
			}
		return R;
	}

	uint64_t SerialNumberToOUI(const std::string & S) {
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

	uint64_t GetDefaultMacAsInt64() {
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

	void SaveSystemId(uint64_t Id) {
		try {
			std::ofstream O;
			O.open(Daemon()->DataDir() + "/system.id",std::ios::binary | std::ios::trunc);
			O << Id;
			O.close();
		} catch (...)
		{
			std::cout << "Could not save system ID" << std::endl;
		}
	}

	uint64_t InitializeSystemId() {
		std::random_device	RDev;
		std::srand(RDev());
		std::chrono::high_resolution_clock	Clock;
		auto Now = Clock.now().time_since_epoch().count();
		auto S = (GetDefaultMacAsInt64() + std::rand() + Now)  ;
		SaveSystemId(S);
		std::cout << "ID: " << S << std::endl;
		return S;
	}

	uint64_t GetSystemId() {
		uint64_t ID=0;

		// if the system ID file exists, open and read it.
		Poco::File	SID( Daemon()->DataDir() + "/system.id");
		try {
			if (SID.exists()) {
				std::ifstream I;
				I.open(SID.path());
				I >> ID;
				I.close();
				if (ID == 0)
					return InitializeSystemId();
				return ID;
			} else {
				return InitializeSystemId();
			}
		} catch (...) {
			return InitializeSystemId();
		}
	}

	bool ValidEMailAddress(const std::string &email) {
		// define a regular expression
		const std::regex pattern
			("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");

		// try to match the string with the regular expression
		return std::regex_match(email, pattern);
	}

	std::string LoadFile( const Poco::File & F) {
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

    MediaTypeEncoding FindMediaType(const Poco::File &F) {
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

	std::string BinaryFileToHexString(const Poco::File &F) {
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

    std::string SecondsToNiceText(uint64_t Seconds) {
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

	bool wgets(const std::string &URL, std::string &Response) {
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

}
