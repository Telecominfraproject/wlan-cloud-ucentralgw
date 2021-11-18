//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#pragma once

#include <array>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <set>
#include <thread>
#include <chrono>
#include <fstream>
#include <regex>
#include <random>
#include <iomanip>
#include <queue>

using namespace std::chrono_literals;

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Crypto/RSAKey.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/SHA2Engine.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Process.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/Socket.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/PrivateKeyPassphraseHandler.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/FTPStreamFactory.h"
#include "Poco/Net/FTPSStreamFactory.h"
#include "Poco/Net/HTTPSStreamFactory.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/File.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/OAuth20Credentials.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/TemporaryFile.h"
#include "Poco/NullStream.h"
#include "Poco/CountingStream.h"
#include "Poco/URI.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/NetworkInterface.h"
#include "Poco/ExpireLRUCache.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/StringTokenizer.h"

#include "cppkafka/cppkafka.h"

#include "framework/KafkaTopics.h"
#include "framework/RESTAPI_protocol.h"
#include "framework/RESTAPI_errors.h"
#include "framework/uCentral_Protocol.h"
#include "RESTObjects/RESTAPI_SecurityObjects.h"
#include "nlohmann/json.hpp"

namespace OpenWifi {

    enum UNAUTHORIZED_REASON {
        SUCCESS=0,
        PASSWORD_CHANGE_REQUIRED,
        INVALID_CREDENTIALS,
        PASSWORD_ALREADY_USED,
        USERNAME_PENDING_VERIFICATION,
        PASSWORD_INVALID,
        INTERNAL_ERROR,
        ACCESS_DENIED,
        INVALID_TOKEN,
        EXPIRED_TOKEN,
        RATE_LIMIT_EXCEEDED
    };

	class AppServiceRegistry {
	  public:
		inline AppServiceRegistry();

		static AppServiceRegistry & instance() {
		    static AppServiceRegistry *instance_= new AppServiceRegistry;
			return *instance_;
		}

		inline ~AppServiceRegistry() {
			Save();
		}

		inline void Save() {
			std::istringstream  IS( to_string(Registry_));
			std::ofstream       OF;
			OF.open(FileName,std::ios::binary | std::ios::trunc);
			Poco::StreamCopier::copyStream(IS, OF);
		}

		inline void Set(const char *Key, uint64_t Value ) {
			Registry_[Key] = Value;
			Save();
		}

		inline void Set(const char *Key, const std::string &Value ) {
			Registry_[Key] = Value;
			Save();
		}

		inline void Set(const char *Key, bool Value ) {
			Registry_[Key] = Value;
			Save();
		}

		inline bool Get(const char *Key, bool & Value ) {
			if(Registry_[Key].is_boolean()) {
				Value = Registry_[Key].get<bool>();
				return true;
			}
			return false;
		}

		inline bool Get(const char *Key, uint64_t & Value ) {
			if(Registry_[Key].is_number_unsigned()) {
				Value = Registry_[Key].get<uint64_t>();
				return true;
			}
			return false;
		}

		inline bool Get(const char *Key, std::string & Value ) {
			if(Registry_[Key].is_string()) {
				Value = Registry_[Key].get<std::string>();
				return true;
			}
			return false;
		}

	  private:
		std::string         FileName;
		nlohmann::json      Registry_;
	};
}

namespace OpenWifi::RESTAPI_utils {

    inline void EmbedDocument(const std::string & ObjName, Poco::JSON::Object & Obj, const std::string &ObjStr) {
        std::string D = ObjStr.empty() ? "{}" : ObjStr;
        Poco::JSON::Parser P;
        Poco::Dynamic::Var result = P.parse(D);
        const auto &DetailsObj = result.extract<Poco::JSON::Object::Ptr>();
        Obj.set(ObjName, DetailsObj);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, bool V) {
        Obj.set(Field,V);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const std::string & S) {
        Obj.set(Field,S);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::StringPairVec & S) {
        Poco::JSON::Array   Array;
        for(const auto &i:S) {
            Poco::JSON::Object  O;
            O.set("tag",i.first);
            O.set("value", i.second);
            Array.add(O);
        }
        Obj.set(Field,Array);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const char * S) {
        Obj.set(Field,S);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, uint64_t V) {
        Obj.set(Field,V);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::StringVec &V) {
        Poco::JSON::Array	A;
        for(const auto &i:V)
            A.add(i);
        Obj.set(Field,A);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::TagList &V) {
        Poco::JSON::Array	A;
        for(const auto &i:V)
            A.add(i);
        Obj.set(Field,A);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, const Types::CountedMap &M) {
        Poco::JSON::Array	A;
        for(const auto &[Key,Value]:M) {
            Poco::JSON::Object  O;
            O.set("tag",Key);
            O.set("value", Value);
            A.add(O);
        }
        Obj.set(Field,A);
    }

    template<typename T> void field_to_json(Poco::JSON::Object &Obj,
            const char *Field,
            const T &V,
            std::function<std::string(const T &)> F) {
        Obj.set(Field, F(V));
    }

    template<typename T> bool field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, T & V,
            std::function<T(const std::string &)> F) {
        if(Obj->has(Field))
            V = F(Obj->get(Field).toString());
        return true;
    }

    inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, std::string &S) {
        if(Obj->has(Field))
            S = Obj->get(Field).toString();
    }

    inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, uint64_t &V) {
        if(Obj->has(Field))
            V = Obj->get(Field);
    }

    inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, bool &V) {
        if(Obj->has(Field))
            V = (Obj->get(Field).toString() == "true");
    }


    inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, Types::StringPairVec &Vec) {
        if(Obj->isArray(Field)) {
            auto O = Obj->getArray(Field);
            for(const auto &i:*O) {
                std::string S1,S2;
                auto Inner = i.extract<Poco::JSON::Object::Ptr>();
                if(Inner->has("tag"))
                    S1 = Inner->get("tag").toString();
                if(Inner->has("value"))
                    S2 = Inner->get("value").toString();
                auto P = std::make_pair(S1,S2);
                Vec.push_back(P);
            }
        }
    }

    inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, Types::StringVec &V) {
        if(Obj->isArray(Field)) {
            V.clear();
            Poco::JSON::Array::Ptr A = Obj->getArray(Field);
            for(const auto &i:*A) {
                V.push_back(i.toString());
            }
        }
    }

    inline void field_from_json(Poco::JSON::Object::Ptr Obj, const char *Field, Types::TagList &V) {
        if(Obj->isArray(Field)) {
            V.clear();
            Poco::JSON::Array::Ptr A = Obj->getArray(Field);
            for(const auto &i:*A) {
                V.push_back(i);
            }
        }
    }

    template<class T> void field_to_json(Poco::JSON::Object &Obj, const char *Field, const std::vector<T> &Value) {
        Poco::JSON::Array Arr;
        for(const auto &i:Value) {
            Poco::JSON::Object	AO;
            i.to_json(AO);
            Arr.add(AO);
        }
        Obj.set(Field, Arr);
    }

    inline void field_to_json(Poco::JSON::Object &Obj, const char *Field, int Value) {
        Obj.set(Field, Value);
    }

    template<class T> void field_to_json(Poco::JSON::Object &Obj, const char *Field, const T &Value) {
        Poco::JSON::Object  Answer;
        Value.to_json(Answer);
        Obj.set(Field, Answer);
    }

    template<class T> void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, std::vector<T> &Value) {
        if(Obj->isArray(Field)) {
            Poco::JSON::Array::Ptr	Arr = Obj->getArray(Field);
            for(auto &i:*Arr) {
                auto InnerObj = i.extract<Poco::JSON::Object::Ptr>();
                T	NewItem;
                NewItem.from_json(InnerObj);
                Value.push_back(NewItem);
            }
        }
    }

    inline void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, int &Value) {
        if(Obj->isObject(Field)) {
            Value = Obj->get(Field);
        }
    }

    template<class T> void field_from_json(const Poco::JSON::Object::Ptr &Obj, const char *Field, T &Value) {
        if(Obj->isObject(Field)) {
            Poco::JSON::Object::Ptr	A = Obj->getObject(Field);
            Value.from_json(A);
        }
    }

    inline std::string to_string(const Types::TagList & ObjectArray) {
        Poco::JSON::Array OutputArr;
        if(ObjectArray.empty())
            return "[]";
        for(auto const &i:ObjectArray) {
            OutputArr.add(i);
        }
        std::ostringstream OS;
        Poco::JSON::Stringifier::stringify(OutputArr,OS, 0,0, Poco::JSON_PRESERVE_KEY_ORDER );
        return OS.str();
    }

    inline std::string to_string(const Types::StringVec & ObjectArray) {
        Poco::JSON::Array OutputArr;
        if(ObjectArray.empty())
            return "[]";
        for(auto const &i:ObjectArray) {
            OutputArr.add(i);
        }
        std::ostringstream OS;
        Poco::JSON::Stringifier::condense(OutputArr,OS);
        return OS.str();
    }

    inline std::string to_string(const Types::StringPairVec & ObjectArray) {
        Poco::JSON::Array OutputArr;
        if(ObjectArray.empty())
            return "[]";
        for(auto const &i:ObjectArray) {
            Poco::JSON::Array InnerArray;
            InnerArray.add(i.first);
            InnerArray.add(i.second);
            OutputArr.add(InnerArray);
        }
        std::ostringstream OS;
        Poco::JSON::Stringifier::condense(OutputArr,OS);
        return OS.str();
    }

    template<class T> std::string to_string(const std::vector<T> & ObjectArray) {
        Poco::JSON::Array OutputArr;
        if(ObjectArray.empty())
            return "[]";
        for(auto const &i:ObjectArray) {
            Poco::JSON::Object O;
            i.to_json(O);
            OutputArr.add(O);
        }
        std::ostringstream OS;
        Poco::JSON::Stringifier::condense(OutputArr,OS);
        return OS.str();
    }

    template<class T> std::string to_string(const std::vector<std::vector<T>> & ObjectArray) {
        Poco::JSON::Array OutputArr;
        if(ObjectArray.empty())
            return "[]";
        for(auto const &i:ObjectArray) {
            Poco::JSON::Array InnerArr;
            for(auto const &j:i) {
                if constexpr(std::is_integral<T>::value) {
                    InnerArr.add(j);
                } if constexpr(std::is_same_v<T,std::string>) {
                    InnerArr.add(j);
                } else {
                    InnerArr.add(j);
                    Poco::JSON::Object O;
                    j.to_json(O);
                    InnerArr.add(O);
                }
            }
            OutputArr.add(InnerArr);
        }
        std::ostringstream OS;
        Poco::JSON::Stringifier::condense(OutputArr,OS);
        return OS.str();
    }

    template<class T> std::string to_string(const T & Object) {
        Poco::JSON::Object OutputObj;
        Object.to_json(OutputObj);
        std::ostringstream OS;
        Poco::JSON::Stringifier::condense(OutputObj,OS);
        return OS.str();
    }

    inline Types::StringVec to_object_array(const std::string & ObjectString) {

        Types::StringVec 	Result;
        if(ObjectString.empty())
            return Result;

        try {
            Poco::JSON::Parser P;
            auto Object = P.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
            for (auto const &i : *Object) {
                Result.push_back(i.toString());
            }
        } catch (...) {

        }
        return Result;
    }

    inline OpenWifi::Types::TagList to_taglist(const std::string & ObjectString) {
        Types::TagList 	Result;
        if(ObjectString.empty())
            return Result;

        try {
            Poco::JSON::Parser P;
            auto Object = P.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
            for (auto const &i : *Object) {
                Result.push_back(i);
            }
        } catch (...) {

        }
        return Result;
    }

    inline Types::StringPairVec to_stringpair_array(const std::string &S) {
        Types::StringPairVec   R;
        if(S.empty())
            return R;
        try {
            Poco::JSON::Parser P;
            auto Object = P.parse(S).template extract<Poco::JSON::Array::Ptr>();
            for (const auto &i : *Object) {
                auto InnerObject = i.template extract<Poco::JSON::Array::Ptr>();
                if(InnerObject->size()==2) {
                    auto S1 = InnerObject->getElement<std::string>(0);
                    auto S2 = InnerObject->getElement<std::string>(1);
                    R.push_back(std::make_pair(S1,S2));
                }
            }
        } catch (...) {

        }

        return R;
    }

    template<class T> std::vector<T> to_object_array(const std::string & ObjectString) {
        std::vector<T>	Result;
        if(ObjectString.empty())
            return Result;

        try {
            Poco::JSON::Parser P;
            auto Object = P.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
            for (auto const i : *Object) {
                auto InnerObject = i.template extract<Poco::JSON::Object::Ptr>();
                T Obj;
                Obj.from_json(InnerObject);
                Result.push_back(Obj);
            }
        } catch (...) {

        }
        return Result;
    }

    template<class T> std::vector<std::vector<T>> to_array_of_array_of_object(const std::string & ObjectString) {
        std::vector<std::vector<T>>	Result;
        if(ObjectString.empty())
            return Result;
        try {
            Poco::JSON::Parser P1;
            auto OutterArray = P1.parse(ObjectString).template extract<Poco::JSON::Array::Ptr>();
            for (auto const &i : *OutterArray) {
                Poco::JSON::Parser P2;
                auto InnerArray = P2.parse(i).template extract<Poco::JSON::Array::Ptr>();
                std::vector<T>  InnerVector;
                for(auto const &j: *InnerArray) {
                    auto Object = j.template extract<Poco::JSON::Object::Ptr>();
                    T Obj;
                    Obj.from_json(Object);
                    InnerVector.push_back(Obj);
                }
                Result.push_back(InnerVector);
            }
        } catch (...) {

        }
        return Result;
    }

    template<class T> T to_object(const std::string & ObjectString) {
        T	Result;

        if(ObjectString.empty())
            return Result;

        Poco::JSON::Parser	P;
        auto Object = P.parse(ObjectString).template extract<Poco::JSON::Object::Ptr>();
        Result.from_json(Object);

        return Result;
    }

    template<class T> bool from_request(T & Obj, Poco::Net::HTTPServerRequest &Request) {
        Poco::JSON::Parser IncomingParser;
        auto RawObject = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Obj.from_json(RawObject);
        return true;
    }
}

namespace OpenWifi::Utils {

    enum MediaTypeEncodings {
        PLAIN,
        BINARY,
        BASE64
    };
    struct MediaTypeEncoding {
        MediaTypeEncodings  Encoding=PLAIN;
        std::string         ContentType;
    };

    [[nodiscard]] inline bool ValidSerialNumber(const std::string &Serial) {
        return ((Serial.size() < uCentralProtocol::SERIAL_NUMBER_LENGTH) &&
        std::all_of(Serial.begin(),Serial.end(),[](auto i){return std::isxdigit(i);}));
    }

