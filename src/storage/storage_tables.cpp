//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "StorageService.h"

namespace OpenWifi {

	int Storage::Create_Tables() {

		Create_Statistics();
		Create_Devices();
		Create_Capabilities();
		Create_HealthChecks();
		Create_DeviceLogs();
		Create_DefaultConfigs();
		Create_CommandList();
		Create_BlackList();
		Create_FileUploads();

		return 0;
	}

	int Storage::Create_Statistics() {
		try {
			Poco::Data::Session Sess = Pool_->get();

			if (dbType_ == pgsql || dbType_ == sqlite) {
				Sess << "CREATE TABLE IF NOT EXISTS Statistics ("
						"SerialNumber VARCHAR(30), "
						"UUID INTEGER, "
						"Data TEXT, "
						"Recorded BIGINT)",
					Poco::Data::Keywords::now;
				Sess << "CREATE INDEX IF NOT EXISTS StatsSerial ON Statistics (SerialNumber ASC, "
						"Recorded ASC)",
					Poco::Data::Keywords::now;
			} else if (dbType_ == mysql) {
				Sess << "CREATE TABLE IF NOT EXISTS Statistics ("
						"SerialNumber VARCHAR(30), "
						"UUID INTEGER, "
						"Data TEXT, "
						"Recorded BIGINT, "
						"INDEX StatSerial (SerialNumber ASC, Recorded ASC))",
					Poco::Data::Keywords::now;
			}
			return 0;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return -1;
	}

	int Storage::Create_Devices() {
		try {
			Poco::Data::Session Sess = Pool_->get();

			if (dbType_ == mysql) {
				Sess << "CREATE TABLE IF NOT EXISTS Devices ("
						"SerialNumber  	VARCHAR(30) UNIQUE PRIMARY KEY, "
						"DeviceType    	VARCHAR(32), "
						"MACAddress    	VARCHAR(30), "
						"Manufacturer  	VARCHAR(64), "
						"Configuration 	TEXT, "
						"Notes         	TEXT, "
						"Owner  		VARCHAR(64), "
						"Location  		TEXT, "
						"Venue  		VARCHAR(64), "
						"DevicePassword VARCHAR(64), "
						"Firmware 		VARCHAR(128),"
						"Compatible		VARCHAR(128),"
						"FWUpdatePolicy	VARCHAR(128),"
						"UUID          	BIGINT,      "
						"CreationTimestamp BIGINT,   "
						"LastConfigurationChange BIGINT, "
						"LastConfigurationDownload BIGINT, "
						"LastFWUpdate	BIGINT,		"
						"subscriber 	VARCHAR(64), "
						"entity     	VARCHAR(64), "
						"modified		BIGINT,"
						"locale 		varchar(32), "
						"restrictedDevice BOOLEAN, "
						"pendingConfiguration TEXT, "
						"pendingConfigurationCmd VARCHAR(64), "
						"restrictionDetails TEXT, "
						"pendingUUID	BIGINT "
						",INDEX DeviceOwner (Owner ASC),"
						"INDEX LocationIndex (Location ASC))",
					Poco::Data::Keywords::now;
			} else if (dbType_ == sqlite || dbType_ == pgsql) {
				Sess << "CREATE TABLE IF NOT EXISTS Devices ("
						"SerialNumber  	VARCHAR(30) UNIQUE PRIMARY KEY, "
						"DeviceType    	VARCHAR(32), "
						"MACAddress    	VARCHAR(30), "
						"Manufacturer  	VARCHAR(64), "
						"Configuration 	TEXT, "
						"Notes         	TEXT, "
						"Owner  		VARCHAR(64), "
						"Location  		TEXT, "
						"Venue  		VARCHAR(64), "
						"DevicePassword VARCHAR(64), "
						"Firmware 		VARCHAR(128),"
						"Compatible		VARCHAR(128),"
						"FWUpdatePolicy	VARCHAR(128),"
						"UUID          	BIGINT,      "
						"CreationTimestamp BIGINT,   "
						"LastConfigurationChange BIGINT, "
						"LastConfigurationDownload BIGINT, "
						"LastFWUpdate	BIGINT		,"
						"subscriber 	VARCHAR(64) , "
						"entity     	VARCHAR(64) , "
						"modified		BIGINT,"
						"locale 		varchar(32), "
						"restrictedDevice BOOLEAN, "
						"pendingConfiguration TEXT,"
						"pendingConfigurationCmd VARCHAR(64), "
						"restrictionDetails TEXT,"
						"pendingUUID 	BIGINT "
						")",
					Poco::Data::Keywords::now;
				Sess << "CREATE INDEX IF NOT EXISTS DeviceOwner ON Devices (Owner ASC)",
					Poco::Data::Keywords::now;
				Sess << "CREATE INDEX IF NOT EXISTS DeviceLocation ON Devices (Location ASC)",
					Poco::Data::Keywords::now;
			}

			// we must upgrade old DBs
			std::vector<std::string> Script{
				"alter table devices add column subscriber varchar(64)",
				"alter table devices add column entity varchar(64)",
				"alter table devices add column modified bigint",
				"alter table devices add column locale varchar(32)",
				"alter table devices add column restrictedDevice boolean",
				"alter table devices add column pendingConfiguration TEXT",
				"alter table devices add column pendingConfigurationCmd VARCHAR(64)",
				"alter table devices add column restrictionDetails TEXT",
				"alter table devices add column pendingUUID bigint"};

			for (const auto &i : Script) {
				try {
					Sess << i, Poco::Data::Keywords::now;
				} catch (...) {
				}
			}
			return 0;

		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return -1;
	}

	int Storage::Create_Capabilities() {
		try {
			Poco::Data::Session Sess = Pool_->get();

			if (dbType_ == pgsql || dbType_ == sqlite || dbType_ == mysql) {
				Sess << "CREATE TABLE IF NOT EXISTS Capabilities ("
						"SerialNumber VARCHAR(30) PRIMARY KEY, "
						"Capabilities TEXT, "
						"FirstUpdate BIGINT, "
						"LastUpdate BIGINT"
						")",
					Poco::Data::Keywords::now;
			}

			return 0;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return -1;
	}

	int Storage::Create_HealthChecks() {
		try {
			Poco::Data::Session Sess = Pool_->get();

			if (dbType_ == mysql) {
				Sess << "CREATE TABLE IF NOT EXISTS HealthChecks ("
						"SerialNumber VARCHAR(30), "
						"UUID          BIGINT, "
						"Data TEXT, "
						"Sanity BIGINT , "
						"Recorded BIGINT, "
						"INDEX HealthSerial (SerialNumber ASC, Recorded ASC)"
						")",
					Poco::Data::Keywords::now;
			} else if (dbType_ == sqlite || dbType_ == pgsql) {
				Sess << "CREATE TABLE IF NOT EXISTS HealthChecks ("
						"SerialNumber VARCHAR(30), "
						"UUID          BIGINT, "
						"Data TEXT, "
						"Sanity BIGINT , "
						"Recorded BIGINT) ",
					Poco::Data::Keywords::now;
				Sess << "CREATE INDEX IF NOT EXISTS HealthSerial ON HealthChecks (SerialNumber "
						"ASC, Recorded ASC)",
					Poco::Data::Keywords::now;
			}
			return 0;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return -1;
	}

	int Storage::Create_DeviceLogs() {
		try {
			Poco::Data::Session Sess = Pool_->get();

			if (dbType_ == mysql) {
				Sess << "CREATE TABLE IF NOT EXISTS DeviceLogs ("
						"SerialNumber   VARCHAR(30), "
						"Log            TEXT, "
						"Data           TEXT, "
						"Severity       BIGINT, "
						"Recorded       BIGINT, "
						"LogType        BIGINT, "
						"UUID	        BIGINT, "
						"INDEX LogSerial (SerialNumber ASC, Recorded ASC)"
						")",
					Poco::Data::Keywords::now;
			} else if (dbType_ == pgsql || dbType_ == sqlite) {
				Sess << "CREATE TABLE IF NOT EXISTS DeviceLogs ("
						"SerialNumber   VARCHAR(30), "
						"Log            TEXT, "
						"Data           TEXT, "
						"Severity       BIGINT, "
						"Recorded       BIGINT, "
						"LogType        BIGINT, "
						"UUID	        BIGINT  "
						")",
					Poco::Data::Keywords::now;
				Sess << "CREATE INDEX IF NOT EXISTS LogSerial ON DeviceLogs (SerialNumber ASC, "
						"Recorded ASC)",
					Poco::Data::Keywords::now;
			}

			return 0;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return -1;
	}

	int Storage::Create_DefaultConfigs() {
		try {
			Poco::Data::Session Sess = Pool_->get();

			if (dbType_ == pgsql || dbType_ == sqlite || dbType_ == mysql) {
				Sess << "CREATE TABLE IF NOT EXISTS DefaultConfigs ("
						"Name VARCHAR(30) PRIMARY KEY, "
						"Configuration TEXT, "
						"Models TEXT, "
						"Description TEXT, "
						"Created BIGINT , "
						"LastModified BIGINT)",
					Poco::Data::Keywords::now;
			}
			return 0;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return -1;
	}

	// mysql = float
	// sqlite, postgresql = real

	int Storage::Create_CommandList() {
		try {
			Poco::Data::Session Sess = Pool_->get();
			if (dbType_ == mysql) {
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
						"AttachDate     BIGINT,"
						"AttachSize     BIGINT,"
						"AttachType     VARCHAR(64),"
						"INDEX CommandListIndex (SerialNumber ASC, Submitted ASC)"
						")",
					Poco::Data::Keywords::now;
			} else if (dbType_ == pgsql || dbType_ == sqlite) {
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
						"AttachDate     BIGINT,"
						"AttachSize     BIGINT,"
						"AttachType     VARCHAR(64)"
						")",
					Poco::Data::Keywords::now;
				Sess << "CREATE INDEX IF NOT EXISTS CommandListIndex ON CommandList (SerialNumber "
						"ASC, Submitted ASC)",
					Poco::Data::Keywords::now;
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}

		std::vector<std::string> Script{
			dbType_ == mysql ? "alter table CommandList add column executionTime float default 0.00"
							 : "alter table CommandList add column executionTime real default 0.00",
			"alter table CommandList add column LastTry bigint default 0",
			"alter table CommandList add column deferred BOOLEAN default false"};

		for (const auto &i : Script) {
			try {
				Poco::Data::Session Sess = Pool_->get();
				Sess << i, Poco::Data::Keywords::now;
			} catch (const Poco::Data::DataException &) {
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			}
		}

		return 0;
	}

	int Storage::Create_BlackList() {
		try {
			Poco::Data::Session Sess = Pool_->get();

			if (dbType_ == mysql || dbType_ == pgsql || dbType_ == sqlite) {
				Sess << "CREATE TABLE IF NOT EXISTS BlackList ("
						"SerialNumber	VARCHAR(30) PRIMARY KEY, "
						"Reason			TEXT, "
						"Created		BIGINT, "
						"Author			VARCHAR(64)"
						")",
					Poco::Data::Keywords::now;
			}
			return 0;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return -1;
	}

	int Storage::Create_FileUploads() {
		try {
			Poco::Data::Session Sess = Pool_->get();

			if (dbType_ == sqlite) {
				Sess << "CREATE TABLE IF NOT EXISTS FileUploads ("
						"UUID			VARCHAR(64) PRIMARY KEY, "
						"Type			VARCHAR(32), "
						"Created 		BIGINT, "
						"FileContent	BLOB"
						") ",
					Poco::Data::Keywords::now;
			} else if (dbType_ == mysql) {
				Sess << "CREATE TABLE IF NOT EXISTS FileUploads ("
						"UUID			VARCHAR(64) PRIMARY KEY, "
						"Type			VARCHAR(32), "
						"Created 		BIGINT, "
						"FileContent	LONGBLOB"
						") ",
					Poco::Data::Keywords::now;
			} else if (dbType_ == pgsql) {
				Sess << "CREATE TABLE IF NOT EXISTS FileUploads ("
						"UUID			VARCHAR(64) PRIMARY KEY, "
						"Type			VARCHAR(32), "
						"Created 		BIGINT, "
						"FileContent	BYTEA"
						") ",
					Poco::Data::Keywords::now;
			}

			return 0;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		}
		return -1;
	}

} // namespace OpenWifi