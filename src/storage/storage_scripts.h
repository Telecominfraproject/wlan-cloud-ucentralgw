//
// Created by stephane bourque on 2022-11-21.
//

#pragma once

#include "RESTObjects/RESTAPI_GWobjects.h"
#include "framework/orm.h"

namespace OpenWifi {

	typedef Poco::Tuple<std::string, //  id
						std::string, //  name
						std::string, //  description
						std::string, //  uri
						std::string, //  content
						std::string, //  version
						std::string, //  type
						uint64_t,	 //  created = 0;
						uint64_t,	 //  modified = 0;
						std::string, //  author
						std::string, //  restricted
						bool,		 //	deferred
						uint64_t,	 //	timeout
						std::string>
		ScriptRecordTuple;
	typedef std::vector<ScriptRecordTuple> ScriptRecordTupleList;

	class ScriptDB : public ORM::DB<ScriptRecordTuple, GWObjects::ScriptEntry> {
	  public:
		ScriptDB(const std::string &name, const std::string &shortname, OpenWifi::DBType T,
				 Poco::Data::SessionPool &P, Poco::Logger &L);
		virtual ~ScriptDB() {}
		inline uint32_t Version() override { return 1; }

		bool Upgrade(uint32_t from, uint32_t &to) override;
		void Initialize();
	};

} // namespace OpenWifi