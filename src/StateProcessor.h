//
// Created by stephane bourque on 2021-05-23.
//

#ifndef UCENTRALGW_STATEPROCESSOR_H
#define UCENTRALGW_STATEPROCESSOR_H

#include "Poco/JSON/Object.h"
#include "RESTObjects//RESTAPI_GWobjects.h"
#include <map>

namespace OpenWifi {
	class StateProcessor {
	  public:
		explicit StateProcessor(GWObjects::ConnectionState * Conn):
 			Conn_(Conn) {}

		~StateProcessor() {
			Save();
		}

		const Poco::JSON::Object &Get() const { return State_; }
		bool Add(const Poco::JSON::Object::Ptr &O);
		bool Add(const std::string &S);
		void Print() const;
		void to_json(Poco::JSON::Object & Obj) const;
		[[nodiscard]] std::string toString() const;
		bool Initialize(std::string & SerialNumber);
		bool Save();
		static bool GetAssociations(const Poco::JSON::Object::Ptr &Ptr, uint64_t &Radios_2G, uint64_t &Radios_5G);

	  private:
		std::string 					SerialNumber_;
		Poco::JSON::Object 	State_;
		// interface name is the key, each entry is a map with the stats name as key and then the value
		std::map<std::string,std::map<std::string,uint64_t>>	Stats_;
		uint64_t 						UpdatesSinceLastWrite_ = 0 ;
		GWObjects::ConnectionState	* 	Conn_ = nullptr;
	};

}
#endif // UCENTRALGW_STATEPROCESSOR_H
