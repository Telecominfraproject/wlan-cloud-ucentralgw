//
// Created by stephane bourque on 2021-06-13.
//

#ifndef UCENTRALGW_UCENTRALTYPES_H
#define UCENTRALGW_UCENTRALTYPES_H

#include "SubSystemServer.h"

namespace uCentral::Types {
	typedef std::vector<std::pair<std::string,std::string>>	StringPairVec;
	typedef std::vector<std::string>						StringVec;
	typedef std::vector<SubSystemServer*>					SubSystemVec;
	typedef std::map<std::string,std::set<std::string>>		StringMapStringSet;
	typedef std::function<void(std::string,std::string)>    TopicNotifyFunction;
	typedef std::list<std::pair<TopicNotifyFunction,int>>   TopicNotifyFunctionList;
	typedef std::map<std::string, TopicNotifyFunctionList>  NotifyTable;
};

#endif // UCENTRALGW_UCENTRALTYPES_H
