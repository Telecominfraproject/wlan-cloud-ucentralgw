//
// Created by stephane bourque on 2021-06-13.
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
        std::string Result;

        bool first=true;
        for(const auto &i:V) {
            if(first) {
                Result += i;
                first = false;
            } else {
                Result += ",";
                Result += i;
            }
        }
        return Result;
    }

    inline void from_string(const std::string &S, StringVec &V) {
        Poco::StringTokenizer   Tokens(S,",",Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);

        for(auto const &i:Tokens)
            V.emplace_back(i);
    }
};

#endif // UCENTRALGW_UCENTRALTYPES_H
