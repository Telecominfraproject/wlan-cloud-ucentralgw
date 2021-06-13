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

#ifdef	SMALL_BUILD
	int Service::Setup_MySQL() { uCentral::instance()->exit(Poco::Util::Application::EXIT_CONFIG);}
#else

	int Storage::Setup_MySQL() {

		dbType_ = mysql ;

		Logger_.notice("MySQL Storage enabled.");
		auto NumSessions = Daemon()->ConfigGetInt("storage.type.mysql.maxsessions", 64);
		auto IdleTime = Daemon()->ConfigGetInt("storage.type.mysql.idletime", 60);
		auto Host = Daemon()->ConfigGetString("storage.type.mysql.host");
		auto Username = Daemon()->ConfigGetString("storage.type.mysql.username");
		auto Password = Daemon()->ConfigGetString("storage.type.mysql.password");
		auto Database = Daemon()->ConfigGetString("storage.type.mysql.database");
		auto Port = Daemon()->ConfigGetString("storage.type.mysql.port");

		std::string ConnectionStr =
			"host=" + Host +
			";user=" + Username +
			";password=" + Password +
			";db=" + Database +
			";port=" + Port +
			";compress=true;auto-reconnect=true";

		MySQLConn_ = std::make_unique<Poco::Data::MySQL::Connector>();
		MySQLConn_->registerConnector();
		Pool_ = std::make_unique<Poco::Data::SessionPool>(MySQLConn_->name(), ConnectionStr, 4, NumSessions, IdleTime);

		return 0;
	}
#endif
}