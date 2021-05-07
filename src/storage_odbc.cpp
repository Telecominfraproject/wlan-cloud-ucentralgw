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

#ifdef SMALL_BUILD
	int Service::Setup_ODBC() { uCentral::instance()->exit(Poco::Util::Application::EXIT_CONFIG);}
#else
	int Service::Setup_ODBC() {

		dbType_ = odbc ;

		Logger_.notice("ODBC Storage enabled.");

		auto NumSessions = uCentral::ServiceConfig::GetInt("storage.type.postgresql.maxsessions", 64);
		auto IdleTime = uCentral::ServiceConfig::GetInt("storage.type.postgresql.idletime", 60);
		auto Host = uCentral::ServiceConfig::GetString("storage.type.postgresql.host");
		auto Username = uCentral::ServiceConfig::GetString("storage.type.postgresql.username");
		auto Password = uCentral::ServiceConfig::GetString("storage.type.postgresql.password");
		auto Database = uCentral::ServiceConfig::GetString("storage.type.postgresql.database");
		auto Port = uCentral::ServiceConfig::GetString("storage.type.postgresql.port");
		auto ConnectionTimeout = uCentral::ServiceConfig::GetString("storage.type.postgresql.connectiontimeout");

		std::string ConnectionStr =
			"host=" + Host +
			" user=" + Username +
			" password=" + Password +
			" dbname=" + Database +
			" port=" + Port +
			" connect_timeout=" + ConnectionTimeout;

		ODBCConn_ = std::make_unique<Poco::Data::ODBC::Connector>();
		ODBCConn_->registerConnector();
		Pool_ = std::make_unique<Poco::Data::SessionPool>(ODBCConn_->name(), ConnectionStr, 4, NumSessions, IdleTime);

		return 0;
	}
#endif
}