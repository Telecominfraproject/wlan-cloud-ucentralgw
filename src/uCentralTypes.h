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

namespace uCentral::Types {
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

    inline void UpdateCountedMap(CountedMap &M, const std::string &S ) {
        auto it = M.find(S);
        if(it==M.end())
            M[S]=1;
        else
            it->second += 1;
    }
};

#endif // UCENTRALGW_UCENTRALTYPES_H