    [[nodiscard]] inline std::vector<std::string> Split(const std::string &List, char Delimiter=',' ) {
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


    [[nodiscard]] inline std::string FormatIPv6(const std::string & I )
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

    inline void padTo(std::string& str, size_t num, char paddingChar = '\0') {
        str.append(num - str.length() % num, paddingChar);
    }

    [[nodiscard]] inline std::string SerialToMAC(const std::string &Serial) {
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

    [[nodiscard]] inline std::string ToHex(const std::vector<unsigned char> & B) {
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

    [[nodiscard]] inline std::string base64encode(const byte *input, unsigned long size) {
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

    [[nodiscard]] inline std::vector<byte> base64decode(const std::string& input)
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

    inline bool ParseTime(const std::string &Time, int & Hours, int & Minutes, int & Seconds) {
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


    inline bool ParseDate(const std::string &Time, int & Year, int & Month, int & Day) {
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

    inline bool CompareTime( int H1, int H2, int M1, int M2, int S1, int S2) {
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

    [[nodiscard]] inline std::string LogLevelToString(int Level) {
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

    [[nodiscard]] inline uint64_t SerialNumberToInt(const std::string & S) {
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

    [[nodiscard]] inline std::string IntToSerialNumber(uint64_t S) {
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


    [[nodiscard]] inline bool SerialNumberMatch(const std::string &S1, const std::string &S2, int Bits=2) {
        auto S1_i = SerialNumberToInt(S1);
        auto S2_i = SerialNumberToInt(S2);
        return ((S1_i>>Bits)==(S2_i>>Bits));
    }

    [[nodiscard]] inline uint64_t SerialNumberToOUI(const std::string & S) {
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

    [[nodiscard]] inline uint64_t GetDefaultMacAsInt64() {
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

    [[nodiscard]] inline uint64_t InitializeSystemId() {
        std::random_device	RDev;
        std::srand(RDev());
        std::chrono::high_resolution_clock	Clock;
        auto Now = Clock.now().time_since_epoch().count();
        auto S = (GetDefaultMacAsInt64() + std::rand() + Now)  ;
		OpenWifi::AppServiceRegistry().Set("systemid",S);
        return S;
    }

    [[nodiscard]] inline uint64_t GetSystemId();

    [[nodiscard]] inline bool ValidEMailAddress(const std::string &email) {
        // define a regular expression
        static const std::regex pattern
        ("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");

        // try to match the string with the regular expression
        return std::regex_match(email, pattern);
    }


    [[nodiscard]] inline std::string LoadFile( const Poco::File & F) {
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

    inline void ReplaceVariables( std::string & Content , const Types::StringPairVec & P) {
        for(const auto &[Variable,Value]:P) {
            Poco::replaceInPlace(Content,"${" + Variable + "}", Value);
        }
    }

    [[nodiscard]] inline MediaTypeEncoding FindMediaType(const Poco::File &F) {
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

    [[nodiscard]] inline std::string BinaryFileToHexString(const Poco::File &F) {
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

    [[nodiscard]] inline std::string SecondsToNiceText(uint64_t Seconds) {
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

    [[nodiscard]] inline bool wgets(const std::string &URL, std::string &Response) {
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

    template< typename T >
    std::string int_to_hex( T i )
    {
        std::stringstream stream;
        stream << std::setfill ('0') << std::setw(12)
        << std::hex << i;
        return stream.str();
    }

}

namespace OpenWifi {

	static const std::string uSERVICE_SECURITY{"owsec"};
	static const std::string uSERVICE_GATEWAY{"owgw"};
	static const std::string uSERVICE_FIRMWARE{ "owfms"};
    static const std::string uSERVICE_TOPOLOGY{ "owtopo"};
    static const std::string uSERVICE_PROVISIONING{ "owprov"};
    static const std::string uSERVICE_OWLS{ "owls"};
    static const std::string uSERVICE_SUBCRIBER{ "owsub"};
    static const std::string uSERVICE_INSTALLER{ "owinst"};


	class MyErrorHandler : public Poco::ErrorHandler {
	  public:
		explicit MyErrorHandler(Poco::Util::Application &App) : App_(App) {}
		inline void exception(const Poco::Exception & E) {
		    Poco::Thread * CurrentThread = Poco::Thread::current();
		    App_.logger().log(E);
		    App_.logger().error(Poco::format("Exception occurred in %s",CurrentThread->getName()));
		}

		inline void exception(const std::exception & E) {
		    Poco::Thread * CurrentThread = Poco::Thread::current();
		    App_.logger().warning(Poco::format("std::exception on %s",CurrentThread->getName()));
		}

		inline void exception() {
		    Poco::Thread * CurrentThread = Poco::Thread::current();
		    App_.logger().warning(Poco::format("exception on %s",CurrentThread->getName()));
		}
	  private:
		Poco::Util::Application	&App_;
	};

	class BusEventManager : public Poco::Runnable {
	  public:
		inline void run() final;
		inline void Start();
		inline void Stop();
	  private:
		std::atomic_bool 	Running_ = false;
		Poco::Thread		Thread_;
	};

	class MyPrivateKeyPassphraseHandler : public Poco::Net::PrivateKeyPassphraseHandler {
	public:
	    explicit MyPrivateKeyPassphraseHandler(const std::string &Password, Poco::Logger & Logger):
	    PrivateKeyPassphraseHandler(true),
	    Logger_(Logger),
	    Password_(Password) {}
	    void onPrivateKeyRequested(const void * pSender,std::string & privateKey) {
	        Logger_.information("Returning key passphrase.");
	        privateKey = Password_;
	    };
	private:
	    std::string Password_;
	    Poco::Logger & Logger_;
	};

	class PropertiesFileServerEntry {
	public:
	    PropertiesFileServerEntry(std::string Address, uint32_t port, std::string Key_file,
                                  std::string Cert_file, std::string RootCa, std::string Issuer,
                                  std::string ClientCas, std::string Cas,
                                  std::string Key_file_password = "", std::string Name = "",
                                  Poco::Net::Context::VerificationMode M =
                                          Poco::Net::Context::VerificationMode::VERIFY_RELAXED,
                                          int backlog = 64)
                                          : address_(std::move(Address)), port_(port), key_file_(std::move(Key_file)),
                                          cert_file_(std::move(Cert_file)), root_ca_(std::move(RootCa)),
                                          issuer_cert_file_(std::move(Issuer)), client_cas_(std::move(ClientCas)),
                                          cas_(std::move(Cas)), key_file_password_(std::move(Key_file_password)),
                                          name_(std::move(Name)), level_(M), backlog_(backlog){};

	    [[nodiscard]] inline const std::string &Address() const { return address_; };
	    [[nodiscard]] inline uint32_t Port() const { return port_; };
	    [[nodiscard]] inline const std::string &KeyFile() const { return key_file_; };
	    [[nodiscard]] inline const std::string &CertFile() const { return cert_file_; };
	    [[nodiscard]] inline const std::string &RootCA() const { return root_ca_; };
	    [[nodiscard]] inline const std::string &KeyFilePassword() const { return key_file_password_; };
	    [[nodiscard]] inline const std::string &IssuerCertFile() const { return issuer_cert_file_; };
	    [[nodiscard]] inline const std::string &Name() const { return name_; };
	    [[nodiscard]] inline int Backlog() const { return backlog_; }

	    [[nodiscard]] inline Poco::Net::SecureServerSocket CreateSecureSocket(Poco::Logger &L) const {
	        Poco::Net::Context::Params P;

	        P.verificationMode = level_;
	        P.verificationDepth = 9;
	        P.loadDefaultCAs = root_ca_.empty();
	        P.cipherList = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
	        P.dhUse2048Bits = true;
	        P.caLocation = cas_;

	        auto Context = Poco::AutoPtr<Poco::Net::Context>(new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P));

	        if(!key_file_password_.empty()) {
	            auto PassphraseHandler = Poco::SharedPtr<MyPrivateKeyPassphraseHandler>( new MyPrivateKeyPassphraseHandler(key_file_password_,L));
	            Poco::Net::SSLManager::instance().initializeServer(PassphraseHandler, nullptr,Context);
	        }

	        if (!cert_file_.empty() && !key_file_.empty()) {
	            Poco::Crypto::X509Certificate Cert(cert_file_);
	            Poco::Crypto::X509Certificate Root(root_ca_);

	            Context->useCertificate(Cert);
	            Context->addChainCertificate(Root);

	            Context->addCertificateAuthority(Root);

	            if (level_ == Poco::Net::Context::VERIFY_STRICT) {
	                if (issuer_cert_file_.empty()) {
	                    L.fatal("In strict mode, you must supply ans issuer certificate");
	                }
	                if (client_cas_.empty()) {
	                    L.fatal("In strict mode, client cas must be supplied");
	                }
	                Poco::Crypto::X509Certificate Issuing(issuer_cert_file_);
	                Context->addChainCertificate(Issuing);
	                Context->addCertificateAuthority(Issuing);
	            }

	            Poco::Crypto::RSAKey Key("", key_file_, key_file_password_);
	            Context->usePrivateKey(Key);

	            SSL_CTX *SSLCtx = Context->sslContext();
	            if (!SSL_CTX_check_private_key(SSLCtx)) {
	                L.fatal(Poco::format("Wrong Certificate(%s) for Key(%s)", cert_file_, key_file_));
	            }

	            SSL_CTX_set_verify(SSLCtx, SSL_VERIFY_PEER, nullptr);

	            if (level_ == Poco::Net::Context::VERIFY_STRICT) {
	                SSL_CTX_set_client_CA_list(SSLCtx, SSL_load_client_CA_file(client_cas_.c_str()));
	            }
	            SSL_CTX_enable_ct(SSLCtx, SSL_CT_VALIDATION_STRICT);
	            SSL_CTX_dane_enable(SSLCtx);

	            Context->enableSessionCache();
	            Context->setSessionCacheSize(0);
	            Context->setSessionTimeout(10);
	            Context->enableExtendedCertificateVerification(true);
	            Context->disableStatelessSessionResumption();
	        }

	        if (address_ == "*") {
	            Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
	                    Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
	                    : Poco::Net::AddressFamily::IPv4));
	            Poco::Net::SocketAddress SockAddr(Addr, port_);

	            return Poco::Net::SecureServerSocket(SockAddr, backlog_, Context);
	        } else {
	            Poco::Net::IPAddress Addr(address_);
	            Poco::Net::SocketAddress SockAddr(Addr, port_);

	            return Poco::Net::SecureServerSocket(SockAddr, backlog_, Context);
	        }
	    }

	    inline void LogCertInfo(Poco::Logger &L, const Poco::Crypto::X509Certificate &C) const {
	        L.information("=============================================================================================");
	        L.information(Poco::format(">          Issuer: %s", C.issuerName()));
	        L.information("---------------------------------------------------------------------------------------------");
	        L.information(Poco::format(">     Common Name: %s",
                                       C.issuerName(Poco::Crypto::X509Certificate::NID_COMMON_NAME)));
	        L.information(Poco::format(">         Country: %s",
                                       C.issuerName(Poco::Crypto::X509Certificate::NID_COUNTRY)));
	        L.information(Poco::format(">        Locality: %s",
                                       C.issuerName(Poco::Crypto::X509Certificate::NID_LOCALITY_NAME)));
	        L.information(Poco::format(">      State/Prov: %s",
                                       C.issuerName(Poco::Crypto::X509Certificate::NID_STATE_OR_PROVINCE)));
	        L.information(Poco::format(">        Org name: %s",
                                       C.issuerName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_NAME)));
	        L.information(
	                Poco::format(">        Org unit: %s",
                                 C.issuerName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_UNIT_NAME)));
	        L.information(
	                Poco::format(">           Email: %s",
                                 C.issuerName(Poco::Crypto::X509Certificate::NID_PKCS9_EMAIL_ADDRESS)));
	        L.information(Poco::format(">         Serial#: %s",
                                       C.issuerName(Poco::Crypto::X509Certificate::NID_SERIAL_NUMBER)));
	        L.information("---------------------------------------------------------------------------------------------");
	        L.information(Poco::format(">         Subject: %s", C.subjectName()));
	        L.information("---------------------------------------------------------------------------------------------");
	        L.information(Poco::format(">     Common Name: %s",
                                       C.subjectName(Poco::Crypto::X509Certificate::NID_COMMON_NAME)));
	        L.information(Poco::format(">         Country: %s",
                                       C.subjectName(Poco::Crypto::X509Certificate::NID_COUNTRY)));
	        L.information(Poco::format(">        Locality: %s",
                                       C.subjectName(Poco::Crypto::X509Certificate::NID_LOCALITY_NAME)));
	        L.information(
	                Poco::format(">      State/Prov: %s",
                                 C.subjectName(Poco::Crypto::X509Certificate::NID_STATE_OR_PROVINCE)));
	        L.information(
	                Poco::format(">        Org name: %s",
                                 C.subjectName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_NAME)));
	        L.information(
	                Poco::format(">        Org unit: %s",
                                 C.subjectName(Poco::Crypto::X509Certificate::NID_ORGANIZATION_UNIT_NAME)));
	        L.information(
	                Poco::format(">           Email: %s",
                                 C.subjectName(Poco::Crypto::X509Certificate::NID_PKCS9_EMAIL_ADDRESS)));
	        L.information(Poco::format(">         Serial#: %s",
                                       C.subjectName(Poco::Crypto::X509Certificate::NID_SERIAL_NUMBER)));
	        L.information("---------------------------------------------------------------------------------------------");
	        L.information(Poco::format(">  Signature Algo: %s", C.signatureAlgorithm()));
	        auto From = Poco::DateTimeFormatter::format(C.validFrom(), Poco::DateTimeFormat::HTTP_FORMAT);
	        L.information(Poco::format(">      Valid from: %s", From));
	        auto Expires =
	                Poco::DateTimeFormatter::format(C.expiresOn(), Poco::DateTimeFormat::HTTP_FORMAT);
	        L.information(Poco::format(">      Expires on: %s", Expires));
	        L.information(Poco::format(">         Version: %d", (int)C.version()));
	        L.information(Poco::format(">        Serial #: %s", C.serialNumber()));
	        L.information("=============================================================================================");
	    }

	    inline void LogCert(Poco::Logger &L) const {
	        try {
	            Poco::Crypto::X509Certificate C(cert_file_);
	            L.information("=============================================================================================");
	            L.information("=============================================================================================");
	            L.information(Poco::format("Certificate Filename: %s", cert_file_));
	            LogCertInfo(L, C);
	            L.information("=============================================================================================");

	            if (!issuer_cert_file_.empty()) {
	                Poco::Crypto::X509Certificate C1(issuer_cert_file_);
	                L.information("=============================================================================================");
	                L.information("=============================================================================================");
	                L.information(Poco::format("Issues Certificate Filename: %s", issuer_cert_file_));
	                LogCertInfo(L, C1);
	                L.information("=============================================================================================");
	            }

	            if (!client_cas_.empty()) {
	                std::vector<Poco::Crypto::X509Certificate> Certs =
	                        Poco::Net::X509Certificate::readPEM(client_cas_);

	                L.information("=============================================================================================");
	                L.information("=============================================================================================");
	                L.information(Poco::format("Client CAs Filename: %s", client_cas_));
	                L.information("=============================================================================================");
	                auto i = 1;
	                for (const auto &C3 : Certs) {
	                    L.information(Poco::format(" Index: %d", i));
	                    L.information("=============================================================================================");
	                    LogCertInfo(L, C3);
	                    i++;
	                }
	                L.information("=============================================================================================");
	            }

	        } catch (const Poco::Exception &E) {
	            L.log(E);
	        }
	    }

	    inline void LogCas(Poco::Logger &L) const {
	        try {
	            std::vector<Poco::Crypto::X509Certificate> Certs =
	                    Poco::Net::X509Certificate::readPEM(root_ca_);

	            L.information("=============================================================================================");
	            L.information("=============================================================================================");
	            L.information(Poco::format("CA Filename: %s", root_ca_));
	            L.information("=============================================================================================");
	            auto i = 1;
	            for (const auto &C : Certs) {
	                L.information(Poco::format(" Index: %d", i));
	                L.information("=============================================================================================");
	                LogCertInfo(L, C);
	                i++;
	            }
	            L.information("=============================================================================================");
	        } catch (const Poco::Exception &E) {
	            L.log(E);
	        }
	    }

	private:
	    std::string address_;
	    std::string cert_file_;
	    std::string key_file_;
	    std::string root_ca_;
	    std::string key_file_password_;
	    std::string issuer_cert_file_;
	    std::string client_cas_;
	    std::string cas_;
	    uint32_t port_;
	    std::string name_;
	    int backlog_;
	    Poco::Net::Context::VerificationMode level_;
	};

	class SubSystemServer : public Poco::Util::Application::Subsystem {
	public:
	    SubSystemServer(std::string Name, const std::string &LoggingPrefix,
                        std::string SubSystemConfigPrefix)
                        : Name_(std::move(Name)), Logger_(Poco::Logger::get(LoggingPrefix)),
                        SubSystemConfigPrefix_(std::move(SubSystemConfigPrefix)) {
	        Logger_.setLevel(Poco::Message::PRIO_NOTICE);
	    }

	    inline void initialize(Poco::Util::Application &self) override;
	    inline void uninitialize() override {
	    }
	    inline void reinitialize(Poco::Util::Application &self) override {
	        Logger_.information("Reloading of this subsystem is not supported.");
	    }
	    inline void defineOptions(Poco::Util::OptionSet &options) override {
	    }
	    inline const std::string & Name() const { return Name_; };
	    inline const char * name() const override { return Name_.c_str(); }

	    inline const PropertiesFileServerEntry & Host(uint64_t index) { return ConfigServersList_[index]; };
	    inline uint64_t HostSize() const { return ConfigServersList_.size(); }
	    inline Poco::Logger &Logger() { return Logger_; };
	    inline void SetLoggingLevel(Poco::Message::Priority NewPriority) { Logger_.setLevel(NewPriority); }
	    inline int GetLoggingLevel() { return Logger_.getLevel(); }

	    virtual int Start() = 0;
	    virtual void Stop() = 0;

	protected:
	    std::recursive_mutex Mutex_;
	    Poco::Logger 		&Logger_;
	    std::string 		Name_;
	    std::vector<PropertiesFileServerEntry> ConfigServersList_;
	    std::string 		SubSystemConfigPrefix_;
	};

	class RESTAPI_GenericServer {
	public:

	    enum {
	        LOG_GET=0,
	        LOG_DELETE,
	        LOG_PUT,
	        LOG_POST
	    };

	    void inline SetFlags(bool External, const std::string &Methods) {
	        Poco::StringTokenizer   Tokens(Methods,",");
	        auto Offset = (External ? 0 : 4);
	        for(const auto &i:Tokens) {
	            if(Poco::icompare(i,Poco::Net::HTTPRequest::HTTP_DELETE)==0)
	                LogFlags_[Offset+LOG_DELETE]=true;
	            else if(Poco::icompare(i,Poco::Net::HTTPRequest::HTTP_PUT)==0)
	                LogFlags_[Offset+LOG_PUT]=true;
	            else if(Poco::icompare(i,Poco::Net::HTTPRequest::HTTP_POST)==0)
	                LogFlags_[Offset+LOG_POST]=true;
	            else if(Poco::icompare(i,Poco::Net::HTTPRequest::HTTP_GET)==0)
	                LogFlags_[Offset+LOG_GET]=true;
	        }
	    }

	    inline void InitLogging();

	    [[nodiscard]] inline bool LogIt(const std::string &Method, bool External) const {
	        auto Offset = (External ? 0 : 4);
	        if(Method == Poco::Net::HTTPRequest::HTTP_GET)
	            return LogFlags_[Offset+LOG_GET];
	        if(Method == Poco::Net::HTTPRequest::HTTP_POST)
	            return LogFlags_[Offset+LOG_POST];
	        if(Method == Poco::Net::HTTPRequest::HTTP_PUT)
	            return LogFlags_[Offset+LOG_PUT];
	        if(Method == Poco::Net::HTTPRequest::HTTP_DELETE)
	            return LogFlags_[Offset+LOG_DELETE];
	        return false;
	    };

	    [[nodiscard]] inline bool LogBadTokens(bool External) const {
	        return LogBadTokens_[ (External ? 0 : 1) ];
	    };

	private:
	    std::array<bool,8>       LogFlags_{false};
	    std::array<bool,2>       LogBadTokens_{false};
	};

	class RESTAPI_PartHandler: public Poco::Net::PartHandler
	        {
	        public:
	            RESTAPI_PartHandler():
	            _length(0)
	            {
	            }

	            inline void handlePart(const Poco::Net::MessageHeader& header, std::istream& stream) override
	            {
	                _type = header.get("Content-Type", "(unspecified)");
	                if (header.has("Content-Disposition"))
	                {
	                    std::string disp;
	                    Poco::Net::NameValueCollection params;
	                    Poco::Net::MessageHeader::splitParameters(header["Content-Disposition"], disp, params);
	                    _name = params.get("name", "(unnamed)");
	                    _fileName = params.get("filename", "(unnamed)");
	                }

	                Poco::CountingInputStream istr(stream);
	                Poco::NullOutputStream ostr;
	                Poco::StreamCopier::copyStream(istr, ostr);
	                _length = (int)istr.chars();
	            }

	            [[nodiscard]] inline int length() const
	            {
	                return _length;
	            }

	            [[nodiscard]] inline const std::string& name() const
	            {
	                return _name;
	            }

	            [[nodiscard]] inline const std::string& fileName() const
	            {
	                return _fileName;
	            }

	            [[nodiscard]] inline const std::string& contentType() const
	            {
	                return _type;
	            }

	        private:
	            int _length;
	            std::string _type;
	            std::string _name;
	            std::string _fileName;
    };

	class RESTAPI_RateLimiter : public SubSystemServer {
	public:

	    struct ClientCacheEntry {
	        int64_t  Start=0;
	        int      Count=0;
	    };

	    static RESTAPI_RateLimiter *instance() {
	        static RESTAPI_RateLimiter * instance_ = new RESTAPI_RateLimiter;
	        return instance_;
	    }

	    inline int Start() final { return 0;};
	    inline void Stop() final { };

	    inline bool IsRateLimited(const Poco::Net::HTTPServerRequest &R, int64_t Period, int64_t MaxCalls) {
	        Poco::URI   uri(R.getURI());
	        auto H = str_hash(uri.getPath() + R.clientAddress().host().toString());
	        auto E = Cache_.get(H);
	        auto Now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	        if(E.isNull()) {
	            Cache_.add(H,ClientCacheEntry{.Start=Now, .Count=1});
	            return false;
	        }
	        if((Now-E->Start)<Period) {
	            E->Count++;
	            Cache_.update(H,E);
	            if(E->Count > MaxCalls) {
	                Logger_.warning(Poco::format("RATE-LIMIT-EXCEEDED: from '%s'", R.clientAddress().toString()));
	                return true;
	            }
	            return false;
	        }
	        E->Start = Now;
	        E->Count = 1;
	        Cache_.update(H,E);
	        return false;
	    }

	    inline void Clear() {
	        Cache_.clear();
	    }

	private:
	    Poco::ExpireLRUCache<uint64_t,ClientCacheEntry>      Cache_{2048};
	    std::hash<std::string>          str_hash;

	    RESTAPI_RateLimiter() noexcept:
	    SubSystemServer("RateLimiter", "RATE-LIMITER", "rate.limiter")
	    {
	    }

	};

	inline RESTAPI_RateLimiter * RESTAPI_RateLimiter() { return RESTAPI_RateLimiter::instance(); }

	class RESTAPIHandler : public Poco::Net::HTTPRequestHandler {
	public:
	    struct QueryBlock {
	        uint64_t StartDate = 0 , EndDate = 0 , Offset = 0 , Limit = 0, LogType = 0 ;
	        std::string SerialNumber, Filter, Select;
	        bool Lifetime=false, LastOnly=false, Newest=false, CountOnly=false, AdditionalInfo=false;
	    };
	    typedef std::map<std::string, std::string> BindingMap;

	    struct RateLimit {
	        int64_t     Interval=1000;
	        int64_t     MaxCalls=10;
	    };

	    RESTAPIHandler( BindingMap map,
                        Poco::Logger &l,
                        std::vector<std::string> Methods,
                        RESTAPI_GenericServer & Server,
                        bool Internal=false,
                        bool AlwaysAuthorize=true,
                        bool RateLimited=false,
	                    const RateLimit & Profile = RateLimit{.Interval=1000,.MaxCalls=100})
	    :   Bindings_(std::move(map)),
	        Logger_(l),
	        Methods_(std::move(Methods)),
	        Server_(Server),
	        Internal_(Internal),
	        AlwaysAuthorize_(AlwaysAuthorize),
	        RateLimited_(RateLimited),
	        MyRates_(Profile){
	    }

	    inline bool RoleIsAuthorized(const std::string & Path, const std::string & Method, std::string & Reason) {
	        return true;
	    }

	    inline void handleRequest(Poco::Net::HTTPServerRequest &RequestIn,
                                  Poco::Net::HTTPServerResponse &ResponseIn) final {
	        try {
	            Request = &RequestIn;
	            Response = &ResponseIn;

	            if(RateLimited_ && RESTAPI_RateLimiter()->IsRateLimited(RequestIn,MyRates_.Interval, MyRates_.MaxCalls)) {
	                return UnAuthorized("Rate limit exceeded.",RATE_LIMIT_EXCEEDED);
	            }

	            if (!ContinueProcessing())
	                return;

	            bool Expired=false;
	            if (AlwaysAuthorize_ && !IsAuthorized(Expired)) {
	                if(Expired)
	                    return UnAuthorized(RESTAPI::Errors::ExpiredToken, EXPIRED_TOKEN);
	                return UnAuthorized(RESTAPI::Errors::InvalidCredentials, ACCESS_DENIED);
	            }

	            std::string Reason;
	            if(!RoleIsAuthorized(RequestIn.getURI(), Request->getMethod(), Reason)) {
                    return UnAuthorized(Reason, ACCESS_DENIED);
	            }

	            ParseParameters();
	            if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
	                DoGet();
	            else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
	                DoPost();
	            else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
	                DoDelete();
	            else if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
	                DoPut();
	            else
	                BadRequest(RESTAPI::Errors::UnsupportedHTTPMethod);
	            return;
	        } catch (const Poco::Exception &E) {
	            Logger_.log(E);
	            BadRequest(RESTAPI::Errors::InternalError);
	        }
	    }

	    inline bool NeedAdditionalInfo() const { return QB_.AdditionalInfo; }
	    inline const std::string & SelectedRecords() const { return QB_.Select; }

	    inline const Poco::JSON::Object::Ptr & ParseStream() {
	        return IncomingParser_.parse(Request->stream()).extract<Poco::JSON::Object::Ptr>();
	    }


	    inline static bool ParseBindings(const std::string & Request, const std::list<const char *> & EndPoints, BindingMap &bindings) {
	        bindings.clear();
	        std::vector<std::string> PathItems = Utils::Split(Request, '/');

	        for(const auto &EndPoint:EndPoints) {
	            std::vector<std::string> ParamItems = Utils::Split(EndPoint, '/');
	            if (PathItems.size() != ParamItems.size())
	                continue;

	            bool Matched = true;
	            for (auto i = 0; i != PathItems.size() && Matched; i++) {
	                if (PathItems[i] != ParamItems[i]) {
	                    if (ParamItems[i][0] == '{') {
	                        auto ParamName = ParamItems[i].substr(1, ParamItems[i].size() - 2);
	                        bindings[Poco::toLower(ParamName)] = PathItems[i];
	                    } else {
	                        Matched = false;
	                    }
	                }
	            }
	            if(Matched)
	                return true;
	        }
	        return false;
	    }

	    inline void PrintBindings() {
	        for (const auto &[key, value] : Bindings_)
	            std::cout << "Key = " << key << "  Value= " << value << std::endl;
	    }

	    inline void ParseParameters() {
	        Poco::URI uri(Request->getURI());
	        Parameters_ = uri.getQueryParameters();
	        InitQueryBlock();
	    }

	    inline static bool is_number(const std::string &s) {
	        return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
	    }

	    inline static bool is_bool(const std::string &s) {
	        if (s == "true" || s == "false")
	            return true;
	        return false;
	    }

	    inline uint64_t GetParameter(const std::string &Name, const uint64_t Default) {
	        auto Hint = std::find_if(Parameters_.begin(),Parameters_.end(),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
	        if(Hint==Parameters_.end() || !is_number(Hint->second))
	            return Default;
	        return std::stoull(Hint->second);
	    }

	    inline bool GetBoolParameter(const std::string &Name, bool Default) {
	        auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
	        if(Hint==end(Parameters_) || !is_bool(Hint->second))
	            return Default;
	        return Hint->second=="true";
	    }

	    [[nodiscard]] inline std::string GetParameter(const std::string &Name, const std::string &Default) {
	        auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
	        if(Hint==end(Parameters_))
	            return Default;
	        return Hint->second;
	    }

	    [[nodiscard]] inline bool HasParameter(const std::string &Name, std::string &Value) {
	        auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
	        if(Hint==end(Parameters_))
	            return false;
	        Value = Hint->second;
	        return true;
	    }

	    [[nodiscard]] inline bool HasParameter(const std::string &Name, uint64_t & Value) {
	        auto Hint = std::find_if(begin(Parameters_),end(Parameters_),[Name](const std::pair<std::string,std::string> &S){ return S.first==Name; });
	        if(Hint==end(Parameters_))
	            return false;
	        Value = std::stoull(Hint->second);
	        return true;
	    }

	    [[nodiscard]] inline const std::string & GetBinding(const std::string &Name, const std::string &Default) {
	        auto E = Bindings_.find(Poco::toLower(Name));
	        if (E == Bindings_.end())
	            return Default;

	        return E->second;
	    }

	    inline static std::string MakeList(const std::vector<std::string> &L) {
	        std::string Return;
	        for (const auto &i : L)
	            if (Return.empty())
	                Return = i;
	            else
	                Return += ", " + i;

	            return Return;
	    }

	    inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, std::string &Value) {
	        if(O->has(Field)) {
	            Value = O->get(Field).toString();
	            return true;
	        }
	        return false;
	    }

	    inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, uint64_t &Value) {
	        if(O->has(Field)) {
	            Value = O->get(Field);
	            return true;
	        }
	        return false;
	    }

	    inline bool AssignIfPresent(const Poco::JSON::Object::Ptr &O, const std::string &Field, bool &Value) {
	        if(O->has(Field)) {
	            Value = O->get(Field).toString()=="true";
	            return true;
	        }
	        return false;
	    }

	    inline void AddCORS() {
	        auto Origin = Request->find("Origin");
	        if (Origin != Request->end()) {
	            Response->set("Access-Control-Allow-Origin", Origin->second);
	            Response->set("Vary", "Origin");
	        } else {
	            Response->set("Access-Control-Allow-Origin", "*");
	        }
	        Response->set("Access-Control-Allow-Headers", "*");
	        Response->set("Access-Control-Allow-Methods", MakeList(Methods_));
	        Response->set("Access-Control-Max-Age", "86400");
	    }

	    inline void SetCommonHeaders(bool CloseConnection=false) {
	        Response->setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
	        Response->setChunkedTransferEncoding(true);
	        Response->setContentType("application/json");
	        if(CloseConnection) {
	            Response->set("Connection", "close");
	            Response->setKeepAlive(false);
	        } else {
	            Response->setKeepAlive(true);
	            Response->set("Connection", "Keep-Alive");
	            Response->set("Keep-Alive", "timeout=5, max=1000");
	        }
	    }

	    inline void ProcessOptions() {
	        AddCORS();
	        SetCommonHeaders();
	        Response->setContentLength(0);
	        Response->set("Access-Control-Allow-Credentials", "true");
	        Response->setStatus(Poco::Net::HTTPResponse::HTTP_OK);
	        Response->set("Vary", "Origin, Access-Control-Request-Headers, Access-Control-Request-Method");
	        Response->send();
	    }

	    inline void PrepareResponse(Poco::Net::HTTPResponse::HTTPStatus Status = Poco::Net::HTTPResponse::HTTP_OK,
                                    bool CloseConnection = false) {
	        Response->setStatus(Status);
	        AddCORS();
	        SetCommonHeaders(CloseConnection);
	    }

	    inline void BadRequest(const std::string & Reason) {
	        PrepareResponse(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
	        Poco::JSON::Object	ErrorObject;
	        ErrorObject.set("ErrorCode",400);
	        ErrorObject.set("ErrorDetails",Request->getMethod());
	        ErrorObject.set("ErrorDescription",Reason.empty() ? "Command is missing parameters or wrong values." : Reason) ;
	        std::ostream &Answer = Response->send();
	        Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
	    }

	    inline void InternalError(const std::string & Reason = "") {
	        PrepareResponse(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
	        Poco::JSON::Object	ErrorObject;
	        ErrorObject.set("ErrorCode",500);
	        ErrorObject.set("ErrorDetails",Request->getMethod());
	        ErrorObject.set("ErrorDescription",Reason.empty() ? "Please try later or review the data submitted." : Reason) ;
	        std::ostream &Answer = Response->send();
	        Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
	    }

	    inline void UnAuthorized(const std::string & Reason = "", int Code = INVALID_CREDENTIALS ) {
	        PrepareResponse(Poco::Net::HTTPResponse::HTTP_FORBIDDEN);
	        Poco::JSON::Object	ErrorObject;
	        ErrorObject.set("ErrorCode",Code);
	        ErrorObject.set("ErrorDetails",Request->getMethod());
	        ErrorObject.set("ErrorDescription",Reason.empty() ? "No access allowed." : Reason) ;
	        std::ostream &Answer = Response->send();
	        Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
	    }

	    inline void NotFound() {
	        PrepareResponse(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
	        Poco::JSON::Object	ErrorObject;
	        ErrorObject.set("ErrorCode",404);
	        ErrorObject.set("ErrorDetails",Request->getMethod());
	        ErrorObject.set("ErrorDescription","This resource does not exist.");
	        std::ostream &Answer = Response->send();
	        Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
	        Logger_.debug(Poco::format("RES-NOTFOUND: User='%s@%s' Method='%s' Path='%s",
                                       UserInfo_.userinfo.email,
                                       Utils::FormatIPv6(Request->clientAddress().toString()),
                                       Request->getMethod(),
                                       Request->getURI()));
	    }

	    inline void OK() {
	        PrepareResponse();
	        if(	Request->getMethod()==Poco::Net::HTTPRequest::HTTP_DELETE ||
	        Request->getMethod()==Poco::Net::HTTPRequest::HTTP_OPTIONS) {
	            Response->send();
	        } else {
	            Poco::JSON::Object ErrorObject;
	            ErrorObject.set("Code", 0);
	            ErrorObject.set("Operation", Request->getMethod());
	            ErrorObject.set("Details", "Command completed.");
	            std::ostream &Answer = Response->send();
	            Poco::JSON::Stringifier::stringify(ErrorObject, Answer);
	        }
	    }

	    inline void SendFile(Poco::File & File, const std::string & UUID) {
	        Response->set("Content-Type","application/octet-stream");
	        Response->set("Content-Disposition", "attachment; filename=" + UUID );
	        Response->set("Content-Transfer-Encoding","binary");
	        Response->set("Accept-Ranges", "bytes");
	        Response->set("Cache-Control", "private");
	        Response->set("Pragma", "private");
	        Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
	        Response->setContentLength(File.getSize());
	        AddCORS();
	        Response->sendFile(File.path(),"application/octet-stream");
	    }

	    inline void SendFile(Poco::File & File) {
	        Poco::Path  P(File.path());
	        auto MT = Utils::FindMediaType(File);
	        if(MT.Encoding==Utils::BINARY) {
	            Response->set("Content-Transfer-Encoding","binary");
	            Response->set("Accept-Ranges", "bytes");
	        }
	        Response->set("Cache-Control", "private");
	        Response->set("Pragma", "private");
	        Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
	        AddCORS();
	        Response->sendFile(File.path(),MT.ContentType);
	    }

	    inline void SendFile(Poco::TemporaryFile &TempAvatar, const std::string &Type, const std::string & Name) {
	        auto MT = Utils::FindMediaType(Name);
	        if(MT.Encoding==Utils::BINARY) {
	            Response->set("Content-Transfer-Encoding","binary");
	            Response->set("Accept-Ranges", "bytes");
	        }
	        Response->set("Content-Disposition", "attachment; filename=" + Name );
	        Response->set("Accept-Ranges", "bytes");
	        Response->set("Cache-Control", "private");
	        Response->set("Pragma", "private");
	        Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
	        AddCORS();
	        Response->sendFile(TempAvatar.path(),MT.ContentType);
	    }

	    inline void SendHTMLFileBack(Poco::File & File,
                                     const Types::StringPairVec & FormVars) {
	        Response->set("Pragma", "private");
	        Response->set("Expires", "Mon, 26 Jul 2027 05:00:00 GMT");
	        std::string FormContent = Utils::LoadFile(File.path());
	        Utils::ReplaceVariables(FormContent, FormVars);
	        Response->setContentLength(FormContent.size());
	        AddCORS();
	        Response->setChunkedTransferEncoding(true);
	        Response->setContentType("text/html");
	        std::ostream& ostr = Response->send();
	        ostr << FormContent;
	    }

	    inline void ReturnStatus(Poco::Net::HTTPResponse::HTTPStatus Status, bool CloseConnection=false) {
	        PrepareResponse(Status, CloseConnection);
	        if(Status == Poco::Net::HTTPResponse::HTTP_NO_CONTENT) {
	            Response->setContentLength(0);
	            Response->erase("Content-Type");
	            Response->setChunkedTransferEncoding(false);
	        }
	        Response->send();
	    }

	    inline bool ContinueProcessing() {
	        if (Request->getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
	            ProcessOptions();
	            return false;
	        } else if (std::find(Methods_.begin(), Methods_.end(), Request->getMethod()) == Methods_.end()) {
	            BadRequest(RESTAPI::Errors::UnsupportedHTTPMethod);
	            return false;
	        }

	        return true;
	    }

	    inline bool IsAuthorized(bool & Expired);

	        inline void ReturnObject(Poco::JSON::Object &Object) {
	            PrepareResponse();
	            std::ostream &Answer = Response->send();
	            Poco::JSON::Stringifier::stringify(Object, Answer);
	        }

	        inline void ReturnCountOnly(uint64_t Count) {
	            Poco::JSON::Object  Answer;
	            Answer.set("count", Count);
	            ReturnObject(Answer);
	        }

	        inline bool InitQueryBlock() {
	            if(QueryBlockInitialized_)
	                return true;
	            QueryBlockInitialized_=true;
	            QB_.SerialNumber = GetParameter(RESTAPI::Protocol::SERIALNUMBER, "");
	            QB_.StartDate = GetParameter(RESTAPI::Protocol::STARTDATE, 0);
	            QB_.EndDate = GetParameter(RESTAPI::Protocol::ENDDATE, 0);
	            QB_.Offset = GetParameter(RESTAPI::Protocol::OFFSET, 0);
	            QB_.Limit = GetParameter(RESTAPI::Protocol::LIMIT, 100);
	            QB_.Filter = GetParameter(RESTAPI::Protocol::FILTER, "");
	            QB_.Select = GetParameter(RESTAPI::Protocol::SELECT, "");
	            QB_.Lifetime = GetBoolParameter(RESTAPI::Protocol::LIFETIME,false);
	            QB_.LogType = GetParameter(RESTAPI::Protocol::LOGTYPE,0);
	            QB_.LastOnly = GetBoolParameter(RESTAPI::Protocol::LASTONLY,false);
	            QB_.Newest = GetBoolParameter(RESTAPI::Protocol::NEWEST,false);
	            QB_.CountOnly = GetBoolParameter(RESTAPI::Protocol::COUNTONLY,false);
	            QB_.AdditionalInfo = GetBoolParameter(RESTAPI::Protocol::WITHEXTENDEDINFO,false);

	            if(QB_.Offset<1)
	                QB_.Offset=0;
	            return true;
	        }

	        [[nodiscard]] inline uint64_t Get(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, uint64_t Default=0){
	            if(Obj->has(Parameter))
	                return Obj->get(Parameter);
	            return Default;
	        }

	        [[nodiscard]] inline std::string GetS(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, const std::string & Default=""){
	            if(Obj->has(Parameter))
	                return Obj->get(Parameter).toString();
	            return Default;
	        }

	        [[nodiscard]] inline bool GetB(const char *Parameter,const Poco::JSON::Object::Ptr &Obj, bool Default=false){
	            if(Obj->has(Parameter))
	                return Obj->get(Parameter).toString()=="true";
	            return Default;
	        }

	        [[nodiscard]] inline uint64_t GetWhen(const Poco::JSON::Object::Ptr &Obj) {
	            return RESTAPIHandler::Get(RESTAPI::Protocol::WHEN, Obj);
	        }

	        template<typename T> void ReturnObject(const char *Name, const std::vector<T> & Objects) {
	            Poco::JSON::Object  Answer;
	            RESTAPI_utils::field_to_json(Answer,Name,Objects);
	            ReturnObject(Answer);
	        }

	        Poco::Logger & Logger() { return Logger_; }

	        virtual void DoGet() = 0 ;
	        virtual void DoDelete() = 0 ;
	        virtual void DoPost() = 0 ;
	        virtual void DoPut() = 0 ;

	    protected:
	        BindingMap 					Bindings_;
	        Poco::URI::QueryParameters 	Parameters_;
	        Poco::Logger 				&Logger_;
	        std::string 				SessionToken_;
	        SecurityObjects::UserInfoAndPolicy 	UserInfo_;
	        std::vector<std::string> 	Methods_;
	        QueryBlock					QB_;
	        bool                        Internal_=false;
	        bool                        RateLimited_=false;
	        bool                        QueryBlockInitialized_=false;
	        Poco::Net::HTTPServerRequest    *Request= nullptr;
	        Poco::Net::HTTPServerResponse   *Response= nullptr;
	        bool                        AlwaysAuthorize_=true;
	        Poco::JSON::Parser          IncomingParser_;
	        RESTAPI_GenericServer       & Server_;
	        RateLimit                   MyRates_;
	    };

	    class RESTAPI_UnknownRequestHandler : public RESTAPIHandler {
	    public:
	        RESTAPI_UnknownRequestHandler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server)
	        : RESTAPIHandler(bindings, L, std::vector<std::string>{}, Server) {}
	        inline void DoGet() override {};
	        inline void DoPost() override {};
	        inline void DoPut() override {};
	        inline void DoDelete() override {};
	    };

	    template<class T>
	            constexpr auto test_has_PathName_method(T*)
	            -> decltype(  T::PathName() , std::true_type{} )
	            {
	                return std::true_type{};
	            }
	            constexpr auto test_has_PathName_method(...) -> std::false_type
	            {
	                return std::false_type{};
	            }

	            template<typename T, typename... Args>
	            RESTAPIHandler * RESTAPI_Router(const std::string & RequestedPath, RESTAPIHandler::BindingMap &Bindings, Poco::Logger & Logger, RESTAPI_GenericServer & Server) {
	                static_assert(test_has_PathName_method((T*)nullptr), "Class must have a static PathName() method.");
	                if(RESTAPIHandler::ParseBindings(RequestedPath,T::PathName(),Bindings)) {
	                    return new T(Bindings, Logger, Server, false);
	                }

	                if constexpr (sizeof...(Args) == 0) {
	                    return new RESTAPI_UnknownRequestHandler(Bindings,Logger, Server);
	                } else {
	                    return RESTAPI_Router<Args...>(RequestedPath, Bindings, Logger, Server);
	                }
	            }

	            template<typename T, typename... Args>
	            RESTAPIHandler * RESTAPI_Router_I(const std::string & RequestedPath, RESTAPIHandler::BindingMap &Bindings, Poco::Logger & Logger, RESTAPI_GenericServer & Server) {
	                static_assert(test_has_PathName_method((T*)nullptr), "Class must have a static PathName() method.");
	                if(RESTAPIHandler::ParseBindings(RequestedPath,T::PathName(),Bindings)) {
	                    return new T(Bindings, Logger, Server, true);
	                }

	                if constexpr (sizeof...(Args) == 0) {
	                    return new RESTAPI_UnknownRequestHandler(Bindings,Logger, Server);
	                } else {
	                    return RESTAPI_Router_I<Args...>(RequestedPath, Bindings, Logger, Server);
	                }
	            }

	class OpenAPIRequestGet {
	public:
	    explicit OpenAPIRequestGet( std::string Type,
                                    std::string EndPoint,
                                    Types::StringPairVec & QueryData,
                                    uint64_t msTimeout):
                                    Type_(std::move(Type)),
                                    EndPoint_(std::move(EndPoint)),
                                    QueryData_(QueryData),
                                    msTimeout_(msTimeout) {};
	    inline int Do(Poco::JSON::Object::Ptr &ResponseObject);
	private:
	    std::string 			Type_;
	    std::string 			EndPoint_;
	    Types::StringPairVec 	QueryData_;
	    uint64_t 				msTimeout_;
	};

	class OpenAPIRequestPut {
	public:
	    explicit OpenAPIRequestPut( std::string Type,
                                    std::string EndPoint,
                                    Types::StringPairVec & QueryData,
                                    Poco::JSON::Object Body,
                                    uint64_t msTimeout):
                                    Type_(std::move(Type)),
                                    EndPoint_(std::move(EndPoint)),
                                    QueryData_(QueryData),
                                    msTimeout_(msTimeout),
                                    Body_(std::move(Body)){};

	    inline int Do(Poco::JSON::Object::Ptr &ResponseObject);

	private:
	    std::string 			Type_;
	    std::string 			EndPoint_;
	    Types::StringPairVec 	QueryData_;
	    uint64_t 				msTimeout_;
	    Poco::JSON::Object      Body_;
	};

	class OpenAPIRequestPost {
	public:
	    explicit OpenAPIRequestPost( std::string Type,
                                     std::string EndPoint,
                                     Types::StringPairVec & QueryData,
                                     Poco::JSON::Object Body,
                                     uint64_t msTimeout):
                                     Type_(std::move(Type)),
                                     EndPoint_(std::move(EndPoint)),
                                     QueryData_(QueryData),
                                     msTimeout_(msTimeout),
                                     Body_(std::move(Body)){};
	    inline int Do(Poco::JSON::Object::Ptr &ResponseObject);
	private:
	    std::string 			Type_;
	    std::string 			EndPoint_;
	    Types::StringPairVec 	QueryData_;
	    uint64_t 				msTimeout_;
	    Poco::JSON::Object      Body_;
	};

    class KafkaProducer : public Poco::Runnable {
    public:
        inline void run();
        void Start() {
            if(!Running_) {
                Running_=true;
                Worker_.start(*this);
            }
        }
        void Stop() {
            if(Running_) {
                Running_=false;
                Worker_.wakeUp();
                Worker_.join();
            }
        }
    private:
        std::mutex          Mutex_;
        Poco::Thread        Worker_;
        std::atomic_bool    Running_=false;
    };

    class KafkaConsumer : public Poco::Runnable {
    public:
        inline void run();
        void Start() {
            if(!Running_) {
                Running_=true;
                Worker_.start(*this);
            }
        }
        void Stop() {
            if(Running_) {
                Running_=false;
                Worker_.wakeUp();
                Worker_.join();
            }
        }
    private:
        std::mutex          Mutex_;
        Poco::Thread        Worker_;
        std::atomic_bool    Running_=false;
    };

	class KafkaManager : public SubSystemServer {
	public:
	    struct KMessage {
	        std::string Topic,
	        Key,
	        PayLoad;
	    };

	    friend class KafkaConsumer;
	    friend class KafkaProducer;

	    inline void initialize(Poco::Util::Application & self) override;

	    static KafkaManager *instance() {
	        static KafkaManager * instance_ = new KafkaManager;
	        return instance_;
	    }

	    inline int Start() override {
	        if(!KafkaEnabled_)
	            return 0;
	        ConsumerThr_.Start();
	        ProducerThr_.Start();
	        return 0;
	    }

	    inline void Stop() override {
	        if(KafkaEnabled_) {
	            ProducerThr_.Stop();
	            ConsumerThr_.Stop();
	            return;
	        }
	    }

	    inline void PostMessage(const std::string &topic, const std::string & key, const std::string &PayLoad, bool WrapMessage = true  ) {
	        if(KafkaEnabled_) {
	            std::lock_guard G(Mutex_);
	            KMessage M{
	                .Topic = topic,
	                .Key = key,
	                .PayLoad = WrapMessage ? WrapSystemId(PayLoad) : PayLoad };
	            Queue_.push(M);
	        }
	    }

	    [[nodiscard]] inline std::string WrapSystemId(const std::string & PayLoad) {
	        return std::move( SystemInfoWrapper_ + PayLoad + "}");
	    }

	    [[nodiscard]] inline bool Enabled() const { return KafkaEnabled_; }

	    inline int RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction &F) {
	        if(KafkaEnabled_) {
	            std::lock_guard G(Mutex_);
	            auto It = Notifiers_.find(Topic);
	            if(It == Notifiers_.end()) {
	                Types::TopicNotifyFunctionList L;
	                L.emplace(L.end(),std::make_pair(F,FunctionId_));
	                Notifiers_[Topic] = std::move(L);
	            } else {
	                It->second.emplace(It->second.end(),std::make_pair(F,FunctionId_));
	            }
	            return FunctionId_++;
	        } else {
	            return 0;
	        }
	    }

	    inline void UnregisterTopicWatcher(const std::string &Topic, int Id) {
	        if(KafkaEnabled_) {
	            std::lock_guard G(Mutex_);
	            auto It = Notifiers_.find(Topic);
	            if(It != Notifiers_.end()) {
	                Types::TopicNotifyFunctionList & L = It->second;
	                for(auto it=L.begin(); it!=L.end(); it++)
	                    if(it->second == Id) {
	                        L.erase(it);
	                        break;
	                    }
	            }
	        }
	    }

	    // void WakeUp();

	private:
	    bool 							KafkaEnabled_ = false;
	    std::queue<KMessage>			Queue_;
	    std::string 					SystemInfoWrapper_;
	    int                       		FunctionId_=1;
	    Types::NotifyTable        		Notifiers_;
	    KafkaProducer                   ProducerThr_;
	    KafkaConsumer                   ConsumerThr_;

	    inline void PartitionAssignment(const cppkafka::TopicPartitionList& partitions) {
	        Logger_.information(Poco::format("Partition assigned: %Lu...",(uint64_t )partitions.front().get_partition()));
	    }
	    inline void PartitionRevocation(const cppkafka::TopicPartitionList& partitions) {
	        Logger_.information(Poco::format("Partition revocation: %Lu...",(uint64_t )partitions.front().get_partition()));
	    }

	    KafkaManager() noexcept:
	    SubSystemServer("KafkaManager", "KAFKA-SVR", "openwifi.kafka")
	    {
	    }
	};

	inline KafkaManager * KafkaManager() { return KafkaManager::instance(); }

	class AuthClient : public SubSystemServer {
	public:
	    explicit AuthClient() noexcept:
	    SubSystemServer("Authentication", "AUTH-CLNT", "authentication")
	    {
	    }

	    static AuthClient *instance() {
	        static AuthClient * instance_ = new AuthClient;
	        return instance_;
	    }

	    inline int Start() override {
	        return 0;
	    }

	    inline void Stop() override {
	        Cache_.clear();
	    }

	    inline void RemovedCachedToken(const std::string &Token) {
	        std::lock_guard	G(Mutex_);
	        Cache_.remove(Token);
	    }

	    inline static bool IsTokenExpired(const SecurityObjects::WebToken &T) {
	        return ((T.expires_in_+T.created_)<std::time(nullptr));
	    }

	    inline bool RetrieveTokenInformation(const std::string & SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired) {
	        try {
	            Types::StringPairVec QueryData;
	            QueryData.push_back(std::make_pair("token",SessionToken));
	            OpenAPIRequestGet	Req(    uSERVICE_SECURITY,
                                             "/api/v1/validateToken",
                                             QueryData,
                                             5000);
	            Poco::JSON::Object::Ptr Response;
	            if(Req.Do(Response)==Poco::Net::HTTPResponse::HTTP_OK) {
	                if(Response->has("tokenInfo") && Response->has("userInfo")) {
	                    UInfo.from_json(Response);
	                    if(IsTokenExpired(UInfo.webtoken)) {
	                        Expired = true;
	                        return false;
	                    }
	                    Expired = false;
	                    Cache_.update(SessionToken, UInfo);
	                    return true;
	                }
	            }
	        } catch (...) {

	        }
	        Expired = false;
	        return false;
	    }

        inline bool IsAuthorized(const std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired) {
	        auto User = Cache_.get(SessionToken);
	        if(!User.isNull()) {
	            if(IsTokenExpired(User->webtoken)) {
	                Expired = true;
	                return false;
	            }
	            Expired = false;
                UInfo = *User;
                return true;
	        }
	        return RetrieveTokenInformation(SessionToken, UInfo, Expired);
	    }

	private:
	    Poco::ExpireLRUCache<std::string,OpenWifi::SecurityObjects::UserInfoAndPolicy>      Cache_{1024,1200000 };
	};

	inline AuthClient * AuthClient() { return AuthClient::instance(); }

	class ALBRequestHandler: public Poco::Net::HTTPRequestHandler
	        /// Return a HTML document with the current date and time.
	        {
	        public:
	            explicit ALBRequestHandler(Poco::Logger & L)
	            : Logger_(L)
	            {
	            }

	            void handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
	            {
	                Logger_.information(Poco::format("ALB-REQUEST(%s): New ALB request.",Request.clientAddress().toString()));
	                Response.setChunkedTransferEncoding(true);
	                Response.setContentType("text/html");
	                Response.setDate(Poco::Timestamp());
	                Response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
	                Response.setKeepAlive(true);
	                Response.set("Connection","keep-alive");
	                Response.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
	                std::ostream &Answer = Response.send();
	                Answer << "uCentralGW Alive and kicking!" ;
	            }

	        private:
	            Poco::Logger 	& Logger_;
	        };

	class ALBRequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
	        {
	        public:
	            explicit ALBRequestHandlerFactory(Poco::Logger & L):
	            Logger_(L)
	            {
	            }

	            ALBRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override
	            {
	                if (request.getURI() == "/")
	                    return new ALBRequestHandler(Logger_);
	                else
	                    return nullptr;
	            }

	        private:
	            Poco::Logger	&Logger_;
	        };

	class ALBHealthCheckServer : public SubSystemServer {
	public:
	    ALBHealthCheckServer() noexcept:
	    SubSystemServer("ALBHealthCheckServer", "ALB-SVR", "alb")
	    {
	    }

	    static ALBHealthCheckServer *instance() {
	        static ALBHealthCheckServer * instance = new ALBHealthCheckServer;
	        return instance;
	    }

	    inline int Start() override;

	    inline void Stop() override {
	        if(Running_)
	            Server_->stop();
	    }

	private:
	    std::unique_ptr<Poco::Net::HTTPServer>   	Server_;
	    std::unique_ptr<Poco::Net::ServerSocket> 	Socket_;
	    int                                     	Port_ = 0;
	    std::atomic_bool                            Running_=false;
	};

	inline ALBHealthCheckServer * ALBHealthCheckServer() { return ALBHealthCheckServer::instance(); }

	Poco::Net::HTTPRequestHandler * RESTAPI_external_server(const char *Path, RESTAPIHandler::BindingMap &Bindings,
                                           Poco::Logger & L, RESTAPI_GenericServer & S);

	Poco::Net::HTTPRequestHandler * RESTAPI_internal_server(const char *Path, RESTAPIHandler::BindingMap &Bindings,
                                                            Poco::Logger & L, RESTAPI_GenericServer & S);


	class RESTAPI_server : public SubSystemServer {
	public:
	    static RESTAPI_server *instance() {
	        static RESTAPI_server *instance_ = new RESTAPI_server;
	        return instance_;
	    }
	    int Start() override;
	    inline void Stop() override {
	        Logger_.information("Stopping ");
	        for( const auto & svr : RESTServers_ )
	            svr->stop();
	        Pool_.joinAll();
	        RESTServers_.clear();
	    }

	    inline void reinitialize(Poco::Util::Application &self) override;

	    inline Poco::Net::HTTPRequestHandler *CallServer(const char *Path) {
	        RESTAPIHandler::BindingMap Bindings;
	        return RESTAPI_external_server(Path, Bindings, Logger_, Server_);
	    }

	private:
	    std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   RESTServers_;
	    Poco::ThreadPool	    Pool_;
	    RESTAPI_GenericServer   Server_;

	    RESTAPI_server() noexcept:
	    SubSystemServer("RESTAPIServer", "RESTAPIServer", "openwifi.restapi")
            {
            }
	};

	inline RESTAPI_server * RESTAPI_server() { return RESTAPI_server::instance(); };

	class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	public:
	    RequestHandlerFactory(RESTAPI_GenericServer & Server) :
	    Logger_(RESTAPI_server::instance()->Logger()),
	    Server_(Server)
	    {

	    }

	    inline Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &Request) override {
	        Poco::URI uri(Request.getURI());
	        auto *Path = uri.getPath().c_str();
	        return RESTAPI_server()->CallServer(Path);
	    }

	private:
	    Poco::Logger            &Logger_;
	    RESTAPI_GenericServer   &Server_;
	};

	inline int RESTAPI_server::Start() {
	    Logger_.information("Starting.");
	    Server_.InitLogging();

	    for(const auto & Svr: ConfigServersList_) {
	        Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.Address(), std::to_string(Svr.Port()),
                                             Svr.KeyFile(),Svr.CertFile()));

	        auto Sock{Svr.CreateSecureSocket(Logger_)};

	        Svr.LogCert(Logger_);
	        if(!Svr.RootCA().empty())
	            Svr.LogCas(Logger_);

	        Poco::Net::HTTPServerParams::Ptr Params = new Poco::Net::HTTPServerParams;
	        Params->setMaxThreads(50);
	        Params->setMaxQueued(200);
	        Params->setKeepAlive(true);

	        auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new RequestHandlerFactory(Server_), Pool_, Sock, Params);
	        NewServer->start();
	        RESTServers_.push_back(std::move(NewServer));
	    }

	    return 0;
	}

	class RESTAPI_InternalServer : public SubSystemServer {

	public:
	    static RESTAPI_InternalServer *instance() {
	        static RESTAPI_InternalServer *instance_ = new RESTAPI_InternalServer;
	        return instance_;
	    }

	    inline int Start() override;
	    inline void Stop() override {
	        Logger_.information("Stopping ");
	        for( const auto & svr : RESTServers_ )
	            svr->stop();
	        Pool_.stopAll();
	    }

	    inline void reinitialize(Poco::Util::Application &self) override;

	    inline Poco::Net::HTTPRequestHandler *CallServer(const char *Path) {
	        RESTAPIHandler::BindingMap Bindings;
	        return RESTAPI_internal_server(Path, Bindings, Logger_, Server_);
	    }
	private:
	    std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   RESTServers_;
	    Poco::ThreadPool	    Pool_;
	    RESTAPI_GenericServer   Server_;

	    RESTAPI_InternalServer() noexcept: SubSystemServer("RESTAPIInternalServer", "REST-ISRV", "openwifi.internal.restapi")
	    {
	    }
	};

	inline RESTAPI_InternalServer * RESTAPI_InternalServer() { return RESTAPI_InternalServer::instance(); };

	class InternalRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	public:
	    InternalRequestHandlerFactory(RESTAPI_GenericServer & Server) :
	    Logger_(RESTAPI_InternalServer()->Logger()),
	    Server_(Server){}

	    inline Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &Request) override {
	        Poco::URI uri(Request.getURI());
	        auto *Path = uri.getPath().c_str();
	        return RESTAPI_InternalServer()->CallServer(Path);
	    }
	private:
	    Poco::Logger    & Logger_;
	    RESTAPI_GenericServer   & Server_;
	};

	inline int RESTAPI_InternalServer::Start() {
	    Logger_.information("Starting.");
	    Server_.InitLogging();

	    for(const auto & Svr: ConfigServersList_) {
	        Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.Address(), std::to_string(Svr.Port()),
                                             Svr.KeyFile(),Svr.CertFile()));

	        auto Sock{Svr.CreateSecureSocket(Logger_)};

	        Svr.LogCert(Logger_);
	        if(!Svr.RootCA().empty())
	            Svr.LogCas(Logger_);
	        auto Params = new Poco::Net::HTTPServerParams;
	        Params->setMaxThreads(50);
	        Params->setMaxQueued(200);
	        Params->setKeepAlive(true);

	        auto NewServer = std::make_unique<Poco::Net::HTTPServer>(new InternalRequestHandlerFactory(Server_), Pool_, Sock, Params);
	        NewServer->start();
	        RESTServers_.push_back(std::move(NewServer));
	    }

	    return 0;
	}

	struct MicroServiceMeta {
		uint64_t 		Id=0;
		std::string 	Type;
		std::string 	PrivateEndPoint;
		std::string 	PublicEndPoint;
		std::string 	AccessKey;
		std::string		Version;
		uint64_t 		LastUpdate=0;
	};



	class SubSystemServer;
	typedef std::map<uint64_t, MicroServiceMeta>	MicroServiceMetaMap;
	typedef std::vector<MicroServiceMeta>			MicroServiceMetaVec;
	typedef std::vector<SubSystemServer *>          SubSystemVec;

	class MicroService : public Poco::Util::ServerApplication {
	  public:
		explicit MicroService( 	std::string PropFile,
					 	std::string RootEnv,
					 	std::string ConfigVar,
					 	std::string AppName,
					  	uint64_t BusTimer,
					  	SubSystemVec Subsystems) :
			DAEMON_PROPERTIES_FILENAME(std::move(PropFile)),
			DAEMON_ROOT_ENV_VAR(std::move(RootEnv)),
			DAEMON_CONFIG_ENV_VAR(std::move(ConfigVar)),
			DAEMON_APP_NAME(std::move(AppName)),
			DAEMON_BUS_TIMER(BusTimer),
			SubSystems_(std::move(Subsystems)) {
		    instance_ = this;
		    RandomEngine_.seed(std::chrono::steady_clock::now().time_since_epoch().count());
		}

		[[nodiscard]] std::string Version() { return Version_; }
		[[nodiscard]] const Poco::SharedPtr<Poco::Crypto::RSAKey> & Key() { return AppKey_; }
		[[nodiscard]] inline const std::string & DataDir() { return DataDir_; }
		[[nodiscard]] inline const std::string & WWWAssetsDir() { return WWWAssetsDir_; }
		[[nodiscard]] bool Debug() const { return DebugMode_; }
		[[nodiscard]] uint64_t ID() const { return ID_; }
		[[nodiscard]] std::string Hash() const { return MyHash_; };
		[[nodiscard]] std::string ServiceType() const { return DAEMON_APP_NAME; };
		[[nodiscard]] std::string PrivateEndPoint() const { return MyPrivateEndPoint_; };
		[[nodiscard]] std::string PublicEndPoint() const { return MyPublicEndPoint_; };
		[[nodiscard]] const SubSystemVec & GetFullSubSystems() { return SubSystems_; }
		inline uint64_t DaemonBusTimer() const { return DAEMON_BUS_TIMER; };
        [[nodiscard]] const std::string & AppName() { return DAEMON_APP_NAME; }
		static inline uint64_t GetPID() { return Poco::Process::id(); };
		[[nodiscard]] inline const std::string GetPublicAPIEndPoint() { return MyPublicEndPoint_ + "/api/v1"; };
		[[nodiscard]] inline const std::string & GetUIURI() const { return UIURI_;};
		[[nodiscard]] inline uint64_t Random(uint64_t ceiling) {
		    return (RandomEngine_() % ceiling);
		}

		[[nodiscard]] inline uint64_t Random(uint64_t min, uint64_t max) {
		    return ((RandomEngine_() % (max-min)) + min);
		}

		inline void Exit(int Reason);
		inline void BusMessageReceived(const std::string &Key, const std::string & Message);
		inline MicroServiceMetaVec GetServices(const std::string & Type);
		inline MicroServiceMetaVec GetServices();
		inline void LoadConfigurationFile();
		inline void Reload();
		inline void LoadMyConfig();
		inline void initialize(Poco::Util::Application &self) override;
		inline void uninitialize() override;
		inline void reinitialize(Poco::Util::Application &self) override;
		inline void defineOptions(Poco::Util::OptionSet &options) override;
		inline void handleHelp(const std::string &name, const std::string &value);
		inline void handleVersion(const std::string &name, const std::string &value);
		inline void handleDebug(const std::string &name, const std::string &value);
		inline void handleLogs(const std::string &name, const std::string &value);
		inline void handleConfig(const std::string &name, const std::string &value);
		inline void displayHelp();
		inline void InitializeSubSystemServers();
		inline void StartSubSystemServers();
		inline void StopSubSystemServers();
		[[nodiscard]] static inline std::string CreateUUID();
		inline bool SetSubsystemLogLevel(const std::string &SubSystem, const std::string &Level);
		inline void Reload(const std::string &Sub);
		inline Types::StringVec GetSubSystems() const;
		inline Types::StringPairVec GetLogLevels();
		inline const Types::StringVec & GetLogLevelNames();
		inline uint64_t ConfigGetInt(const std::string &Key,uint64_t Default);
		inline uint64_t ConfigGetInt(const std::string &Key);
		inline uint64_t ConfigGetBool(const std::string &Key,bool Default);
		inline uint64_t ConfigGetBool(const std::string &Key);
		inline std::string ConfigGetString(const std::string &Key,const std::string & Default);
		inline std::string ConfigGetString(const std::string &Key);
		inline std::string ConfigPath(const std::string &Key,const std::string & Default);
		inline std::string ConfigPath(const std::string &Key);
		inline std::string Encrypt(const std::string &S);
		inline std::string Decrypt(const std::string &S);
		inline std::string CreateHash(const std::string &S);
		inline std::string MakeSystemEventMessage( const std::string & Type ) const;
		[[nodiscard]] inline bool IsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request);
		inline static void SavePID();
		inline int main(const ArgVec &args) override;
		static MicroService & instance() { return *instance_; }

	  private:
	    static MicroService         * instance_;
		bool                        HelpRequested_ = false;
		std::string                 LogDir_;
		std::string                 ConfigFileName_;
		Poco::UUIDGenerator         UUIDGenerator_;
		uint64_t                    ID_ = 1;
		Poco::SharedPtr<Poco::Crypto::RSAKey>	AppKey_;
		bool                        DebugMode_ = false;
		std::string 				DataDir_;
		std::string                 WWWAssetsDir_;
		SubSystemVec			    SubSystems_;
		Poco::Crypto::CipherFactory & CipherFactory_ = Poco::Crypto::CipherFactory::defaultFactory();
		Poco::Crypto::Cipher        * Cipher_ = nullptr;
		Poco::SHA2Engine			SHA2_;
		MicroServiceMetaMap			Services_;
		std::string 				MyHash_;
		std::string 				MyPrivateEndPoint_;
		std::string 				MyPublicEndPoint_;
		std::string                 UIURI_;
		std::string 				Version_{std::string(APP_VERSION) + "("+ BUILD_NUMBER + ")"};
		BusEventManager				BusEventManager_;
		std::mutex 					InfraMutex_;
		std::default_random_engine  RandomEngine_;

		std::string DAEMON_PROPERTIES_FILENAME;
		std::string DAEMON_ROOT_ENV_VAR;
		std::string DAEMON_CONFIG_ENV_VAR;
		std::string DAEMON_APP_NAME;
		uint64_t 	DAEMON_BUS_TIMER;
	};

	inline void MicroService::Exit(int Reason) {
	    std::exit(Reason);
	}

	inline void MicroService::BusMessageReceived(const std::string &Key, const std::string & Message) {
	    std::lock_guard G(InfraMutex_);
	    try {
	        Poco::JSON::Parser P;
	        auto Object = P.parse(Message).extract<Poco::JSON::Object::Ptr>();
	        if (Object->has(KafkaTopics::ServiceEvents::Fields::ID) &&
	        Object->has(KafkaTopics::ServiceEvents::Fields::EVENT)) {
	            uint64_t 	ID = Object->get(KafkaTopics::ServiceEvents::Fields::ID);
	            auto 		Event = Object->get(KafkaTopics::ServiceEvents::Fields::EVENT).toString();
	            if (ID != ID_) {
	                if(	Event==KafkaTopics::ServiceEvents::EVENT_JOIN ||
	                Event==KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE ||
	                Event==KafkaTopics::ServiceEvents::EVENT_LEAVE ) {
	                    if(	Object->has(KafkaTopics::ServiceEvents::Fields::TYPE) &&
	                    Object->has(KafkaTopics::ServiceEvents::Fields::PUBLIC) &&
	                    Object->has(KafkaTopics::ServiceEvents::Fields::PRIVATE) &&
	                    Object->has(KafkaTopics::ServiceEvents::Fields::VRSN) &&
	                    Object->has(KafkaTopics::ServiceEvents::Fields::KEY)) {

	                        if (Event == KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE && Services_.find(ID) != Services_.end()) {
	                            Services_[ID].LastUpdate = std::time(nullptr);
	                        } else if (Event == KafkaTopics::ServiceEvents::EVENT_LEAVE) {
	                            Services_.erase(ID);
	                            logger().information(Poco::format("Service %s ID=%Lu leaving system.",Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),ID));
	                        } else if (Event == KafkaTopics::ServiceEvents::EVENT_JOIN || Event == KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE) {
	                            logger().information(Poco::format("Service %s ID=%Lu joining system.",Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),ID));
	                            Services_[ID] = MicroServiceMeta{
	                                .Id = ID,
	                                .Type = Poco::toLower(Object->get(KafkaTopics::ServiceEvents::Fields::TYPE).toString()),
	                                .PrivateEndPoint = Object->get(KafkaTopics::ServiceEvents::Fields::PRIVATE).toString(),
	                                .PublicEndPoint = Object->get(KafkaTopics::ServiceEvents::Fields::PUBLIC).toString(),
	                                .AccessKey = Object->get(KafkaTopics::ServiceEvents::Fields::KEY).toString(),
	                                .Version = Object->get(KafkaTopics::ServiceEvents::Fields::VRSN).toString(),
	                                .LastUpdate = (uint64_t)std::time(nullptr)};
	                            for (const auto &[Id, Svc] : Services_) {
	                                logger().information(Poco::format("ID: %Lu Type: %s EndPoint: %s",Id,Svc.Type,Svc.PrivateEndPoint));
	                            }
	                        }
	                    } else {
	                        logger().error(Poco::format("KAFKA-MSG: invalid event '%s', missing a field.",Event));
	                    }
	                } else if (Event==KafkaTopics::ServiceEvents::EVENT_REMOVE_TOKEN) {
	                    if(Object->has(KafkaTopics::ServiceEvents::Fields::TOKEN)) {
#ifndef TIP_SECURITY_SERVICE
	                        AuthClient()->RemovedCachedToken(Object->get(KafkaTopics::ServiceEvents::Fields::TOKEN).toString());
#endif
	                    } else {
	                        logger().error(Poco::format("KAFKA-MSG: invalid event '%s', missing token",Event));
	                    }
	                } else {
	                    logger().error(Poco::format("Unknown Event: %s Source: %Lu", Event, ID));
	                }
	            }
	        } else {
	            logger().error("Bad bus message.");
	        }

	        auto i=Services_.begin();
	        auto Now = (uint64_t )std::time(nullptr);
	        for(;i!=Services_.end();) {
	            if((Now - i->second.LastUpdate)>60) {
	                i = Services_.erase(i);
	            } else
	                ++i;
	        }

	    } catch (const Poco::Exception &E) {
	        logger().log(E);
	    }
	}

	inline MicroServiceMetaVec MicroService::GetServices(const std::string & Type) {
	    std::lock_guard G(InfraMutex_);

	    auto T = Poco::toLower(Type);
	    MicroServiceMetaVec	Res;
	    for(const auto &[Id,ServiceRec]:Services_) {
	        if(ServiceRec.Type==T)
	            Res.push_back(ServiceRec);
	    }
	    return Res;
	}

	inline MicroServiceMetaVec MicroService::GetServices() {
	    std::lock_guard G(InfraMutex_);

	    MicroServiceMetaVec	Res;
	    for(const auto &[Id,ServiceRec]:Services_) {
	        Res.push_back(ServiceRec);
	    }
	    return Res;
	}

	inline void MicroService::LoadConfigurationFile() {
	    std::string Location = Poco::Environment::get(DAEMON_CONFIG_ENV_VAR,".");
	    Poco::Path ConfigFile;

	    ConfigFile = ConfigFileName_.empty() ? Location + "/" + DAEMON_PROPERTIES_FILENAME : ConfigFileName_;

	    if(!ConfigFile.isFile())
	    {
	        std::cerr << DAEMON_APP_NAME << ": Configuration "
	        << ConfigFile.toString() << " does not seem to exist. Please set " + DAEMON_CONFIG_ENV_VAR
	        + " env variable the path of the " + DAEMON_PROPERTIES_FILENAME + " file." << std::endl;
	        std::exit(Poco::Util::Application::EXIT_CONFIG);
	    }

	    loadConfiguration(ConfigFile.toString());
	}

	inline void MicroService::Reload() {
	    LoadConfigurationFile();
	    LoadMyConfig();
	}

	inline void MicroService::LoadMyConfig() {
	    std::string KeyFile = ConfigPath("openwifi.service.key");
	    std::string KeyFilePassword = ConfigPath("openwifi.service.key.password" , "" );
	    AppKey_ = Poco::SharedPtr<Poco::Crypto::RSAKey>(new Poco::Crypto::RSAKey("", KeyFile, KeyFilePassword));
	    Cipher_ = CipherFactory_.createCipher(*AppKey_);
	    ID_ = Utils::GetSystemId();
	    if(!DebugMode_)
	        DebugMode_ = ConfigGetBool("openwifi.system.debug",false);
	    MyPrivateEndPoint_ = ConfigGetString("openwifi.system.uri.private");
	    MyPublicEndPoint_ = ConfigGetString("openwifi.system.uri.public");
	    UIURI_ = ConfigGetString("openwifi.system.uri.ui");
	    MyHash_ = CreateHash(MyPublicEndPoint_);
	}

	void MicroServicePostInitialization();

	inline void MicroService::initialize(Poco::Util::Application &self) {
	    // add the default services
	    SubSystems_.push_back(KafkaManager());
	    SubSystems_.push_back(ALBHealthCheckServer());
	    SubSystems_.push_back(RESTAPI_server());
	    SubSystems_.push_back(RESTAPI_InternalServer());

	    Poco::Net::initializeSSL();
	    Poco::Net::HTTPStreamFactory::registerFactory();
	    Poco::Net::HTTPSStreamFactory::registerFactory();
	    Poco::Net::FTPStreamFactory::registerFactory();
	    Poco::Net::FTPSStreamFactory::registerFactory();

	    LoadConfigurationFile();

	    static const char * LogFilePathKey = "logging.channels.c2.path";

	    if(LogDir_.empty()) {
	        std::string OriginalLogFileValue = ConfigPath(LogFilePathKey);
	        config().setString(LogFilePathKey, OriginalLogFileValue);
	    } else {
	        config().setString(LogFilePathKey, LogDir_);
	    }

	    Poco::File	DataDir(ConfigPath("openwifi.system.data"));
	    DataDir_ = DataDir.path();
	    if(!DataDir.exists()) {
	        try {
	            DataDir.createDirectory();
	        } catch (const Poco::Exception &E) {
	            logger().log(E);
	        }
	    }
	    WWWAssetsDir_ = ConfigPath("openwifi.restapi.wwwassets","");
	    if(WWWAssetsDir_.empty())
	        WWWAssetsDir_ = DataDir_;

	    LoadMyConfig();

	    InitializeSubSystemServers();
	    ServerApplication::initialize(self);

	    Types::TopicNotifyFunction F = [this](std::string s1,std::string s2) { this->BusMessageReceived(s1,s2); };
	    KafkaManager()->RegisterTopicWatcher(KafkaTopics::SERVICE_EVENTS, F);

	    MicroServicePostInitialization();
	}

	inline void MicroService::uninitialize() {
	    // add your own uninitialization code here
	    ServerApplication::uninitialize();
	}

	inline void MicroService::reinitialize(Poco::Util::Application &self) {
	    ServerApplication::reinitialize(self);
	    // add your own reinitialization code here
	}

	inline void MicroService::defineOptions(Poco::Util::OptionSet &options) {
	    ServerApplication::defineOptions(options);

	    options.addOption(
	            Poco::Util::Option("help", "", "display help information on command line arguments")
	            .required(false)
	            .repeatable(false)
	            .callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleHelp)));

	    options.addOption(
	            Poco::Util::Option("file", "", "specify the configuration file")
	            .required(false)
	            .repeatable(false)
	            .argument("file")
	            .callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleConfig)));

	    options.addOption(
	            Poco::Util::Option("debug", "", "to run in debug, set to true")
	            .required(false)
	            .repeatable(false)
	            .callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleDebug)));

	    options.addOption(
	            Poco::Util::Option("logs", "", "specify the log directory and file (i.e. dir/file.log)")
	            .required(false)
	            .repeatable(false)
	            .argument("dir")
	            .callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleLogs)));

	    options.addOption(
	            Poco::Util::Option("version", "", "get the version and quit.")
	            .required(false)
	            .repeatable(false)
	            .callback(Poco::Util::OptionCallback<MicroService>(this, &MicroService::handleVersion)));

	}

	inline void MicroService::handleHelp(const std::string &name, const std::string &value) {
	    HelpRequested_ = true;
	    displayHelp();
	    stopOptionsProcessing();
	}

	inline void MicroService::handleVersion(const std::string &name, const std::string &value) {
	    HelpRequested_ = true;
	    std::cout << Version() << std::endl;
	    stopOptionsProcessing();
	}

	inline void MicroService::handleDebug(const std::string &name, const std::string &value) {
	    if(value == "true")
	        DebugMode_ = true ;
	}

	inline void MicroService::handleLogs(const std::string &name, const std::string &value) {
	    LogDir_ = value;
	}

	inline void MicroService::handleConfig(const std::string &name, const std::string &value) {
	    ConfigFileName_ = value;
	}

	inline void MicroService::displayHelp() {
	    Poco::Util::HelpFormatter helpFormatter(options());
	    helpFormatter.setCommand(commandName());
	    helpFormatter.setUsage("OPTIONS");
	    helpFormatter.setHeader("A " + DAEMON_APP_NAME + " implementation for TIP.");
	    helpFormatter.format(std::cout);
	}

	inline void MicroService::InitializeSubSystemServers() {
	    for(auto i:SubSystems_)
	        addSubsystem(i);
	}

	inline void MicroService::StartSubSystemServers() {
	    for(auto i:SubSystems_) {
	        i->Start();
	    }
	    BusEventManager_.Start();
	}

	inline void MicroService::StopSubSystemServers() {
	    BusEventManager_.Stop();
	    for(auto i=SubSystems_.rbegin(); i!=SubSystems_.rend(); ++i) {
			(*i)->Stop();
		}
	}

	[[nodiscard]] inline std::string MicroService::CreateUUID() {
        static std::random_device              rd;
        static std::mt19937_64                 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static std::uniform_int_distribution<> dis2(8, 11);

        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }

	inline bool MicroService::SetSubsystemLogLevel(const std::string &SubSystem, const std::string &Level) {
	    try {
	        auto P = Poco::Logger::parseLevel(Level);
	        auto Sub = Poco::toLower(SubSystem);

	        if (Sub == "all") {
	            for (auto i : SubSystems_) {
	                i->Logger().setLevel(P);
	            }
	            return true;
	        } else {
	            for (auto i : SubSystems_) {
	                if (Sub == Poco::toLower(i->Name())) {
	                    i->Logger().setLevel(P);
	                    return true;
	                }
	            }
	        }
	    } catch (const Poco::Exception & E) {
	        std::cout << "Exception" << std::endl;
	    }
	    return false;
	}

	inline void MicroService::Reload(const std::string &Sub) {
	    for (auto i : SubSystems_) {
	        if (Poco::toLower(Sub) == Poco::toLower(i->Name())) {
	            i->reinitialize(Poco::Util::Application::instance());
	            return;
	        }
	    }
	}

	inline Types::StringVec MicroService::GetSubSystems() const {
	    Types::StringVec Result;
	    for(auto i:SubSystems_)
	        Result.push_back(Poco::toLower(i->Name()));
	    return Result;
	}

	inline Types::StringPairVec MicroService::GetLogLevels() {
	    Types::StringPairVec Result;

	    for(auto &i:SubSystems_) {
	        auto P = std::make_pair( i->Name(), Utils::LogLevelToString(i->GetLoggingLevel()));
	        Result.push_back(P);
	    }
	    return Result;
	}

	inline const Types::StringVec & MicroService::GetLogLevelNames() {
	    static Types::StringVec LevelNames{"none", "fatal", "critical", "error", "warning", "notice", "information", "debug", "trace" };
	    return LevelNames;
	}

	inline uint64_t MicroService::ConfigGetInt(const std::string &Key,uint64_t Default) {
	    return (uint64_t) config().getInt64(Key,Default);
	}

	inline uint64_t MicroService::ConfigGetInt(const std::string &Key) {
	    return config().getInt(Key);
	}

	inline uint64_t MicroService::ConfigGetBool(const std::string &Key,bool Default) {
	    return config().getBool(Key,Default);
	}

	inline uint64_t MicroService::ConfigGetBool(const std::string &Key) {
	    return config().getBool(Key);
	}

	inline std::string MicroService::ConfigGetString(const std::string &Key,const std::string & Default) {
	    return config().getString(Key, Default);
	}

	inline std::string MicroService::ConfigGetString(const std::string &Key) {
	    return config().getString(Key);
	}

	inline std::string MicroService::ConfigPath(const std::string &Key,const std::string & Default) {
	    std::string R = config().getString(Key, Default);
	    return Poco::Path::expand(R);
	}

	inline std::string MicroService::ConfigPath(const std::string &Key) {
	    std::string R = config().getString(Key);
	    return Poco::Path::expand(R);
	}

	inline std::string MicroService::Encrypt(const std::string &S) {
	    return Cipher_->encryptString(S, Poco::Crypto::Cipher::Cipher::ENC_BASE64);;
	}

	inline std::string MicroService::Decrypt(const std::string &S) {
	    return Cipher_->decryptString(S, Poco::Crypto::Cipher::Cipher::ENC_BASE64);;
	}

	inline std::string MicroService::CreateHash(const std::string &S) {
	    SHA2_.update(S);
	    return Utils::ToHex(SHA2_.digest());
	}

	inline std::string MicroService::MakeSystemEventMessage( const std::string & Type ) const {
	    Poco::JSON::Object	Obj;
	    Obj.set(KafkaTopics::ServiceEvents::Fields::EVENT,Type);
	    Obj.set(KafkaTopics::ServiceEvents::Fields::ID,ID_);
	    Obj.set(KafkaTopics::ServiceEvents::Fields::TYPE,Poco::toLower(DAEMON_APP_NAME));
	    Obj.set(KafkaTopics::ServiceEvents::Fields::PUBLIC,MyPublicEndPoint_);
	    Obj.set(KafkaTopics::ServiceEvents::Fields::PRIVATE,MyPrivateEndPoint_);
	    Obj.set(KafkaTopics::ServiceEvents::Fields::KEY,MyHash_);
	    Obj.set(KafkaTopics::ServiceEvents::Fields::VRSN,Version_);
	    std::stringstream ResultText;
	    Poco::JSON::Stringifier::stringify(Obj, ResultText);
	    return ResultText.str();
	}

	[[nodiscard]] inline bool MicroService::IsValidAPIKEY(const Poco::Net::HTTPServerRequest &Request) {
	    try {
	        auto APIKEY = Request.get("X-API-KEY");
	        return APIKEY == MyHash_;
	    } catch (const Poco::Exception &E) {
	        logger().log(E);
	    }
	    return false;
	}

	inline void MicroService::SavePID() {
	    try {
	        std::ofstream O;
	        O.open(MicroService::instance().DataDir() + "/pidfile",std::ios::binary | std::ios::trunc);
	        O << Poco::Process::id();
	        O.close();
	    } catch (...)
	    {
	        std::cout << "Could not save system ID" << std::endl;
	    }
	}

	inline int MicroService::main(const ArgVec &args) {

	    MyErrorHandler	ErrorHandler(*this);
	    Poco::ErrorHandler::set(&ErrorHandler);

	    if (!HelpRequested_) {
	        SavePID();
	        Poco::Logger &logger = Poco::Logger::get(DAEMON_APP_NAME);
	        logger.notice(Poco::format("Starting %s version %s.",DAEMON_APP_NAME, Version()));

	        if(Poco::Net::Socket::supportsIPv6())
	            logger.information("System supports IPv6.");
	        else
	            logger.information("System does NOT support IPv6.");

	        if (config().getBool("application.runAsDaemon", false)) {
	            logger.information("Starting as a daemon.");
	        }
	        logger.information(Poco::format("System ID set to %Lu",ID_));
	        StartSubSystemServers();
	        waitForTerminationRequest();
	        StopSubSystemServers();
	        logger.notice(Poco::format("Stopped %s...",DAEMON_APP_NAME));
	    }

	    return Application::EXIT_OK;
	}

	AppServiceRegistry::AppServiceRegistry() {
		FileName = MicroService::instance().DataDir() + "/registry.json";
		Poco::File F(FileName);

		try {
			if(F.exists()) {
				std::ostringstream  OS;
				std::ifstream       IF(FileName);
				Poco::StreamCopier::copyStream(IF, OS);
				Registry_ = nlohmann::json::parse(OS.str());
			}
		} catch (...) {
			Registry_ = nlohmann::json::parse("{}");
		}
	}

	inline void SubSystemServer::initialize(Poco::Util::Application &self) {
	    Logger_.notice("Initializing...");
	    auto i = 0;
	    bool good = true;

	    ConfigServersList_.clear();
	    while (good) {
	        std::string root{SubSystemConfigPrefix_ + ".host." + std::to_string(i) + "."};

	        std::string address{root + "address"};
	        if (MicroService::instance().ConfigGetString(address, "").empty()) {
	            good = false;
	        } else {
	            std::string port{root + "port"};
	            std::string key{root + "key"};
	            std::string key_password{root + "key.password"};
	            std::string cert{root + "cert"};
	            std::string name{root + "name"};
	            std::string backlog{root + "backlog"};
	            std::string rootca{root + "rootca"};
	            std::string issuer{root + "issuer"};
	            std::string clientcas(root + "clientcas");
	            std::string cas{root + "cas"};

	            std::string level{root + "security"};
	            Poco::Net::Context::VerificationMode M = Poco::Net::Context::VERIFY_RELAXED;

	            auto L = MicroService::instance().ConfigGetString(level, "");

	            if (L == "strict") {
	                M = Poco::Net::Context::VERIFY_STRICT;
	            } else if (L == "none") {
	                M = Poco::Net::Context::VERIFY_NONE;
	            } else if (L == "relaxed") {
	                M = Poco::Net::Context::VERIFY_RELAXED;
	            } else if (L == "once")
	                M = Poco::Net::Context::VERIFY_ONCE;

	            PropertiesFileServerEntry entry(MicroService::instance().ConfigGetString(address, ""),
                                                MicroService::instance().ConfigGetInt(port, 0),
                                                MicroService::instance().ConfigPath(key, ""),
                                                MicroService::instance().ConfigPath(cert, ""),
                                                MicroService::instance().ConfigPath(rootca, ""),
                                                MicroService::instance().ConfigPath(issuer, ""),
                                                MicroService::instance().ConfigPath(clientcas, ""),
                                                MicroService::instance().ConfigPath(cas, ""),
                                                MicroService::instance().ConfigGetString(key_password, ""),
                                                MicroService::instance().ConfigGetString(name, ""), M,
                                                (int)MicroService::instance().ConfigGetInt(backlog, 64));
	            ConfigServersList_.push_back(entry);
	            i++;
	        }
	    }
	}

	inline int ALBHealthCheckServer::Start() {
	    if(MicroService::instance().ConfigGetBool("alb.enable",false)) {
	        Running_=true;
	        Port_ = (int)MicroService::instance().ConfigGetInt("alb.port",15015);
	        Socket_ = std::make_unique<Poco::Net::ServerSocket>(Port_);
	        auto Params = new Poco::Net::HTTPServerParams;
	        Server_ = std::make_unique<Poco::Net::HTTPServer>(new ALBRequestHandlerFactory(Logger_), *Socket_, Params);
	        Server_->start();
	    }

	    return 0;
	}

    inline void BusEventManager::run() {
        Running_ = true;
        auto Msg = MicroService::instance().MakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_JOIN);
        KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,MicroService::instance().PrivateEndPoint(),Msg, false);
        while(Running_) {
            Poco::Thread::trySleep((unsigned long)MicroService::instance().DaemonBusTimer());
            if(!Running_)
                break;
            Msg = MicroService::instance().MakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_KEEP_ALIVE);
            KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,MicroService::instance().PrivateEndPoint(),Msg, false);
        }
        Msg = MicroService::instance().MakeSystemEventMessage(KafkaTopics::ServiceEvents::EVENT_LEAVE);
        KafkaManager()->PostMessage(KafkaTopics::SERVICE_EVENTS,MicroService::instance().PrivateEndPoint(),Msg, false);
    };

	inline void BusEventManager::Start() {
        if(KafkaManager()->Enabled()) {
            Thread_.start(*this);
        }
    }

    inline void BusEventManager::Stop() {
        if(KafkaManager()->Enabled()) {
            Running_ = false;
            Thread_.wakeUp();
            Thread_.join();
        }
    }

    inline void KafkaManager::initialize(Poco::Util::Application & self) {
	    SubSystemServer::initialize(self);
	    KafkaEnabled_ = MicroService::instance().ConfigGetBool("openwifi.kafka.enable",false);
	}

	inline void KafkaProducer::run() {
	    cppkafka::Configuration Config({
	        { "client.id", MicroService::instance().ConfigGetString("openwifi.kafka.client.id") },
	        { "metadata.broker.list", MicroService::instance().ConfigGetString("openwifi.kafka.brokerlist") }
	    });
	    KafkaManager()->SystemInfoWrapper_ = 	R"lit({ "system" : { "id" : )lit" +
	            std::to_string(MicroService::instance().ID()) +
	            R"lit( , "host" : ")lit" + MicroService::instance().PrivateEndPoint() +
	            R"lit(" } , "payload" : )lit" ;
	    cppkafka::Producer	Producer(Config);
	    Running_ = true;
	    while(Running_) {
	        std::this_thread::sleep_for(std::chrono::milliseconds(200));
	        try
	        {
	            std::lock_guard G(Mutex_);
	            auto Num=0;
	            while (!KafkaManager()->Queue_.empty()) {
	                const auto M = KafkaManager()->Queue_.front();
	                Producer.produce(
	                        cppkafka::MessageBuilder(M.Topic).key(M.Key).payload(M.PayLoad));
	                KafkaManager()->Queue_.pop();
	                Num++;
	            }
	            if(Num)
	                Producer.flush();
	        } catch (const cppkafka::HandleException &E ) {
	            KafkaManager()->Logger_.warning(Poco::format("Caught a Kafka exception (producer): %s",std::string{E.what()}));
	        } catch (const Poco::Exception &E) {
	            KafkaManager()->Logger_.log(E);
	        }
	    }
	    Producer.flush();
	}

	inline void KafkaConsumer::run() {
	    cppkafka::Configuration Config({
	        { "client.id", MicroService::instance().ConfigGetString("openwifi.kafka.client.id") },
	        { "metadata.broker.list", MicroService::instance().ConfigGetString("openwifi.kafka.brokerlist") },
	        { "group.id", MicroService::instance().ConfigGetString("openwifi.kafka.group.id") },
	        { "enable.auto.commit", MicroService::instance().ConfigGetBool("openwifi.kafka.auto.commit",false) },
	        { "auto.offset.reset", "latest" } ,
	        { "enable.partition.eof", false }
	    });

	    cppkafka::TopicConfiguration topic_config = {
	            { "auto.offset.reset", "smallest" }
	    };

	    // Now configure it to be the default topic config
	    Config.set_default_topic_configuration(topic_config);

	    cppkafka::Consumer Consumer(Config);
	    Consumer.set_assignment_callback([this](cppkafka::TopicPartitionList& partitions) {
	        if(!partitions.empty()) {
	            KafkaManager()->Logger_.information(Poco::format("Partition assigned: %Lu...",
                                                 (uint64_t)partitions.front().get_partition()));
	        }
	    });
	    Consumer.set_revocation_callback([this](const cppkafka::TopicPartitionList& partitions) {
	        if(!partitions.empty()) {
	            KafkaManager()->Logger_.information(Poco::format("Partition revocation: %Lu...",
                                                 (uint64_t)partitions.front().get_partition()));
	        }
	    });

	    bool AutoCommit = MicroService::instance().ConfigGetBool("openwifi.kafka.auto.commit",false);
	    auto BatchSize = MicroService::instance().ConfigGetInt("openwifi.kafka.consumer.batchsize",20);

	    Types::StringVec    Topics;
	    for(const auto &i:KafkaManager()->Notifiers_)
	        Topics.push_back(i.first);

	    Consumer.subscribe(Topics);

	    Running_ = true;
	    while(Running_) {
	        try {
	            std::vector<cppkafka::Message> MsgVec = Consumer.poll_batch(BatchSize, std::chrono::milliseconds(200));
	            for(auto const &Msg:MsgVec) {
	                if (!Msg)
	                    continue;
	                if (Msg.get_error()) {
	                    if (!Msg.is_eof()) {
	                        KafkaManager()->Logger_.error(Poco::format("Error: %s", Msg.get_error().to_string()));
	                    }if(!AutoCommit)
	                        Consumer.async_commit(Msg);
	                    continue;
	                }
	                std::lock_guard G(Mutex_);
	                auto It = KafkaManager()->Notifiers_.find(Msg.get_topic());
	                if (It != KafkaManager()->Notifiers_.end()) {
	                    Types::TopicNotifyFunctionList &FL = It->second;
	                    std::string Key{Msg.get_key()};
	                    std::string Payload{Msg.get_payload()};
	                    for (auto &F : FL) {
	                        std::thread T(F.first, Key, Payload);
	                        T.detach();
	                    }
	                }
	                if (!AutoCommit)
	                    Consumer.async_commit(Msg);
	            }
	        } catch (const cppkafka::HandleException &E) {
	            KafkaManager()->Logger_.warning(Poco::format("Caught a Kafka exception (consumer): %s",std::string{E.what()}));
	        } catch (const Poco::Exception &E) {
	            KafkaManager()->Logger_.log(E);
	        }
	    }
	    Consumer.unsubscribe();
	}

	inline void RESTAPI_server::reinitialize(Poco::Util::Application &self) {
	    MicroService::instance().LoadConfigurationFile();
	    Logger_.information("Reinitializing.");
	    Stop();
	    Start();
	}

	void RESTAPI_InternalServer::reinitialize(Poco::Util::Application &self) {
	    MicroService::instance().LoadConfigurationFile();
	    Logger_.information("Reinitializing.");
	    Stop();
	    Start();
	}

	class RESTAPI_system_command : public RESTAPIHandler {
	public:
	    RESTAPI_system_command(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server, bool Internal)
	    : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{Poco::Net::HTTPRequest::HTTP_POST,
                                                  Poco::Net::HTTPRequest::HTTP_GET,
                                                  Poco::Net::HTTPRequest::HTTP_OPTIONS},
                                                  Server,
                                                  Internal) {}
                                                  static const std::list<const char *> PathName() { return std::list<const char *>{"/api/v1/system"};}

        inline void DoGet() {
	        std::string Arg;
	        if(HasParameter("command",Arg) && Arg=="info") {
	            Poco::JSON::Object Answer;
	            Answer.set(RESTAPI::Protocol::VERSION, MicroService::instance().Version());
	            Answer.set(RESTAPI::Protocol::UPTIME, MicroService::instance().uptime().totalSeconds());
	            Answer.set(RESTAPI::Protocol::START, MicroService::instance().startTime().epochTime());
	            Answer.set(RESTAPI::Protocol::OS, Poco::Environment::osName());
	            Answer.set(RESTAPI::Protocol::PROCESSORS, Poco::Environment::processorCount());
	            Answer.set(RESTAPI::Protocol::HOSTNAME, Poco::Environment::nodeName());
	            Answer.set(RESTAPI::Protocol::UI, MicroService::instance().GetUIURI());

	            Poco::JSON::Array   Certificates;
	            auto SubSystems = MicroService::instance().GetFullSubSystems();
	            std::set<std::string>   CertNames;

	            for(const auto &i:SubSystems) {
	                auto Hosts=i->HostSize();
	                for(uint64_t j=0;j<Hosts;++j) {
	                    auto CertFileName = i->Host(j).CertFile();
	                    if(!CertFileName.empty()) {
	                        auto InsertResult = CertNames.insert(CertFileName);
	                        if(InsertResult.second) {
	                            Poco::JSON::Object  Inner;
	                            Poco::Path  F(CertFileName);
	                            Inner.set("filename", F.getFileName());
	                            Poco::Crypto::X509Certificate   C(CertFileName);
	                            auto ExpiresOn = C.expiresOn();
	                            Inner.set("expiresOn",ExpiresOn.timestamp().epochTime());
	                            Certificates.add(Inner);
	                        }
	                    }
	                }
	            }
	            Answer.set("certificates", Certificates);
	            return ReturnObject(Answer);
	        }
	        BadRequest(RESTAPI::Errors::InvalidCommand);
	    }

	    inline void DoPost() final {
	        auto Obj = ParseStream();
	        if (Obj->has(RESTAPI::Protocol::COMMAND)) {
	            auto Command = Poco::toLower(Obj->get(RESTAPI::Protocol::COMMAND).toString());
	            if (Command == RESTAPI::Protocol::SETLOGLEVEL) {
	                if (Obj->has(RESTAPI::Protocol::SUBSYSTEMS) &&
	                Obj->isArray(RESTAPI::Protocol::SUBSYSTEMS)) {
	                    auto ParametersBlock = Obj->getArray(RESTAPI::Protocol::SUBSYSTEMS);
	                    for (const auto &i : *ParametersBlock) {
	                        Poco::JSON::Parser pp;
	                        auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
	                        if (InnerObj->has(RESTAPI::Protocol::TAG) &&
	                        InnerObj->has(RESTAPI::Protocol::VALUE)) {
	                            auto Name = GetS(RESTAPI::Protocol::TAG, InnerObj);
	                            auto Value = GetS(RESTAPI::Protocol::VALUE, InnerObj);
	                            MicroService::instance().SetSubsystemLogLevel(Name, Value);
	                            Logger_.information(
	                                    Poco::format("Setting log level for %s at %s", Name, Value));
	                        }
	                    }
	                    return OK();
	                }
	            } else if (Command == RESTAPI::Protocol::GETLOGLEVELS) {
	                auto CurrentLogLevels = MicroService::instance().GetLogLevels();
	                Poco::JSON::Object Result;
	                Poco::JSON::Array Array;
	                for (auto &[Name, Level] : CurrentLogLevels) {
	                    Poco::JSON::Object Pair;
	                    Pair.set(RESTAPI::Protocol::TAG, Name);
	                    Pair.set(RESTAPI::Protocol::VALUE, Level);
	                    Array.add(Pair);
	                }
	                Result.set(RESTAPI::Protocol::TAGLIST, Array);
	                return ReturnObject(Result);
	            } else if (Command == RESTAPI::Protocol::GETLOGLEVELNAMES) {
	                Poco::JSON::Object Result;
	                Poco::JSON::Array LevelNamesArray;
	                const Types::StringVec &LevelNames =  MicroService::instance().GetLogLevelNames();
	                for (const auto &i : LevelNames)
	                    LevelNamesArray.add(i);
	                Result.set(RESTAPI::Protocol::LIST, LevelNamesArray);
	                return ReturnObject(Result);
	            } else if (Command == RESTAPI::Protocol::GETSUBSYSTEMNAMES) {
	                Poco::JSON::Object Result;
	                Poco::JSON::Array LevelNamesArray;
	                const Types::StringVec &SubSystemNames =  MicroService::instance().GetSubSystems();
	                for (const auto &i : SubSystemNames)
	                    LevelNamesArray.add(i);
	                Result.set(RESTAPI::Protocol::LIST, LevelNamesArray);
	                return ReturnObject(Result);
	            } else if (Command == RESTAPI::Protocol::STATS) {

	            } else if (Command == RESTAPI::Protocol::RELOAD) {
	                if (Obj->has(RESTAPI::Protocol::SUBSYSTEMS) &&
	                Obj->isArray(RESTAPI::Protocol::SUBSYSTEMS)) {
	                    auto SubSystems = Obj->getArray(RESTAPI::Protocol::SUBSYSTEMS);
	                    std::vector<std::string> Names;
	                    for (const auto &i : *SubSystems)
	                        Names.push_back(i.toString());
	                    std::thread	ReloadThread([Names](){
	                        std::this_thread::sleep_for(10000ms);
	                        for(const auto &i:Names) {
	                            if(i=="daemon")
	                                MicroService::instance().Reload();
	                            else
	                                MicroService::instance().Reload(i);
	                        }
	                    });
	                    ReloadThread.detach();
	                }
	                return OK();
	            }
	        } else {
	            return BadRequest(RESTAPI::Errors::InvalidCommand);
	        }
	        BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	    }

	    void DoPut() final {};
	    void DoDelete() final {};
	};

    inline int OpenAPIRequestGet::Do(Poco::JSON::Object::Ptr &ResponseObject) {
        try {
            auto Services = MicroService::instance().GetServices(Type_);
            for(auto const &Svc:Services) {
                Poco::URI	URI(Svc.PrivateEndPoint);
                Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());

                URI.setPath(EndPoint_);
                for (const auto &qp : QueryData_)
                    URI.addQueryParameter(qp.first, qp.second);

                std::string Path(URI.getPathAndQuery());
                Session.setTimeout(Poco::Timespan(msTimeout_/1000, msTimeout_ % 1000));

                Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_GET,
                                               Path,
                                               Poco::Net::HTTPMessage::HTTP_1_1);
                Request.add("X-API-KEY", Svc.AccessKey);
                Request.add("X-INTERNAL-NAME", MicroService::instance().PublicEndPoint());
                Session.sendRequest(Request);

                Poco::Net::HTTPResponse Response;
                std::istream &is = Session.receiveResponse(Response);
                if(Response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
                    Poco::JSON::Parser	P;
                    ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                }
                return Response.getStatus();
            }
        }
        catch (const Poco::Exception &E)
        {
            std::cerr << E.displayText() << std::endl;
        }
        return -1;
    }

    inline int OpenAPIRequestPut::Do(Poco::JSON::Object::Ptr &ResponseObject) {
        try {
            auto Services = MicroService::instance().GetServices(Type_);
            for(auto const &Svc:Services) {
                Poco::URI	URI(Svc.PrivateEndPoint);
                Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());

                URI.setPath(EndPoint_);
                for (const auto &qp : QueryData_)
                    URI.addQueryParameter(qp.first, qp.second);

                std::string Path(URI.getPathAndQuery());
                Session.setTimeout(Poco::Timespan(msTimeout_/1000, msTimeout_ % 1000));

                Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_PUT,
                                               Path,
                                               Poco::Net::HTTPMessage::HTTP_1_1);
                std::ostringstream obody;
                Poco::JSON::Stringifier::stringify(Body_,obody);

                Request.setContentType("application/json");
                Request.setContentLength(obody.str().size());

                Request.add("X-API-KEY", Svc.AccessKey);
                Request.add("X-INTERNAL-NAME", MicroService::instance().PublicEndPoint());

                std::ostream & os = Session.sendRequest(Request);
                os << obody.str();

                Poco::Net::HTTPResponse Response;
                std::istream &is = Session.receiveResponse(Response);
                if(Response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
                    Poco::JSON::Parser	P;
                    ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                } else {
                    Poco::JSON::Parser	P;
                    ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                }
                return Response.getStatus();
            }
        }
        catch (const Poco::Exception &E)
        {
            std::cerr << E.displayText() << std::endl;
        }
        return -1;
    }

    int OpenAPIRequestPost::Do(Poco::JSON::Object::Ptr &ResponseObject) {
        try {
            auto Services = MicroService::instance().GetServices(Type_);
            for(auto const &Svc:Services) {
                Poco::URI	URI(Svc.PrivateEndPoint);
                Poco::Net::HTTPSClientSession Session(URI.getHost(), URI.getPort());

                URI.setPath(EndPoint_);
                for (const auto &qp : QueryData_)
                    URI.addQueryParameter(qp.first, qp.second);

                std::string Path(URI.getPathAndQuery());
                Session.setTimeout(Poco::Timespan(msTimeout_/1000, msTimeout_ % 1000));

                Poco::Net::HTTPRequest Request(Poco::Net::HTTPRequest::HTTP_POST,
                                               Path,
                                               Poco::Net::HTTPMessage::HTTP_1_1);
                std::ostringstream obody;
                Poco::JSON::Stringifier::stringify(Body_,obody);

                Request.setContentType("application/json");
                Request.setContentLength(obody.str().size());

                Request.add("X-API-KEY", Svc.AccessKey);
                Request.add("X-INTERNAL-NAME", MicroService::instance().PublicEndPoint());

                std::ostream & os = Session.sendRequest(Request);
                os << obody.str();

                Poco::Net::HTTPResponse Response;
                std::istream &is = Session.receiveResponse(Response);
                if(Response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
                    Poco::JSON::Parser	P;
                    ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                } else {
                    Poco::JSON::Parser	P;
                    ResponseObject = P.parse(is).extract<Poco::JSON::Object::Ptr>();
                }
                return Response.getStatus();
            }
        }
        catch (const Poco::Exception &E)
        {
            std::cerr << E.displayText() << std::endl;
        }
        return -1;
    }


    inline void RESTAPI_GenericServer::InitLogging() {
        std::string Public = MicroService::instance().ConfigGetString("apilogging.public.methods","PUT,POST,DELETE");
        SetFlags(true, Public);
        std::string Private = MicroService::instance().ConfigGetString("apilogging.private.methods","PUT,POST,DELETE");
        SetFlags(false, Private);

        std::string PublicBadTokens = MicroService::instance().ConfigGetString("apilogging.public.badtokens.methods","");
        LogBadTokens_[0] = (Poco::icompare(PublicBadTokens,"true")==0);
        std::string PrivateBadTokens = MicroService::instance().ConfigGetString("apilogging.private.badtokens.methods","");
        LogBadTokens_[1] = (Poco::icompare(PrivateBadTokens,"true")==0);
    }

