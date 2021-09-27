//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_UCENTRALTYPES_H
#define UCENTRALGW_UCENTRALTYPES_H

#include "SubSystemServer.h"

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <list>
#include <utility>
#include <queue>

#include "Poco/StringTokenizer.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

namespace OpenWifi::Types {
    typedef std::pair<std::string,std::string>              StringPair;
	typedef std::vector<StringPair>	                        StringPairVec;
    typedef std::queue<StringPair>	                        StringPairQueue;
	typedef std::vector<std::string>						StringVec;
	typedef std::set<std::string>                           StringSet;
	typedef std::vector<SubSystemServer*>					SubSystemVec;
	typedef std::map<std::string,std::set<std::string>>		StringMapStringSet;
	typedef std::function<void(std::string, std::string)>   TopicNotifyFunction;
	typedef std::list<std::pair<TopicNotifyFunction,int>>   TopicNotifyFunctionList;
	typedef std::map<std::string, TopicNotifyFunctionList>  NotifyTable;
    typedef std::map<std::string,uint64_t>                  CountedMap;

    typedef std::string         UUID_t;
    typedef std::vector<UUID_t> UUIDvec_t;

    inline void UpdateCountedMap(CountedMap &M, const std::string &S, uint64_t Increment=1) {
        auto it = M.find(S);
        if(it==M.end())
            M[S] = Increment;
        else
            it->second += Increment;
    }

    inline std::string to_string( const StringVec &V) {
        Poco::JSON::Array   O;
        for(const auto &i:V) {
            O.add(i);
        }
        std::stringstream SS;
        Poco::JSON::Stringifier::stringify(O,SS);
        return SS.str();
    }

    inline std::string to_string( const StringPairVec &V) {
        Poco::JSON::Array   O;
        for(const auto &i:V) {
            Poco::JSON::Array OO;
            OO.add(i.first);
            OO.add(i.second);
            O.add(OO);
        }

        std::stringstream SS;
        Poco::JSON::Stringifier::stringify(O,SS);
        return SS.str();
    }

    inline void from_string(const std::string &S, StringPairVec &V) {
        try {
            Poco::JSON::Parser      P;
            auto O = P.parse(S).extract<Poco::JSON::Array::Ptr>();

            for(const auto &i:*O) {
                auto Inner = i.extract<Poco::JSON::Array::Ptr>();
                for(const auto &j:*Inner) {
                    auto S1 = i[0].toString();
                    auto S2 = i[1].toString();
                    V.push_back(std::make_pair(S1,S2));
                }
            }
        } catch (...) {

        }
    }

    inline void from_string(const std::string &S, StringVec &V) {
        try {
            Poco::JSON::Parser      P;
            auto O = P.parse(S).extract<Poco::JSON::Array::Ptr>();

            for(auto const &i:*O) {
                V.push_back(i.toString());
            }
        } catch (...) {

        }
    }
};

#endif // UCENTRALGW_UCENTRALTYPES_H
