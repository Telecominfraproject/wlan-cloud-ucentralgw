//
// Created by stephane bourque on 2022-11-21.
//

#include "storage_scripts.h"

#include "framework/RESTAPI_utils.h"
#include "fmt/format.h"
#include "framework/AppServiceRegistry.h"

namespace OpenWifi {
	static ORM::FieldVec ScriptDB_Fields{
		ORM::Field{"id", 36, true},
		ORM::Field{"name", ORM::FieldType::FT_TEXT},
		ORM::Field{"description", ORM::FieldType::FT_TEXT},
		ORM::Field{"uri", ORM::FieldType::FT_TEXT},
		ORM::Field{"content", ORM::FieldType::FT_TEXT},
		ORM::Field{"version", ORM::FieldType::FT_TEXT},
		ORM::Field{"type", ORM::FieldType::FT_TEXT},
		ORM::Field{"created", ORM::FieldType::FT_BIGINT},
		ORM::Field{"modified", ORM::FieldType::FT_BIGINT},
		ORM::Field{"author", ORM::FieldType::FT_TEXT},
		ORM::Field{"restricted", ORM::FieldType::FT_TEXT}
	};

	static ORM::IndexVec MakeIndices(const std::string & shortname) {
		return ORM::IndexVec{
			{std::string(shortname + "_name_index"),
			 ORM::IndexEntryVec{
				 {
					 std::string("name"),
					 ORM::Indextype::ASC }}}
		};
	};

	ScriptDB::ScriptDB( const std::string &TableName, const std::string &Shortname ,OpenWifi::DBType T,
					   Poco::Data::SessionPool &P, Poco::Logger &L) :
		  DB(T, TableName.c_str(), ScriptDB_Fields, MakeIndices(Shortname), P, L, Shortname.c_str()) {
	}

	bool ScriptDB::Upgrade([[maybe_unused]] uint32_t from, uint32_t &to) {
		to = Version();
		std::vector<std::string>    Script{
		};

		for(const auto &i:Script) {
			try {
				auto Session = Pool_.get();
				Session << i , Poco::Data::Keywords::now;
			} catch (...) {

			}
		}
		return true;
	}

	void ScriptDB::Initialize() {
		bool Initialized = false;
		AppServiceRegistry().Get("script_db_initialized",Initialized);
		if(Initialized)
			return;
		GWObjects::ScriptEntry	SE;

		SE.id = MicroServiceCreateUUID();
		SE.name = "ASB Bundle";
		SE.description = "Automated on-demand detailed debugging information collection from an AP";
		SE.uri = "https://telecominfraproject.atlassian.net/wiki/spaces/WIFI/pages/1678147585/AP+Support+Bundle+ASB";
		SE.version = "1.0";
		SE.author = "blogic";
		SE.modified = SE.created = Utils::Now();
		SE.type = "bundle";
		SE.content = std::string{
R"lit(
bundle.wifi();
let paths = [
        [ 'network.wireless', 'status' ],
        [ 'network.device', 'status' ],
        [ 'network.interface', 'dump' ],
        [ 'log', 'read', { stream: false } ],
];
for (let path in paths)
        bundle.ubus(path[0], path[1], path[2]);
for (let config in [ 'network', 'wireless', 'dhcp', 'firewall', 'system' ])
        bundle.uci(config);
for (let cmd in [ "route", "ifconfig", "logread" ])
        bundle.shell(cmd);
)lit"};

		if(CreateRecord(SE)) {
			AppServiceRegistry().Set("script_db_initialized",true);
			poco_information(Logger(), "Script DB has been in initialized.");
			return;
		}
		poco_error(Logger(), "Script DB could not be initialized.");
	}

} // OpenWifi

template<> void ORM::DB<OpenWifi::ScriptRecordTuple, OpenWifi::GWObjects::ScriptEntry>::Convert(const OpenWifi::ScriptRecordTuple &In, OpenWifi::GWObjects::ScriptEntry &Out) {
	Out.id = In.get<0>();
	Out.name = In.get<1>();
	Out.description = In.get<2>();
	Out.uri = In.get<3>();
	Out.content = In.get<4>();
	Out.version = In.get<5>();
	Out.type = In.get<6>();
	Out.created = In.get<7>();
	Out.modified = In.get<8>();
	Out.author = In.get<9>();
	Out.restricted = OpenWifi::RESTAPI_utils::to_object_array(In.get<10>());
}

template<> void ORM::DB<OpenWifi::ScriptRecordTuple, OpenWifi::GWObjects::ScriptEntry>::Convert(const OpenWifi::GWObjects::ScriptEntry &In, OpenWifi::ScriptRecordTuple &Out) {
	Out.set<0>(In.id);
	Out.set<1>(In.name);
	Out.set<2>(In.description);
	Out.set<3>(In.uri);
	Out.set<4>(In.content);
	Out.set<5>(In.version);
	Out.set<6>(In.type);
	Out.set<7>(In.created);
	Out.set<8>(In.modified);
	Out.set<9>(In.author);
	Out.set<10>(OpenWifi::RESTAPI_utils::to_string(In.restricted));
}