#ifdef    TIP_SECURITY_SERVICE
    [[nodiscard]] bool AuthServiceIsAuthorized(Poco::Net::HTTPServerRequest & Request,std::string &SessionToken, SecurityObjects::UserInfoAndPolicy & UInfo, bool & Expired );
#endif
    inline bool RESTAPIHandler::IsAuthorized( bool & Expired ) {
        if(Internal_) {
            auto Allowed = MicroService::instance().IsValidAPIKEY(*Request);
            if(!Allowed) {
                if(Server_.LogBadTokens(false)) {
                    Logger_.debug(Poco::format("I-REQ-DENIED(%s): Method='%s' Path='%s",
                                               Utils::FormatIPv6(Request->clientAddress().toString()),
                                               Request->getMethod(), Request->getURI()));
                }
            } else {
                auto Id = Request->get("X-INTERNAL-NAME", "unknown");
                if(Server_.LogIt(Request->getMethod(),true)) {
                    Logger_.debug(Poco::format("I-REQ-ALLOWED(%s): User='%s' Method='%s' Path='%s",
                                               Utils::FormatIPv6(Request->clientAddress().toString()), Id,
                                               Request->getMethod(), Request->getURI()));
                }
            }
            return Allowed;
        } else {
            if (SessionToken_.empty()) {
                try {
                    Poco::Net::OAuth20Credentials Auth(*Request);
                    if (Auth.getScheme() == "Bearer") {
                        SessionToken_ = Auth.getBearerToken();
                    }
                } catch (const Poco::Exception &E) {
                    Logger_.log(E);
                }
            }
#ifdef    TIP_SECURITY_SERVICE
            if (AuthServiceIsAuthorized(*Request, SessionToken_, UserInfo_, Expired)) {
#else
            if (AuthClient()->IsAuthorized( SessionToken_, UserInfo_, Expired)) {
#endif
                if(Server_.LogIt(Request->getMethod(),true)) {
                    Logger_.debug(Poco::format("X-REQ-ALLOWED(%s): User='%s@%s' Method='%s' Path='%s",
                                               UserInfo_.userinfo.email,
                                               Utils::FormatIPv6(Request->clientAddress().toString()),
                                               Request->clientAddress().toString(),
                                               Request->getMethod(),
                                               Request->getURI()));
                }
                return true;
            } else {
                if(Server_.LogBadTokens(true)) {
                    Logger_.debug(Poco::format("X-REQ-DENIED(%s): Method='%s' Path='%s",
                                               Utils::FormatIPv6(Request->clientAddress().toString()),
                                               Request->getMethod(), Request->getURI()));
                }
            }
            return false;
        }
    }

	inline MicroService * MicroService::instance_ = nullptr;
}

