//
// Created by stephane bourque on 2021-05-06.
//
#include "uStorageService.h"
#include "uCentral.h"

namespace uCentral::Storage {

#ifdef SMALL_BUILD
	int Service::Setup_PostgreSQL() { uCentral::instance()->exit(Poco::Util::Application::EXIT_CONFIG);}
#else
	int Service::Setup_PostgreSQL() {
		Logger_.notice("PostgreSQL Storage enabled.");

		IsPSQL_ = true ;

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

		PostgresConn_ = std::make_unique<Poco::Data::PostgreSQL::Connector>();
		PostgresConn_->registerConnector();
		Pool_ = std::make_unique<Poco::Data::SessionPool>(PostgresConn_->name(), ConnectionStr, 4, NumSessions, IdleTime);
		Poco::Data::Session Sess = Pool_->get();

		Sess << "CREATE TABLE IF NOT EXISTS Statistics ("
				"SerialNumber VARCHAR(30), "
				"UUID INTEGER, "
				"Data TEXT, "
				"Recorded BIGINT)", Poco::Data::Keywords::now;

		Sess << "CREATE INDEX IF NOT EXISTS StatsSerial ON Statistics (SerialNumber ASC, Recorded ASC)", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS Devices ("
				"SerialNumber  VARCHAR(30) UNIQUE PRIMARY KEY, "
				"DeviceType    VARCHAR(32), "
				"MACAddress    VARCHAR(30), "
				"Manufacturer  VARCHAR(64), "
				"UUID          BIGINT, "
				"Configuration TEXT, "
				"Notes         TEXT, "
				"CreationTimestamp BIGINT, "
				"LastConfigurationChange BIGINT, "
				"LastConfigurationDownload BIGINT, "
				"Owner  VARCHAR(64), 	"
				"Location  VARCHAR(64), 	"
				"Firmware VARCHAR(128)"
				")", Poco::Data::Keywords::now;
		Sess << "CREATE INDEX IF NOT EXISTS DeviceOwner ON Devices (Owner ASC)", Poco::Data::Keywords::now;
		Sess << "CREATE INDEX IF NOT EXISTS DeviceLocation ON Devices (Location ASC)", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS Capabilities ("
				"SerialNumber VARCHAR(30) PRIMARY KEY, "
				"Capabilities TEXT, "
				"FirstUpdate BIGINT, "
				"LastUpdate BIGINT"
				")", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS DeviceLogs ("
				"SerialNumber   VARCHAR(30), "
				"Log            TEXT, "
				"Data           TEXT, "
				"Severity       BIGINT, "
				"Recorded       BIGINT, "
				"LogType        BIGINT"
				")", Poco::Data::Keywords::now;

		Sess << "CREATE INDEX IF NOT EXISTS LogSerial ON DeviceLogs (SerialNumber ASC, Recorded ASC)", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS HealthChecks ("
				"SerialNumber VARCHAR(30), "
				"UUID          BIGINT, "
				"Data TEXT, "
				"Sanity BIGINT , "
				"Recorded BIGINT)", Poco::Data::Keywords::now;

		Sess << "CREATE INDEX IF NOT EXISTS HealthSerial ON HealthChecks (SerialNumber ASC, Recorded ASC)", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS DefaultConfigs ("
				"Name VARCHAR(30) PRIMARY KEY, "
				"Configuration TEXT, "
				"Models TEXT, "
				"Description TEXT, "
				"Created BIGINT , "
				"LastModified BIGINT)", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS CommandList ("
				"UUID           VARCHAR(64) PRIMARY KEY, "
				"SerialNumber   VARCHAR(30), "
				"Command        VARCHAR(32), "
				"Status         VARCHAR(64), "
				"SubmittedBy    VARCHAR(64), "
				"Results        TEXT, "
				"Details        TEXT, "
				"ErrorText      TEXT, "
				"Submitted      BIGINT, "
				"Executed       BIGINT, "
				"Completed      BIGINT, "
				"RunAt          BIGINT, "
				"ErrorCode      BIGINT, "
				"Custom         BIGINT, "
				"WaitingForFile BIGINT, "
				"AttachDate     BIGINT, "
				"AttachSize     BIGINT, "
				"AttachType     VARCHAR(64)"
				")", Poco::Data::Keywords::now;

		Sess << "CREATE INDEX IF NOT EXISTS CommandListIndex ON CommandList (SerialNumber ASC, Submitted ASC)", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS BlackList ("
				"SerialNumber	VARCHAR(30) PRIMARY KEY, "
				"Reason			TEXT, "
				"Created		BIGINT, "
				"Author			VARCHAR(64)"
				")", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS FileUploads ("
				"UUID			VARCHAR(64) PRIMARY KEY, "
				"Type			VARCHAR(32), "
				"Created 		BIGINT, "
				"FileContent	BYTEA"
				") ", Poco::Data::Keywords::now;

		Sess << "CREATE TABLE IF NOT EXISTS Authentication ("
				"Identity			VARCHAR(128) PRIMARY KEY, "
				"Password			VARCHAR(128), "
				"AccessType			INT, "
				"Created			BIGINT, "
				"Modified			BIGINT, "
				"Expires 			BIGINT, "
				"CreatedBy			VARCHAR(128), "
				"ACLRead 			INT, "
				"ACLReadWrite 		INT, "
				"ACLReadWriteCreate INT, "
				"ACLDelete 			INT, "
				"ACLPortal 			INT "
				") ", Poco::Data::Keywords::now;

		return 0;
	}
#endif

}