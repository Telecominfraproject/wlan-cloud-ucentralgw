//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Daemon.h"
#include "StorageService.h"

namespace uCentral {

#ifdef SMALL_BUILD
	int Service::Setup_PostgreSQL() { uCentral::instance()->exit(Poco::Util::Application::EXIT_CONFIG);}
#else
	int Storage::Setup_PostgreSQL() {
		Logger_.notice("PostgreSQL Storage enabled.");

		dbType_ = pgsql ;

		auto NumSessions = Daemon()->ConfigGetInt("storage.type.postgresql.maxsessions", 64);
		auto IdleTime = Daemon()->ConfigGetInt("storage.type.postgresql.idletime", 60);
		auto Host = Daemon()->ConfigGetString("storage.type.postgresql.host");
		auto Username = Daemon()->ConfigGetString("storage.type.postgresql.username");
		auto Password = Daemon()->ConfigGetString("storage.type.postgresql.password");
		auto Database = Daemon()->ConfigGetString("storage.type.postgresql.database");
		auto Port = Daemon()->ConfigGetString("storage.type.postgresql.port");
		auto ConnectionTimeout = Daemon()->ConfigGetString("storage.type.postgresql.connectiontimeout");

		std::string ConnectionStr =
			"host=" + Host +
			" user=" + Username +
			" password=" + Password +
			" dbname=" + Database +
			" port=" + Port +
			" connect_timeout=" + ConnectionTimeout;

		PostgresConn_ = std::make_unique<Poco::Data::PostgreSQL::Connector>();
		PostgresConn_->registerConnector();
		Pool_ = std::make_unique<Poco::Data::SessionPool>(PostgresConn_->name(), ConnectionStr, 4, NumSessions, IdleTime);

		return 0;
	}
#endif

}