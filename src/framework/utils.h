//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <random>
#include <regex>
#include <thread>
#include <shared_mutex>

#include "Poco/Thread.h"
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include "Poco/SHA2Engine.h"
#include "Poco/Message.h"
#include "Poco/StreamCopier.h"
#include "Poco/File.h"
#include "Poco/Net/NetworkInterface.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Base64Decoder.h"
#include "Poco/Base64Encoder.h"
#include "Poco/zlib.h"

#include "framework/ow_constants.h"
#include "framework/OpenWifiTypes.h"

namespace OpenWifi::Utils {

	inline uint64_t Now() { return std::time(nullptr); };

	bool NormalizeMac(std::string & Mac);

	inline void SetThreadName(const char *name) {
	#ifdef __linux__
		Poco::Thread::current()->setName(name);
		pthread_setname_np(pthread_self(), name);
	#endif
	#ifdef __APPLE__
		Poco::Thread::current()->setName(name);
		pthread_setname_np(name);
	#endif
	}

	inline void SetThreadName(Poco::Thread &thr, const char *name) {
	#ifdef __linux__
		thr.setName(name);
		pthread_setname_np(thr.tid(), name);
	#endif
	#ifdef __APPLE__
		thr.setName(name);
	#endif
	}

	enum MediaTypeEncodings {
		PLAIN,
		BINARY,
		BASE64
	};

	struct MediaTypeEncoding {
		MediaTypeEncodings  Encoding=PLAIN;
		std::string         ContentType;
	};

	[[nodiscard]] bool ValidSerialNumber(const std::string &Serial);
	[[nodiscard]] bool ValidUUID(const std::string &UUID);

	template <typename ...Args> std::string ComputeHash(Args&&... args) {
		Poco::SHA2Engine    E;
		auto as_string = [](auto p) {
			if constexpr(std::is_arithmetic_v<decltype(p)>) {
				return std::to_string(p);
			} else {
				return p;
			}
		};
		(E.update(as_string(args)),...);
		return Poco::SHA2Engine::digestToHex(E.digest());
	}

	[[nodiscard]] std::vector<std::string> Split(const std::string &List, char Delimiter=',' );
	[[nodiscard]] std::string FormatIPv6(const std::string & I );
	void padTo(std::string& str, size_t num, char paddingChar = '\0');
	[[nodiscard]] std::string SerialToMAC(const std::string &Serial);
	uint64_t MACToInt(const std::string &MAC);
	[[nodiscard]] std::string ToHex(const std::vector<unsigned char> & B);

	using byte = std::uint8_t;

	[[nodiscard]] std::string base64encode(const byte *input, uint32_t size);
	[[nodiscard]] std::vector<byte> base64decode(const std::string& input);;
	bool ParseTime(const std::string &Time, int & Hours, int & Minutes, int & Seconds);
	bool ParseDate(const std::string &Time, int & Year, int & Month, int & Day);
	bool CompareTime( int H1, int H2, int M1, int M2, int S1, int S2);
	[[nodiscard]] std::string LogLevelToString(int Level);
	[[nodiscard]] uint64_t SerialNumberToInt(const std::string & S);
	[[nodiscard]] std::string IntToSerialNumber(uint64_t S);
	[[nodiscard]] bool SerialNumberMatch(const std::string &S1, const std::string &S2, int Bits=2);
	[[nodiscard]] uint64_t SerialNumberToOUI(const std::string & S);
	[[nodiscard]] uint64_t GetDefaultMacAsInt64();
	[[nodiscard]] uint64_t InitializeSystemId();
	[[nodiscard]] uint64_t GetSystemId();
	[[nodiscard]] bool ValidEMailAddress(const std::string &email);
	[[nodiscard]] std::string LoadFile( const Poco::File & F);
	void ReplaceVariables( std::string & Content , const Types::StringPairVec & P);
	[[nodiscard]] MediaTypeEncoding FindMediaType(const Poco::File &F);
	[[nodiscard]] std::string BinaryFileToHexString(const Poco::File &F);
	[[nodiscard]] std::string SecondsToNiceText(uint64_t Seconds);
	[[nodiscard]] bool wgets(const std::string &URL, std::string &Response);
	[[nodiscard]] bool wgetfile(const Poco::URI &uri, const std::string &FileName);
	[[nodiscard]] bool IsAlphaNumeric(const std::string &s);
    [[nodiscard]] std::string SanitizeToken(const std::string &Token);
	[[nodiscard]] bool ValidateURI(const std::string &uri);

	template< typename T >
	std::string int_to_hex( T i )
	{
		std::stringstream stream;
		stream << std::setfill ('0') << std::setw(12)
			   << std::hex << i;
		return stream.str();
	}

	inline bool SpinLock_Read(std::shared_mutex &M, volatile bool &Flag, uint64_t wait_ms=100) {
		while(!M.try_lock_shared() && Flag) {
			Poco::Thread::yield();
			Poco::Thread::trySleep((long)wait_ms);
		}
		return Flag;
	}

	inline bool SpinLock_Write(std::shared_mutex &M, volatile bool &Flag, uint64_t wait_ms=100) {
		while(!M.try_lock() && Flag) {
			Poco::Thread::yield();
			Poco::Thread::trySleep(wait_ms);
		}
		return Flag;
	}

	bool ExtractBase64CompressedData(const std::string &CompressedData,
											std::string &UnCompressedData, uint64_t compress_sz );
}
