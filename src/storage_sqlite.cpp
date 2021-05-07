//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "uStorageService.h"
#include "uCentral.h"

namespace uCentral::Storage {
	int Service::Setup_SQLite() {
		Logger_.notice("SQLite Storage enabled.");

		auto DBName = uCentral::ServiceConfig::GetString("storage.type.sqlite.db");
		auto NumSessions = uCentral::ServiceConfig::GetInt("storage.type.sqlite.maxsessions", 64);
		auto IdleTime = uCentral::ServiceConfig::GetInt("storage.type.sqlite.idletime", 60);

		SQLiteConn_ = std::make_unique<Poco::Data::SQLite::Connector>();
		SQLiteConn_->registerConnector();
		Pool_ = std::make_unique<Poco::Data::SessionPool>(SQLiteConn_->name(), DBName, 4, NumSessions, IdleTime);

		return 0;
	}
}