namespace OpenWifi::Utils {
	[[nodiscard]] inline uint64_t GetSystemId() {
		uint64_t ID=0;
		if(!AppServiceRegistry().Get("systemid",ID)) {
			return InitializeSystemId();
		}
		return ID;
	}
}

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
        bits_whole = bits >> 5;         // number of whole u32
        bits_incomplete = bits & 0x1F;  // number of bits in incomplete u32
        if (bits_whole) {
            if (memcmp(a, n, bits_whole << 2)!=0) {
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
        L = std::strtol(S,&end,10);
        return end != S;
    }

    static bool CidrIPinRange(const Poco::Net::IPAddress &IP, const std::string &Range) {
        Poco::StringTokenizer	TimeTokens(Range,"/",Poco::StringTokenizer::TOK_TRIM);

        Poco::Net::IPAddress	RangeIP;
        if(Poco::Net::IPAddress::tryParse(TimeTokens[0],RangeIP)) {
            if(TimeTokens.count()==2) {
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

        auto Tokens = Poco::StringTokenizer(R,"-");
        if(Tokens.count()==2) {
            Poco::Net::IPAddress    a,b;
            if(!Poco::Net::IPAddress::tryParse(Tokens[0],a) && Poco::Net::IPAddress::tryParse(Tokens[1],b))
                return false;
            return a.family() == b.family();
        }

        Tokens = Poco::StringTokenizer(R,",");
        if(Tokens.count()>1) {
            return std::all_of(Tokens.begin(), Tokens.end(), [](const std::string &A) {
                Poco::Net::IPAddress    a;
                return Poco::Net::IPAddress::tryParse(A,a);
            } );
        }

        Tokens = Poco::StringTokenizer(R,"/");
        if(Tokens.count()==2) {
            Poco::Net::IPAddress    a;
            if(!Poco::Net::IPAddress::tryParse(Tokens[0],a))
                return false;
            if(std::atoi(Tokens[1].c_str())==0)
                return false;
            return true;
        }

        Poco::Net::IPAddress    a;
        return Poco::Net::IPAddress::tryParse(R,a);
    }

    static bool IpInRange(const Poco::Net::IPAddress & target, const std::string & R) {

        auto Tokens = Poco::StringTokenizer(R,"-");
        if(Tokens.count()==2) {
            auto    a = Poco::Net::IPAddress::parse(Tokens[0]);
            auto    b = Poco::Net::IPAddress::parse(Tokens[1]);
            if(target.family() != a.family())
                return false;
            return (a<=target && b>=target);
        }

        Tokens = Poco::StringTokenizer(R,",");
        if(Tokens.count()>1) {
            return std::any_of(Tokens.begin(), Tokens.end(), [target](const std::string &Element) {
                return Poco::Net::IPAddress::parse(Element) == target ; });
        }

        Tokens = Poco::StringTokenizer(R,"/");
        if(Tokens.count()==2) {
            return CidrIPinRange(target,R);
        }

        return Poco::Net::IPAddress::parse(R)==target;
    }

    [[nodiscard]] inline bool IpInRanges(const std::string &IP, const Types::StringVec &R) {
        Poco::Net::IPAddress    Target;

        if(!Poco::Net::IPAddress::tryParse(IP,Target))
            return false;

        return std::any_of(cbegin(R),cend(R),[Target](const std::string &i) { return IpInRange(Target,i); });
    }

    [[nodiscard]] inline bool ValidateIpRanges(const Types::StringVec & Ranges) {
        return std::all_of(cbegin(Ranges), cend(Ranges), ValidateRange);
    }
}
