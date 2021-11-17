//
// Created by stephane bourque on 2021-11-16.
//

#pragma once

#include <map>
#include <utility>
#include <vector>
#include <functional>
#include <string>
#include <queue>
#include <list>
#include <set>

namespace OpenWifi::Types {
    typedef std::pair<std::string,std::string>              StringPair;
    typedef std::vector<StringPair>	                        StringPairVec;
    typedef std::queue<StringPair>	                        StringPairQueue;
    typedef std::vector<std::string>						StringVec;
    typedef std::set<std::string>                           StringSet;
    typedef std::map<std::string,std::set<std::string>>		StringMapStringSet;
    typedef std::function<void(std::string, std::string)>   TopicNotifyFunction;
    typedef std::list<std::pair<TopicNotifyFunction,int>>   TopicNotifyFunctionList;
    typedef std::map<std::string, TopicNotifyFunctionList>  NotifyTable;
    typedef std::map<std::string,uint64_t>                  CountedMap;
    typedef std::vector<uint64_t>                           TagList;
    typedef std::string                                     UUID_t;
    typedef std::vector<UUID_t>                             UUIDvec_t;
}

namespace OpenWifi {
    inline void UpdateCountedMap(OpenWifi::Types::CountedMap &M, const std::string &S, uint64_t Increment=1) {
        auto it = M.find(S);
        if(it==M.end())
            M[S] = Increment;
        else
            it->second += Increment;
    }
}
