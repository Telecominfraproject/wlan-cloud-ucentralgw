//
// Created by stephane bourque on 2021-03-01.
//
#include <iostream>
#include <fstream>

#include "uStorageService.h"
#include "uCentral.h"
#include "uCentralConfig.h"
#include "uDeviceRegistry.h"

#include "Poco/Data/RecordSet.h"
#include "Poco/DateTime.h"
#include "Poco/Util/Application.h"
#include "Poco/Data/LOB.h"
#include "Poco/Data/LOBStream.h"
#include "Poco/File.h"

namespace uCentral::Storage {

    Service *Service::instance_ = nullptr;

    typedef Poco::Tuple<
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            uint64_t,
            uint64_t,
            uint64_t,
            uint64_t,
            uint64_t,
            uint64_t,
            uint64_t,
            uint64_t> CommandDetailsRecordTuple;

    typedef Poco::Tuple<
            std::string,
            std::string,
            std::string,
            uint64_t,
            uint64_t,
            uint64_t> DeviceLogsRecordTuple;


    Service::Service() noexcept:
            SubSystemServer("Storage", "STORAGE-SVR", "storage"),
            Pool_(nullptr),
#ifdef SMALL_BUILD
            SQLiteConn_(nullptr)
#else
            SQLiteConn_(nullptr),
            PostgresConn_(nullptr),
            MySQLConn_(nullptr),
            ODBCConn_(nullptr)
#endif
    {
    }

    int Start() {
        return uCentral::Storage::Service::instance()->Start();
    }

    void Stop() {
        uCentral::Storage::Service::instance()->Stop();
    }

    bool AddLog(std::string &SerialNumber, const std::string &Log) {
        return uCentral::Storage::Service::instance()->AddLog(SerialNumber, Log);
    }

    bool AddLog(std::string &SerialNumber, uCentralDeviceLog &DeviceLog, bool CrashLog) {
        return uCentral::Storage::Service::instance()->AddLog(SerialNumber, DeviceLog, CrashLog);
    }

    bool AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats) {
        return uCentral::Storage::Service::instance()->AddStatisticsData(SerialNumber, CfgUUID, NewStats);
    }

    bool
    GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                      std::vector<uCentralStatistics> &Stats) {
        return uCentral::Storage::Service::instance()->GetStatisticsData(SerialNumber, FromDate, ToDate, Offset,
                                                                         HowMany, Stats);
    }

    bool DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        return uCentral::Storage::Service::instance()->DeleteStatisticsData(SerialNumber, FromDate, ToDate);
    }

    bool AddHealthCheckData(std::string &SerialNumber, uCentralHealthcheck &Check) {
        return uCentral::Storage::Service::instance()->AddHealthCheckData(SerialNumber, Check);
    }

    bool
    GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                       std::vector<uCentralHealthcheck> &Checks) {
        return uCentral::Storage::Service::instance()->GetHealthCheckData(SerialNumber, FromDate, ToDate, Offset,
                                                                          HowMany,
                                                                          Checks);
    }

    bool DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        return uCentral::Storage::Service::instance()->DeleteHealthCheckData(SerialNumber, FromDate, ToDate);
    }

    bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t &NewUUID) {
        return uCentral::Storage::Service::instance()->UpdateDeviceConfiguration(SerialNumber, Configuration, NewUUID);
    }

    bool CreateDevice(uCentralDevice &Device) {
        return uCentral::Storage::Service::instance()->CreateDevice(Device);
    }

    bool CreateDefaultDevice(const std::string &SerialNumber, const std::string &Capabilities) {
        return uCentral::Storage::Service::instance()->CreateDefaultDevice(SerialNumber, Capabilities);
    }

    bool GetDevice(std::string &SerialNumber, uCentralDevice &Device) {
        return uCentral::Storage::Service::instance()->GetDevice(SerialNumber, Device);
    }

    bool GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentralDevice> &Devices) {
        return uCentral::Storage::Service::instance()->GetDevices(From, HowMany, Devices);
    }

    bool DeleteDevice(std::string &SerialNumber) {
        return uCentral::Storage::Service::instance()->DeleteDevice(SerialNumber);
    }

    bool UpdateDevice(uCentralDevice &Device) {
        return uCentral::Storage::Service::instance()->UpdateDevice(Device);
    }

	bool SetOwner(std::string & SerialNumber, std::string & OwnerUUID) {
		return uCentral::Storage::Service::instance()->SetOwner(SerialNumber, OwnerUUID);
	}

	bool SetLocation(std::string & SerialNumber, std::string & LocationUUID) {
		return uCentral::Storage::Service::instance()->SetLocation(SerialNumber, LocationUUID);
	}

	bool DeviceExists(std::string &SerialNumber) {
        return uCentral::Storage::Service::instance()->DeviceExists(SerialNumber);
    }

    bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig,
                               uint64_t &NewerUUID) {
        return uCentral::Storage::Service::instance()->ExistingConfiguration(SerialNumber, CurrentConfig, NewConfig,
                                                                             NewerUUID);
    }

    bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &State) {
        return uCentral::Storage::Service::instance()->UpdateDeviceCapabilities(SerialNumber, State);
    }

    bool GetDeviceCapabilities(std::string &SerialNumber, uCentralCapabilities &Capabilities) {
        return uCentral::Storage::Service::instance()->GetDeviceCapabilities(SerialNumber, Capabilities);
    }

    bool DeleteDeviceCapabilities(std::string &SerialNumber) {
        return uCentral::Storage::Service::instance()->DeleteDeviceCapabilities(SerialNumber);
    }

    bool GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                    std::vector<uCentralDeviceLog> &Stats, uint64_t Type) {
        return uCentral::Storage::Service::instance()->GetLogData(SerialNumber, FromDate, ToDate, Offset, HowMany,
                                                                  Stats, Type );
    }

    bool DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Type) {
        return uCentral::Storage::Service::instance()->DeleteLogData(SerialNumber, FromDate, ToDate, Type);
    }

    bool CreateDefaultConfiguration(std::string &name, uCentralDefaultConfiguration &DefConfig) {
        return uCentral::Storage::Service::instance()->CreateDefaultConfiguration(name, DefConfig);
    }

    bool DeleteDefaultConfiguration(std::string &name) {
        return uCentral::Storage::Service::instance()->DeleteDefaultConfiguration(name);
    }

    bool UpdateDefaultConfiguration(std::string &name, uCentralDefaultConfiguration &DefConfig) {
        return uCentral::Storage::Service::instance()->UpdateDefaultConfiguration(name, DefConfig);
    }

    bool GetDefaultConfiguration(std::string &name, uCentralDefaultConfiguration &DefConfig) {
        return uCentral::Storage::Service::instance()->GetDefaultConfiguration(name, DefConfig);
    }

    bool GetDefaultConfigurations(uint64_t From, uint64_t HowMany, std::vector<uCentralDefaultConfiguration> &Devices) {
        return uCentral::Storage::Service::instance()->GetDefaultConfigurations(From, HowMany, Devices);
    }

    bool AddCommand(std::string &SerialNumber, uCentralCommandDetails &Command, bool AlreadyExecuted) {
        return uCentral::Storage::Service::instance()->AddCommand(SerialNumber, Command, AlreadyExecuted);
    }

    bool GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                     std::vector<uCentralCommandDetails> &Commands) {
        return uCentral::Storage::Service::instance()->GetCommands(SerialNumber, FromDate, ToDate, Offset, HowMany,
                                                                   Commands);
    }

    bool DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        return uCentral::Storage::Service::instance()->DeleteCommands(SerialNumber, FromDate, ToDate);
    }

    bool GetNonExecutedCommands(uint64_t Offset, uint64_t HowMany, std::vector<uCentralCommandDetails> &Commands) {
        return uCentral::Storage::Service::instance()->GetNonExecutedCommands(Offset, HowMany, Commands);
    }

    bool UpdateCommand(std::string &UUID, uCentralCommandDetails &Command) {
        return uCentral::Storage::Service::instance()->UpdateCommand(UUID, Command);
    }

    bool GetCommand(std::string &UUID, uCentralCommandDetails &Command) {
        return uCentral::Storage::Service::instance()->GetCommand(UUID, Command);
    }

    bool DeleteCommand(std::string &UUID) {
        return uCentral::Storage::Service::instance()->DeleteCommand(UUID);
    }

    bool GetReadyToExecuteCommands(uint64_t Offset, uint64_t HowMany, std::vector<uCentralCommandDetails> &Commands) {
        return uCentral::Storage::Service::instance()->GetReadyToExecuteCommands(Offset, HowMany, Commands);
    }

    bool CommandExecuted(std::string &UUID) {
        return uCentral::Storage::Service::instance()->CommandExecuted(UUID);
    }

    bool CommandCompleted(std::string &UUID, Poco::DynamicStruct ReturnVars) {
        return uCentral::Storage::Service::instance()->CommandCompleted(UUID, ReturnVars);
    }

    bool AttachFileToCommand(std::string &UUID) {
        return uCentral::Storage::Service::instance()->AttachFileToCommand(UUID);
    }

	bool GetAttachedFile(std::string & UUID, const std::string & FileName) {
		return uCentral::Storage::Service::instance()->GetAttachedFile(UUID,FileName);
	}

	bool AddBlackListDevices(std::vector<uCentralBlackListedDevice> &  Devices) {
		return uCentral::Storage::Service::instance()->AddBlackListDevices(Devices);
	}

	bool DeleteBlackListDevice(std::string & SerialNumber) {
		return uCentral::Storage::Service::instance()->DeleteBlackListDevice(SerialNumber);
	}

	bool GetBlackListDevices(uint64_t Offset, uint64_t HowMany, std::vector<uCentralBlackListedDevice> & Devices ) {
		return uCentral::Storage::Service::instance()->GetBlackListDevices(Offset, HowMany, Devices );
	}

	bool IsBlackListed(std::string &SerialNumber) {
		return uCentral::Storage::Service::instance()->IsBlackListed(SerialNumber);
	}

	inline void padTo(std::string& str, size_t num, char paddingChar = '\0') {
		str.append(num - str.length() % num, paddingChar);
	}

	std::string SerialToMAC(const std::string &Serial) {
		std::string R = Serial;

		if(R.size()<12)
			padTo(R,12,'0');
		else if (R.size()>12)
			R = R.substr(0,12);

		char buf[18];

		buf[0] = R[0]; buf[1] = R[1] ; buf[2] = ':' ;
		buf[3] = R[2] ; buf[4] = R[3]; buf[5] = ':' ;
		buf[6] = R[4]; buf[7] = R[5] ; buf[8] = ':' ;
		buf[9] = R[6] ; buf[10]= R[7]; buf[11] = ':';
		buf[12] = R[8] ; buf[13]= R[9]; buf[14] = ':';
		buf[15] = R[10] ; buf[16]= R[11];buf[17] = 0;

		return buf;
	}

    int Service::Setup_SQLite() {
        Logger_.notice("SQLite Storage enabled.");

        auto DBName = uCentral::ServiceConfig::getString("storage.type.sqlite.db");
        auto NumSessions = uCentral::ServiceConfig::getInt("storage.type.sqlite.maxsessions", 64);
        auto IdleTime = uCentral::ServiceConfig::getInt("storage.type.sqlite.idletime", 60);

        SQLiteConn_ = std::make_unique<Poco::Data::SQLite::Connector>();
        SQLiteConn_->registerConnector();
        Pool_ = std::make_unique<Poco::Data::SessionPool>(SQLiteConn_->name(), DBName, 4, NumSessions, IdleTime);
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
					"Location  VARCHAR(64) 	"
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
                    "Recorded BIGINT) ", Poco::Data::Keywords::now;

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
                    "AttachDate     BIGINT"
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
				"FileContent	BLOB"
				") ", Poco::Data::Keywords::now;

        return 0;
    }

#ifndef SMALL_BUILD

    int Service::Setup_MySQL() {
        Logger_.notice("MySQL Storage enabled.");
        auto NumSessions = uCentral::ServiceConfig::getInt("storage.type.mysql.maxsessions", 64);
        auto IdleTime = uCentral::ServiceConfig::getInt("storage.type.mysql.idletime", 60);
        auto Host = uCentral::ServiceConfig::getString("storage.type.mysql.host");
        auto Username = uCentral::ServiceConfig::getString("storage.type.mysql.username");
        auto Password = uCentral::ServiceConfig::getString("storage.type.mysql.password");
        auto Database = uCentral::ServiceConfig::getString("storage.type.mysql.database");
        auto Port = uCentral::ServiceConfig::getString("storage.type.mysql.port");

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
        Poco::Data::Session Sess = Pool_->get();

        Sess << "CREATE TABLE IF NOT EXISTS Statistics ("
                    "SerialNumber VARCHAR(30), "
                    "UUID INTEGER, "
                    "Data TEXT, "
                    "Recorded BIGINT, "
                    "INDEX StatSerial (SerialNumber ASC, Recorded ASC))", Poco::Data::Keywords::now;

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
                    "INDEX DeviceOwner (Owner ASC),",
					"INDEX LocationIndex (Location ASC))", Poco::Data::Keywords::now;

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
                    "INDEX LogSerial (SerialNumber ASC, Recorded ASC)"
                    ")", Poco::Data::Keywords::now;

        Sess << "CREATE TABLE IF NOT EXISTS HealthChecks ("
                    "SerialNumber VARCHAR(30), "
                    "UUID          BIGINT, "
                    "Data TEXT, "
                    "Sanity BIGINT , "
                    "Recorded BIGINT, "
                    "INDEX HealthSerial (SerialNumber ASC, Recorded ASC)"
                    ")", Poco::Data::Keywords::now;

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
                    "AttachDate     BIGINT"
                    "INDEX CommandListIndex (SerialNumber ASC, Submitted ASC)"
                    ")", Poco::Data::Keywords::now;

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
				"FileContent	LONGBLOB"
				") ", Poco::Data::Keywords::now;

		return 0;
    }

	std::string Service::MakeFieldList(int N) const {
		std::string Result;

		if(IsPSQL_) {
			for(auto i=0;i<N;++i)
			{
				Result += "$" + std::to_string(i+1);
				if(i+1<N)
					Result += ",";
			}
		} else {
			for(auto i=0;i<N;++i)
			{
				Result += "?";
				if(i+1<N)
					Result += ",";
			}
		}

		return "( " + Result + " )";
	}

	std::string Service::ConvertParams(const std::string & S) const {
		std::string R;

		R.reserve(S.size()*2+1);

		if(IsPSQL_) {
			auto Idx=1;
			for(auto const & i:S)
			{
				if(i=='?') {
					R += '$';
					R.append(std::to_string(Idx++));
				} else {
					R += i;
				}
			}
		} else {
			R = S;
		}
		return R;
	}

    int Service::Setup_PostgreSQL() {
        Logger_.notice("PostgreSQL Storage enabled.");

		IsPSQL_ = true ;

        auto NumSessions = uCentral::ServiceConfig::getInt("storage.type.postgresql.maxsessions", 64);
        auto IdleTime = uCentral::ServiceConfig::getInt("storage.type.postgresql.idletime", 60);
        auto Host = uCentral::ServiceConfig::getString("storage.type.postgresql.host");
        auto Username = uCentral::ServiceConfig::getString("storage.type.postgresql.username");
        auto Password = uCentral::ServiceConfig::getString("storage.type.postgresql.password");
        auto Database = uCentral::ServiceConfig::getString("storage.type.postgresql.database");
        auto Port = uCentral::ServiceConfig::getString("storage.type.postgresql.port");
        auto ConnectionTimeout = uCentral::ServiceConfig::getString("storage.type.postgresql.connectiontimeout");

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
					"Location  VARCHAR(64) 	"
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
                    "AttachDate     BIGINT"
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

        return 0;
    }

    int Service::Setup_ODBC() {
        Logger_.notice("ODBC Storage enabled.");

        auto NumSessions = uCentral::ServiceConfig::getInt("storage.type.postgresql.maxsessions", 64);
        auto IdleTime = uCentral::ServiceConfig::getInt("storage.type.postgresql.idletime", 60);
        auto Host = uCentral::ServiceConfig::getString("storage.type.postgresql.host");
        auto Username = uCentral::ServiceConfig::getString("storage.type.postgresql.username");
        auto Password = uCentral::ServiceConfig::getString("storage.type.postgresql.password");
        auto Database = uCentral::ServiceConfig::getString("storage.type.postgresql.database");
        auto Port = uCentral::ServiceConfig::getString("storage.type.postgresql.port");
        auto ConnectionTimeout = uCentral::ServiceConfig::getString("storage.type.postgresql.connectiontimeout");

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
        Poco::Data::Session Sess = Pool_->get();

        return 0;
    }

#endif

    std::string SQLEscapeStr(const std::string & S) {
        std::string R;

        for(const auto &i:S)
            if(i=='\'') {
                R += '\'';
                R += '\'';
            }
            else {
                R += i;
            }
        return R;
    }

    int Service::Start() {
        std::lock_guard<std::mutex> guard(mutex_);

		Logger_.setLevel(Poco::Message::PRIO_NOTICE);
        Logger_.notice("Starting.");
        std::string DBType = uCentral::ServiceConfig::getString("storage.type");

#ifndef SMALL_BUILD
        if (DBType == "sqlite") {
            return Setup_SQLite();
        } else if (DBType == "postgresql") {
            return Setup_PostgreSQL();
        } else if (DBType == "mysql") {
            return Setup_MySQL();
        } else if (DBType == "odbc") {
            return Setup_ODBC();
        }
        std::exit(Poco::Util::Application::EXIT_CONFIG);
#else
        return Setup_SQLite();
#endif
    }

    void Service::Stop() {
        Logger_.notice("Stopping.");
    }

    bool Service::AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats) {

        uCentral::DeviceRegistry::SetStatistics(SerialNumber, NewStats);

        try {
            Logger_.information("Device:" + SerialNumber + " Stats size:" + std::to_string(NewStats.size()));

            uint64_t Now = time(nullptr);
            Poco::Data::Session Sess = Pool_->get();
/*
                    "SerialNumber VARCHAR(30), "
                    "UUID INTEGER, "
                    "Data TEXT, "
                    "Recorded BIGINT)", now;

 */
            Poco::Data::Statement   Insert(Sess);
			std::string St{"INSERT INTO Statistics (SerialNumber, UUID, Data, Recorded) VALUES(?,?,?,?)"};

            Insert << ConvertParams(St),
                        Poco::Data::Keywords::use(SerialNumber),
						Poco::Data::Keywords::use(CfgUUID),
						Poco::Data::Keywords::use(NewStats),
						Poco::Data::Keywords::use(Now);
            Insert.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
                                    uint64_t HowMany,
                                    std::vector<uCentralStatistics> &Stats) {

        typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t> StatRecord;
        typedef std::vector<StatRecord> RecordList;

        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            RecordList              Records;
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

            bool DatesIncluded = (FromDate != 0 || ToDate != 0);

            std::string Prefix{"SELECT SerialNumber, UUID, Data, Recorded FROM Statistics "};
            std::string StatementStr = SerialNumber.empty()
                                    ? Prefix + std::string(DatesIncluded ? "WHERE " : "")
                                    : Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
                                      std::string(DatesIncluded ? " AND " : "");

            std::string DateSelector;
            if (FromDate && ToDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
            } else if (FromDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate);
            } else if (ToDate) {
                DateSelector = " Recorded<=" + std::to_string(ToDate);
            }

            Select << StatementStr + DateSelector,
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::range(Offset, Offset + HowMany - 1);
            Select.execute();

            for (auto i: Records) {
                uCentralStatistics R{
                        .UUID = i.get<1>(),
                        .Data = i.get<2>(),
                        .Recorded = i.get<3>()};
                Stats.push_back(R);
            }
            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        try {
            Poco::Data::Session Sess = Pool_->get();

            bool DatesIncluded = (FromDate != 0 || ToDate != 0);

            std::string Prefix{"DELETE FROM Statistics "};
            std::string Statement = SerialNumber.empty()
                                    ? Prefix + std::string(DatesIncluded ? "WHERE " : "")
                                    : Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
                                      std::string(DatesIncluded ? " AND " : "");

            std::string DateSelector;
            if (FromDate && ToDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
            } else if (FromDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate);
            } else if (ToDate) {
                DateSelector = " Recorded<=" + std::to_string(ToDate);
            }

            Poco::Data::Statement   Select(Sess);
            Select << Statement + DateSelector;
            Select.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::AddHealthCheckData(std::string &SerialNumber, uCentralHealthcheck &Check) {
        try {
            Logger_.information("Device:" + SerialNumber + " HealthCheck: sanity " + std::to_string(Check.Sanity));

            Poco::Data::Session Sess = Pool_->get();

/*          "SerialNumber VARCHAR(30), "
            "UUID          BIGINT, "
            "Data TEXT, "
            "Sanity BIGINT , "
            "Recorded BIGINT) ", now;
*/
            Poco::Data::Statement   Insert(Sess);
			std::string St{"INSERT INTO HealthChecks (SerialNumber, UUID, Data, Sanity, Recorded) VALUES(?,?,?,?,?)"};

            Insert  << 	ConvertParams(St),
						Poco::Data::Keywords::use(SerialNumber),
						Poco::Data::Keywords::use(Check.UUID),
						Poco::Data::Keywords::use(Check.Data),
						Poco::Data::Keywords::use(Check.Sanity),
						Poco::Data::Keywords::use(Check.Recorded);
            Insert.execute();
            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
                                     uint64_t HowMany,
                                     std::vector<uCentralHealthcheck> &Checks) {

        typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t, uint64_t> Record;
        typedef std::vector<Record> RecordList;

        // std::lock_guard<std::mutex> guard(mutex_);
        try {
            RecordList Records;
            Poco::Data::Session Sess = Pool_->get();

            bool DatesIncluded = (FromDate != 0 || ToDate != 0);

            std::string Prefix{"SELECT SerialNumber, UUID, Data, Sanity, Recorded FROM HealthChecks "};
            std::string Statement = SerialNumber.empty()
                                    ? Prefix + std::string(DatesIncluded ? "WHERE " : "")
                                    : Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
                                      std::string(DatesIncluded ? " AND " : "");

            std::string DateSelector;
            if (FromDate && ToDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
            } else if (FromDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate);
            } else if (ToDate) {
                DateSelector = " Recorded<=" + std::to_string(ToDate);
            }

            Poco::Data::Statement   Select(Sess);

            Select << Statement + DateSelector,
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::range(Offset, Offset + HowMany - 1);
            Select.execute();

            for (auto i: Records) {
                uCentralHealthcheck R;

                R.UUID = i.get<1>();
                R.Data = i.get<2>();
                R.Sanity = i.get<3>();
                R.Recorded = i.get<4>();

                Checks.push_back(R);
            }

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        try {
            Poco::Data::Session Sess = Pool_->get();

            bool DatesIncluded = (FromDate != 0 || ToDate != 0);

            std::string Prefix{"DELETE FROM HealthChecks "};
            std::string Statement = SerialNumber.empty()
                                    ? Prefix + std::string(DatesIncluded ? "WHERE " : "")
                                    : Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
                                      std::string(DatesIncluded ? " AND " : "");

            std::string DateSelector;
            if (FromDate && ToDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
            } else if (FromDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate);
            } else if (ToDate) {
                DateSelector = " Recorded<=" + std::to_string(ToDate);
            }

            Poco::Data::Statement   Delete(Sess);

            Delete << Statement + DateSelector;

            Delete.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::AddLog(std::string &SerialNumber, uCentralDeviceLog &Log, bool CrashLog) {

        try {
/*
                    "SerialNumber   VARCHAR(30), "
                    "Log            TEXT, "
                    "Data           TEXT, "
                    "Severity       BIGINT, "
                    "Recorded       BIGINT, "
                    "LogType        BIGINT"
 */
            uint64_t LogType = CrashLog ? 1 : 0 ;
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Insert(Sess);

			std::string St{"INSERT INTO DeviceLogs (SerialNumber, Log, Data, Severity, Recorded, LogType ) VALUES(?,?,?,?,?,?)"};

            Insert << ConvertParams(St) ,
						Poco::Data::Keywords::use(SerialNumber),
						Poco::Data::Keywords::use(Log.Log),
						Poco::Data::Keywords::use(Log.Data),
						Poco::Data::Keywords::use(Log.Severity),
						Poco::Data::Keywords::use(Log.Recorded),
						Poco::Data::Keywords::use(LogType);
            Insert.execute();
            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::AddLog(std::string &SerialNumber, const std::string &Log) {
        uCentralDeviceLog DeviceLog;

        DeviceLog.Log = Log;
        DeviceLog.Data = "";
        DeviceLog.Severity = uCentralDeviceLog::Level::LOG_INFO;
        DeviceLog.Recorded = time(nullptr);

        return AddLog(SerialNumber, DeviceLog, false);
    }

    bool Service::GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
                             uint64_t HowMany,
                             std::vector<uCentralDeviceLog> &Stats, uint64_t Type ) {

/*
                    "SerialNumber   VARCHAR(30), "
                    "Log            TEXT, "
                    "Data           TEXT, "
                    "Severity       BIGINT, "
                    "Recorded       BIGINT, "
                    "LogType        BIGINT"
 */

        typedef std::vector<DeviceLogsRecordTuple> RecordList;

        try {
            RecordList Records;
            Poco::Data::Session Sess = Pool_->get();

            bool DatesIncluded = (FromDate != 0 || ToDate != 0);
            bool HasWhere = DatesIncluded || !SerialNumber.empty();

            std::string Prefix{"SELECT SerialNumber, Log, Data, Severity, Recorded, LogType FROM DeviceLogs  "};
            std::string Statement = SerialNumber.empty()
                                    ? Prefix + std::string(DatesIncluded ? "WHERE " : "")
                                    : Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
                                      std::string(DatesIncluded ? " AND " : "") ;

            std::string DateSelector;
            if (FromDate && ToDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
            } else if (FromDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate);
            } else if (ToDate) {
                DateSelector = " Recorded<=" + std::to_string(ToDate);
            }

            std::string TypeSelector;
            if(Type)
            {
                TypeSelector = (HasWhere ? " AND LogType=" : " WHERE LogType=" ) + std::to_string((Type==1 ? 0 : 1));
            }

            Poco::Data::Statement   Select(Sess);

            Select << Statement + DateSelector + TypeSelector,
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::range(Offset, Offset + HowMany - 1);

            Select.execute();

            for (auto i: Records) {
                uCentralDeviceLog R{
                        .Log = i.get<1>(),
                        .Data = i.get<2>(),
                        .Severity = i.get<3>(),
                        .Recorded = i.get<4>(),
                        .LogType = i.get<5>()};
                Stats.push_back(R);
            }
            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Type) {
        try {
            Poco::Data::Session Sess = Pool_->get();

            bool DatesIncluded = (FromDate != 0 || ToDate != 0);
            bool HasWhere = DatesIncluded || !SerialNumber.empty();

            std::string Prefix{"DELETE FROM DeviceLogs "};
            std::string StatementStr = SerialNumber.empty()
                                    ? Prefix + std::string(DatesIncluded ? "WHERE " : "")
                                    : Prefix + "WHERE SerialNumber='" + SerialNumber + "'" +
                                      std::string(DatesIncluded ? " AND " : "");

            std::string DateSelector;
            if (FromDate && ToDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate) + " AND Recorded<=" + std::to_string(ToDate);
            } else if (FromDate) {
                DateSelector = " Recorded>=" + std::to_string(FromDate);
            } else if (ToDate) {
                DateSelector = " Recorded<=" + std::to_string(ToDate);
            }

            std::string TypeSelector;
            if(Type)
            {
                TypeSelector = (HasWhere ? " AND LogType=" : " WHERE LogType=" ) + std::to_string((Type==1 ? 0 : 1));
            }

            Poco::Data::Statement   Delete(Sess);
            Delete << StatementStr + DateSelector + TypeSelector;

            Delete.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t &NewUUID) {
        try {

            uCentral::Config::Config Cfg(Configuration);

            if (!Cfg.Valid()) {
                Logger_.warning(Poco::format("CONFIG-UPDATE(%s): Configuration was not valid", SerialNumber));
                return false;
            }

            Poco::Data::Session Sess = Pool_->get();

            Poco::Data::Statement   Select(Sess);

            uint64_t CurrentUUID;

			std::string St{"SELECT UUID FROM Devices WHERE SerialNumber=?"};

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(CurrentUUID),
                    Poco::Data::Keywords::use(SerialNumber);
            Select.execute();

			uint64_t Now = time(nullptr);

			NewUUID = CurrentUUID==Now ? Now + 1 : Now;

            if (Cfg.SetUUID(CurrentUUID)) {
                std::string NewConfig = Cfg.get();

                Poco::Data::Statement   Update(Sess);
				std::string St{"UPDATE Devices SET Configuration=? , UUID=?,  LastConfigurationChange=?  WHERE SerialNumber=?"};
                Update  << ConvertParams(St),
							Poco::Data::Keywords::use(NewConfig),
							Poco::Data::Keywords::use(NewUUID),
							Poco::Data::Keywords::use(Now),
							Poco::Data::Keywords::use(SerialNumber);
                Update.execute();

                Logger_.information(Poco::format("CONFIG-UPDATE(%s): UUID is %Lu", SerialNumber, NewUUID));

                return true;
            }

            return false;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::CreateDevice(uCentralDevice &DeviceDetails) {
        // std::lock_guard<std::mutex> guard(mutex_);

        std::string SerialNumber;
        try {

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT SerialNumber FROM Devices WHERE SerialNumber=?"};

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(SerialNumber),
                    Poco::Data::Keywords::use(DeviceDetails.SerialNumber);
            Select.execute();

            if (SerialNumber.empty()) {
                uCentral::Config::Config Cfg(DeviceDetails.Configuration);

                if (Cfg.Valid() && Cfg.SetUUID(DeviceDetails.UUID)) {
                    DeviceDetails.Configuration = Cfg.get();
                    uint64_t Now = time(nullptr);

                    // DeviceDetails.Print();
/*
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
                    "Owner 			VARCHAR(64),
                    "Location		VARCHAR(64)"

 */
                    Poco::Data::Statement   Insert(Sess);

					std::string St{"INSERT INTO Devices (SerialNumber, DeviceType, MACAddress, Manufacturer, UUID, "
								   "Configuration, Notes, CreationTimestamp, LastConfigurationChange, LastConfigurationDownload,"
								   "Owner, Location )"
								   "VALUES(?,?,?,?,?,?,?,?,?,?,?,?)"};

                    Insert  << ConvertParams(St),
								Poco::Data::Keywords::use(DeviceDetails.SerialNumber),
								Poco::Data::Keywords::use(DeviceDetails.DeviceType),
								Poco::Data::Keywords::use(DeviceDetails.MACAddress),
								Poco::Data::Keywords::use(DeviceDetails.Manufacturer),
								Poco::Data::Keywords::use(DeviceDetails.UUID),
								Poco::Data::Keywords::use(DeviceDetails.Configuration),
								Poco::Data::Keywords::use(DeviceDetails.Notes),
								Poco::Data::Keywords::use(Now),
								Poco::Data::Keywords::use(Now),
								Poco::Data::Keywords::use(Now),
								Poco::Data::Keywords::use(DeviceDetails.Owner),
								Poco::Data::Keywords::use(DeviceDetails.Location);
                    Insert.execute();

                    return true;
                } else {
                    Logger_.warning("Cannot create device: invalid configuration.");
                    return false;
                }
            }
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::CreateDefaultDevice(const std::string &SerialNumber, const std::string &Capabilities) {

        uCentralDevice D;
        Logger_.information(Poco::format("AUTO-CREATION(%s)", SerialNumber));
        uint64_t Now = time(nullptr);

        uCentral::Config::Capabilities Caps(Capabilities);
        uCentralDefaultConfiguration DefConfig;

        if (FindDefaultConfigurationForModel(Caps.ModelId(), DefConfig)) {
            uCentral::Config::Config NewConfig(DefConfig.Configuration);
            NewConfig.SetUUID(Now);
            D.Configuration = NewConfig.get();
        } else {
            uCentral::Config::Config NewConfig;
            NewConfig.SetUUID(Now);
            D.Configuration = NewConfig.get();
        }

        D.SerialNumber = SerialNumber;
        D.DeviceType = Caps.DeviceType();
        D.MACAddress = SerialToMAC(SerialNumber);
        D.Manufacturer = Caps.Manufacturer();
        D.UUID = Now;
        D.Notes = "auto created device.";
        D.CreationTimestamp = D.LastConfigurationDownload = D.LastConfigurationChange = Now;

        return CreateDevice(D);
    }

	bool Service::SetLocation(std::string & SerialNumber, std::string & LocationUUID) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);

			std::string St{"UPDATE Devices SET Location=? WHERE SerialNumber=?"};

			Update  << ConvertParams(St) ,
				Poco::Data::Keywords::use(LocationUUID),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::SetOwner(std::string & SerialNumber, std::string & OwnerUUID) {
		try {
			Poco::Data::Session     Sess = Pool_->get();
			Poco::Data::Statement   Update(Sess);
			std::string St{"UPDATE Devices SET Owner=?  WHERE SerialNumber=?"};

			Update << ConvertParams(St) ,
				Poco::Data::Keywords::use(OwnerUUID),
				Poco::Data::Keywords::use(SerialNumber);
			Update.execute();
			return true;
		}
		catch (const Poco::Exception &E) {
			Logger_.warning(
				Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
		}
		return false;
	}

	bool Service::DeleteDevice(std::string &SerialNumber) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Delete(Sess);

			std::string St{"DELETE FROM Devices WHERE SerialNumber=?"};

            Delete << ConvertParams(St),
                        Poco::Data::Keywords::use(SerialNumber);
            Delete.execute();
            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::GetDevice(std::string &SerialNumber, uCentralDevice &DeviceDetails) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT "
						   "SerialNumber, "
						   "DeviceType, "
						   "MACAddress, "
						   "Manufacturer, "
						   "UUID, "
						   "Configuration, "
						   "Notes, "
						   "CreationTimestamp, "
						   "LastConfigurationChange, "
						   "LastConfigurationDownload, "
						   "Owner,"
						   "Location "
							"FROM Devices WHERE SerialNumber=?"};

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(DeviceDetails.SerialNumber),
                    Poco::Data::Keywords::into(DeviceDetails.DeviceType),
                    Poco::Data::Keywords::into(DeviceDetails.MACAddress),
                    Poco::Data::Keywords::into(DeviceDetails.Manufacturer),
                    Poco::Data::Keywords::into(DeviceDetails.UUID),
                    Poco::Data::Keywords::into(DeviceDetails.Configuration),
                    Poco::Data::Keywords::into(DeviceDetails.Notes),
                    Poco::Data::Keywords::into(DeviceDetails.CreationTimestamp),
                    Poco::Data::Keywords::into(DeviceDetails.LastConfigurationChange),
                    Poco::Data::Keywords::into(DeviceDetails.LastConfigurationDownload),
					Poco::Data::Keywords::into(DeviceDetails.Owner),
					Poco::Data::Keywords::into(DeviceDetails.Location),
					Poco::Data::Keywords::use(SerialNumber);

            Select.execute();

            if (DeviceDetails.SerialNumber.empty())
                return false;

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::DeviceExists(std::string &SerialNumber) {
        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

            std::string Serial;

			std::string St{"SELECT SerialNumber FROM Devices WHERE SerialNumber=?"};

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(Serial),
					Poco::Data::Keywords::use(SerialNumber);
            Select.execute();

            if (Serial.empty())
                return false;

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::UpdateDevice(uCentralDevice &NewConfig) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Update(Sess);

            uint64_t Now = time(nullptr);

			std::string St{"UPDATE Devices SET Manufacturer=?, DeviceType=?, MACAddress=?, Notes=?, "
						   "LastConfigurationChange=? WHERE SerialNumber=?"};

            Update  << ConvertParams(St) ,
						Poco::Data::Keywords::use(NewConfig.Manufacturer),
						Poco::Data::Keywords::use(NewConfig.DeviceType),
						Poco::Data::Keywords::use(NewConfig.MACAddress),
						Poco::Data::Keywords::use(NewConfig.Notes),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(NewConfig.SerialNumber);

            Update.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), NewConfig.SerialNumber,
                                         E.displayText()));
        }

        return false;
    }


    bool Service::GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentralDevice> &Devices) {

        typedef Poco::Tuple<
                std::string,
                std::string,
                std::string,
                std::string,
                uint64_t,
                std::string,
                std::string,
                uint64_t,
                uint64_t,
                uint64_t,
				std::string,
				std::string> DeviceRecord;
        typedef std::vector<DeviceRecord> RecordList;

        RecordList Records;

        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

            Select << "SELECT "
                        "SerialNumber, "
                        "DeviceType, "
                        "MACAddress, "
                        "Manufacturer, "
                        "UUID, "
                        "Configuration, "
                        "Notes, "
                        "CreationTimestamp, "
                        "LastConfigurationChange, "
                        "LastConfigurationDownload, "
					  	"Owner, "
					  	"Location "
                        "FROM Devices",
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::range(From, From + HowMany - 1);
            Select.execute();

            for (auto i: Records) {
                uCentralDevice R{
                        .SerialNumber   = i.get<0>(),
                        .DeviceType     = i.get<1>(),
                        .MACAddress     = i.get<2>(),
                        .Manufacturer   = i.get<3>(),
                        .UUID           = i.get<4>(),
                        .Configuration  = i.get<5>(),
                        .Notes          = i.get<6>(),
                        .CreationTimestamp = i.get<7>(),
                        .LastConfigurationChange = i.get<8>(),
                        .LastConfigurationDownload = i.get<9>(),
						.Owner = i.get<10>(),
						.Location = i.get<11>()};

                Devices.push_back(R);
            }
            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
        }
        return false;
    }

    bool Service::UpdateDeviceCapabilities(std::string &SerialNumber, std::string &Capabs) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            std::string SS;

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

            uint64_t Now = time(nullptr);

			std::string St{"SELECT SerialNumber FROM Capabilities WHERE SerialNumber=?"};

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(SS),
                    Poco::Data::Keywords::use(SerialNumber);
            Select.execute();

/*
                    "SerialNumber VARCHAR(30) PRIMARY KEY, "
                    "Capabilities TEXT, "
                    "FirstUpdate BIGINT, "
                    "LastUpdate BIGINT"

 */
            if (SS.empty()) {
                Logger_.information("Adding capabilities for " + SerialNumber);
                Poco::Data::Statement   Insert(Sess);

				std::string St{"INSERT INTO Capabilities (SerialNumber, Capabilities, FirstUpdate, LastUpdate) "
							   "VALUES(?,?,?,?)"};

                Insert  << ConvertParams(St),
                            Poco::Data::Keywords::use(SerialNumber),
							Poco::Data::Keywords::use(Capabs),
							Poco::Data::Keywords::use(Now),
							Poco::Data::Keywords::use(Now);
                Insert.execute();

            } else {
                Logger_.information("Updating capabilities for " + SerialNumber);
                Poco::Data::Statement   Update(Sess);

				std::string St{"UPDATE Capabilities SET Capabilities=?, LastUpdate=? WHERE SerialNumber=?"};

                Update  << 	ConvertParams(St),
                            Poco::Data::Keywords::use(Capabs),
							Poco::Data::Keywords::use(Now),
							Poco::Data::Keywords::use(SerialNumber);
                Update.execute();
            }
            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::GetDeviceCapabilities(std::string &SerialNumber, uCentralCapabilities &Caps) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

            std::string TmpSerialNumber;

			std::string St{"SELECT SerialNumber, Capabilities, FirstUpdate, LastUpdate FROM Capabilities WHERE SerialNumber=?"};

            Select  << ConvertParams(St),
                    Poco::Data::Keywords::into(TmpSerialNumber),
                    Poco::Data::Keywords::into(Caps.Capabilities),
                    Poco::Data::Keywords::into(Caps.FirstUpdate),
                    Poco::Data::Keywords::into(Caps.LastUpdate),
                    Poco::Data::Keywords::use(SerialNumber);
            Select.execute();

            if (TmpSerialNumber.empty())
                return false;

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::DeleteDeviceCapabilities(std::string &SerialNumber) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Delete(Sess);

			std::string St{"DELETE FROM Capabilities WHERE SerialNumber=?"};

            Delete << ConvertParams(St),
                        Poco::Data::Keywords::use(SerialNumber);
            Delete.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig,
                                        uint64_t &UUID) {
        // std::lock_guard<std::mutex> guard(mutex_);
        std::string SS;
        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);
            uint64_t Now = time(nullptr);

			std::string St{"SELECT SerialNumber, UUID, Configuration FROM Devices WHERE SerialNumber=?"};

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(SS),
                    Poco::Data::Keywords::into(UUID),
                    Poco::Data::Keywords::into(NewConfig),
                    Poco::Data::Keywords::use(SerialNumber);

            Select.execute();

            if (SS.empty()) {
                return false;
            }

            //  Let's update the last downloaded time
            Poco::Data::Statement   Update(Sess);
			std::string St2{"UPDATE Devices SET LastConfigurationDownload=?  WHERE SerialNumber=?"};

            Update << ConvertParams(St2),
                    Poco::Data::Keywords::use(Now),
                    Poco::Data::Keywords::use(SerialNumber);
            Update.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    /*
     *  Data model for DefaultConfigurations:

        Table name: DefaultConfigs

                    "Name VARCHAR(30) PRIMARY KEY, "
                    "Configuration TEXT, "
                    "Models TEXT, "
                    "Description TEXT, "
                    "Created BIGINT , "
                    "LastModified BIGINT)", now;
     */

    bool Service::CreateDefaultConfiguration(std::string &Name, uCentralDefaultConfiguration &DefConfig) {
        try {

            std::string TmpName;

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

            Select <<   "SELECT Name FROM DefaultConfigs WHERE Name='%s'",
                            Poco::Data::Keywords::into(TmpName),
                            Name;
            Select.execute();

            if (TmpName.empty()) {

                uCentral::Config::Config Cfg(DefConfig.Configuration);
/*
                     "Name VARCHAR(30) PRIMARY KEY, "
                    "Configuration TEXT, "
                    "Models TEXT, "
                    "Description TEXT, "
                    "Created BIGINT , "
                    "LastModified BIGINT)", now;

 */

                if (Cfg.Valid()) {
                    uint64_t Now = time(nullptr);
                    Poco::Data::Statement   Insert(Sess);

					std::string St{"INSERT INTO DefaultConfigs (Name, Configuration, Models, Description, Created, LastModified) "
								   "VALUES(?,?,?,?,?,?)"};

					Insert  << ConvertParams(St),
								Poco::Data::Keywords::use(Name),
								Poco::Data::Keywords::use(DefConfig.Configuration),
								Poco::Data::Keywords::use(DefConfig.Models),
								Poco::Data::Keywords::use(DefConfig.Description),
								Poco::Data::Keywords::use(Now),
								Poco::Data::Keywords::use(Now);

                    Insert.execute();

                    return true;
                } else {
                    Logger_.warning("Cannot create device: invalid configuration.");
                    return false;
                }
            } else {
                Logger_.warning("Default configuration already exists.");
            }
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), Name, E.displayText()));
        }
        return false;
    }

    bool Service::DeleteDefaultConfiguration(std::string &Name) {
        try {

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Delete(Sess);

			std::string St{"DELETE FROM DefaultConfigs WHERE Name=?"};

            Delete << ConvertParams(St),
                        Poco::Data::Keywords::use(Name);
            Delete.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), Name, E.displayText()));
        }
        return false;
    }

    bool Service::UpdateDefaultConfiguration(std::string &Name, uCentralDefaultConfiguration &DefConfig) {
        try {

            Poco::Data::Session Sess = Pool_->get();

            uCentral::Config::Config Cfg(DefConfig.Configuration);

            if (Cfg.Valid()) {

                uint64_t Now = time(nullptr);
                Poco::Data::Statement   Update(Sess);

				std::string St{"UPDATE DefaultConfigs SET Configuration=?,  Models=?,  Description=?,  LastModified=?  WHERE Name=?"};

                Update << ConvertParams(St),
                            Poco::Data::Keywords::use(DefConfig.Configuration),
							Poco::Data::Keywords::use(DefConfig.Models),
							Poco::Data::Keywords::use(DefConfig.Description),
							Poco::Data::Keywords::use(Now),
							Poco::Data::Keywords::use(Name);

                Update.execute();
                return true;
            } else {
                Logger_.warning(
                        Poco::format("Default configuration: %s cannot be set to an invalid configuration.", Name));
            }
            return false;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), Name, E.displayText()));
        }
        return false;
    }

    bool Service::GetDefaultConfiguration(std::string &Name, uCentralDefaultConfiguration &DefConfig) {
        try {

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT "
						   "Name, "
						   "Configuration, "
						   "Models, "
						   "Description, "
						   "Created, "
						   "LastModified "
						   "FROM DefaultConfigs WHERE Name=?"};

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(DefConfig.Name),
                    Poco::Data::Keywords::into(DefConfig.Configuration),
                    Poco::Data::Keywords::into(DefConfig.Models),
                    Poco::Data::Keywords::into(DefConfig.Description),
                    Poco::Data::Keywords::into(DefConfig.Created),
                    Poco::Data::Keywords::into(DefConfig.LastModified),
                    Poco::Data::Keywords::use(Name);

            Select.execute();

            if (DefConfig.Name.empty())
                return false;

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), Name, E.displayText()));
        }
        return false;
    }

    bool Service::GetDefaultConfigurations(uint64_t From, uint64_t HowMany,
                                           std::vector<uCentralDefaultConfiguration> &DefConfigs) {
        typedef Poco::Tuple<
                std::string,
                std::string,
                std::string,
                std::string,
                uint64_t,
                uint64_t> DeviceRecord;
        typedef std::vector<DeviceRecord> RecordList;

        RecordList Records;

        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

            Select << "SELECT "
                        "Name, "
                        "Configuration, "
                        "Models, "
                        "Description, "
                        "Created, "
                        "LastModified "
                        "FROM DefaultConfigs",
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::range(From, From + HowMany - 1);

            Select.execute();

            for (auto i: Records) {
                uCentralDefaultConfiguration R{
                        .Name           = i.get<0>(),
                        .Configuration  = i.get<1>(),
                        .Models         = i.get<2>(),
                        .Description    = i.get<3>(),
                        .Created        = i.get<4>(),
                        .LastModified   = i.get<5>()};

                DefConfigs.push_back(R);
            }
            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
        }
        return false;
    }

    bool FindInList(const std::string &Model, const std::string &List) {
        auto P = 0;
        std::string Token;

        while (P < List.size()) {
            auto P2 = List.find_first_of(',', P);
            if (P2 == std::string::npos) {
                Token = List.substr(P);
                if (Model.find(Token) != std::string::npos)
                    return true;
                return false;
            } else {
                Token = List.substr(P, P2);
                if (Model.find(Token) != std::string::npos)
                    return true;
            }
            P = P2 + 1;
        }
        return false;
    }

    bool Service::FindDefaultConfigurationForModel(const std::string &Model, uCentralDefaultConfiguration &DefConfig) {
        try {
            typedef Poco::Tuple<
                    std::string,
                    std::string,
                    std::string,
                    std::string,
                    uint64_t,
                    uint64_t> DeviceRecord;
            typedef std::vector<DeviceRecord> RecordList;
            RecordList Records;

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

            Select << "SELECT "
                        "Name, "
                        "Configuration, "
                        "Models, "
                        "Description, "
                        "Created, "
                        "LastModified "
                        "FROM DefaultConfigs",
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::range(0, 2);
            Select.execute();

            for (auto i: Records) {
                DefConfig.Models = i.get<2>();
                if (FindInList(Model, DefConfig.Models)) {
                    DefConfig.Name = i.get<0>();
                    Logger_.information(Poco::format("AUTO-PROVISIONING: found default configuration '%s' for model:%s",
                                                     DefConfig.Name, Model));
                    DefConfig.Name = i.get<0>();
                    DefConfig.Configuration = i.get<1>();
                    DefConfig.Models = i.get<2>();
                    DefConfig.Description = i.get<3>();
                    DefConfig.Created = i.get<4>();
                    DefConfig.LastModified = i.get<5>();
                    return true;
                }
            }
            Logger_.information(Poco::format("AUTO-PROVISIONING: no default configuration for model:%s", Model));
            return false;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
        }
        return false;
    }

    bool Service::AddCommand(std::string &SerialNumber, uCentralCommandDetails &Command, bool AlreadyExecuted) {
        try {
            /*
                    "UUID           VARCHAR(30) PRIMARY KEY, "
                    "SerialNumber   VARCHAR(30), "
                    "Command        VARCHAR(32), "
                    "Status         VARCHAR(64), "
                    "SubmittedBy    VARCHAR(64), "
                    "Results        TEXT, "
                    "Details        TEXT, "
                    "Submitted      BIGINT, "
                    "Executed       BIGINT, "
                    "Completed      BIGINT, "
                    "RunAt          BIGINT, "
                    "ErrorCode      BIGINT, "
                    "Custom         BIGINT, "
                    "WaitingForFile BIGINT, "
                    "AttachDate     BIGINT"
             */

            uint64_t Now = time(nullptr);

            Command.Submitted = Now;
            Command.Completed = 0;
            if (AlreadyExecuted) {
                Command.Executed = Now;
                Command.Status = "executing";
            } else {
                Command.Executed = 0;
                Command.Status = "pending";
            }

            Command.ErrorCode = 0;
            Command.AttachDate = 0 ;

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Insert(Sess);

			std::string St{"INSERT INTO CommandList (UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, "
						   "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate) "
						   "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"};

            Insert  << ConvertParams(St),
				Poco::Data::Keywords::use(Command.UUID),
				Poco::Data::Keywords::use(Command.SerialNumber),
				Poco::Data::Keywords::use(Command.Command),
				Poco::Data::Keywords::use(Command.Status),
				Poco::Data::Keywords::use(Command.SubmittedBy),
				Poco::Data::Keywords::use(Command.Results),
				Poco::Data::Keywords::use(Command.Details),
				Poco::Data::Keywords::use(Command.Submitted),
				Poco::Data::Keywords::use(Command.Executed),
				Poco::Data::Keywords::use(Command.Completed),
				Poco::Data::Keywords::use(Command.RunAt),
				Poco::Data::Keywords::use(Command.ErrorCode),
				Poco::Data::Keywords::use(Command.Custom),
				Poco::Data::Keywords::use(Command.WaitingForFile),
				Poco::Data::Keywords::use(Command.AttachDate);

            Insert.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset,
                              uint64_t HowMany, std::vector<uCentralCommandDetails> &Commands) {
        typedef std::vector<CommandDetailsRecordTuple> RecordList;

        /*
            "UUID           VARCHAR(30) PRIMARY KEY, "
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
            "AttachDate     BIGINT"
         */

        try {
            RecordList Records;
            Poco::Data::Session Sess = Pool_->get();

            bool DatesIncluded = (FromDate != 0 || ToDate != 0);

            std::string Fields{"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText, "
                               "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate  FROM CommandList "};
            std::string IntroStatement = SerialNumber.empty()
                                    ? Fields + std::string(DatesIncluded ? "WHERE " : "")
                                    : Fields + "WHERE SerialNumber='" + SerialNumber + "'" +
                                      std::string(DatesIncluded ? " AND " : "");

            std::string DateSelector;
            if (FromDate && ToDate) {
                DateSelector = " Submitted>=" + std::to_string(FromDate) + " AND Submitted<=" + std::to_string(ToDate);
            } else if (FromDate) {
                DateSelector = " Submitted>=" + std::to_string(FromDate);
            } else if (ToDate) {
                DateSelector = " Submitted<=" + std::to_string(ToDate);
            }

            Poco::Data::Statement   Select(Sess);

            Select << IntroStatement + DateSelector,
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::range(Offset, Offset + HowMany - 1);

            Select.execute();

            for (auto i: Records) {
                uCentralCommandDetails R{
                        .UUID = i.get<0>(),
                        .SerialNumber = i.get<1>(),
                        .Command = i.get<2>(),
                        .Status = i.get<3>(),
                        .SubmittedBy = i.get<4>(),
                        .Results = i.get<5>(),
                        .Details = i.get<6>(),
                        .ErrorText = i.get<7>(),
                        .Submitted = i.get<8>(),
                        .Executed = i.get<9>(),
                        .Completed = i.get<10>(),
                        .RunAt = i.get<11>(),
                        .ErrorCode = i.get<12>(),
                        .Custom = i.get<13>(),
                        .WaitingForFile = i.get<14>(),
                        .AttachDate = i.get<15>()};

                Commands.push_back(R);
            }

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Delete(Sess);

            bool DatesIncluded = (FromDate != 0 || ToDate != 0);

            std::string IntroStatement = SerialNumber.empty()
                                    ? "DELETE FROM CommandList " + std::string(DatesIncluded ? "WHERE " : "")
                                    : "DELETE FROM CommandList WHERE SerialNumber='" + SerialNumber + "'" +
                                      std::string(DatesIncluded ? " AND " : "");

            std::string DateSelector;
            if (FromDate && ToDate) {
                DateSelector = " Submitted>=" + std::to_string(FromDate) + " AND Submitted<=" + std::to_string(ToDate);
            } else if (FromDate) {
                DateSelector = " Submitted>=" + std::to_string(FromDate);
            } else if (ToDate) {
                DateSelector = " Submitted<=" + std::to_string(ToDate);
            }

            Delete << IntroStatement + DateSelector;

            Delete.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("%s(%s): Failed with: %s", std::string(__func__), SerialNumber, E.displayText()));
        }
        return false;
    }

    bool
    Service::GetNonExecutedCommands(uint64_t Offset, uint64_t HowMany, std::vector<uCentralCommandDetails> &Commands) {
        typedef std::vector<CommandDetailsRecordTuple> RecordList;
        /*
            "UUID           VARCHAR(30) PRIMARY KEY, "
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
            "AttachDate     BIGINT"
         */

        try {
            RecordList Records;

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);
			bool Done = false;

			while(Commands.size()<HowMany && !Done) {
				// range(Offset, Offset + HowMany - 1)
				Select << "SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText,"
						  "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate FROM CommandList "
						  " WHERE Executed=0",
					Poco::Data::Keywords::into(Records),
					Poco::Data::Keywords::range(Offset, Offset + HowMany - 1);
				Select.execute();

				for (auto i : Records) {
					Offset++;
					uCentralCommandDetails R{.UUID = i.get<0>(),
											 .SerialNumber = i.get<1>(),
											 .Command = i.get<2>(),
											 .Status = i.get<3>(),
											 .SubmittedBy = i.get<4>(),
											 .Results = i.get<5>(),
											 .Details = i.get<6>(),
											 .ErrorText = i.get<7>(),
											 .Submitted = i.get<8>(),
											 .Executed = i.get<9>(),
											 .Completed = i.get<10>(),
											 .RunAt = i.get<11>(),
											 .ErrorCode = i.get<12>(),
											 .Custom = i.get<13>(),
											 .WaitingForFile = i.get<14>(),
											 .AttachDate = i.get<15>()};

					//	Only return rhe commands for our own devices.
					if (uCentral::DeviceRegistry::Connected(R.SerialNumber))
						Commands.push_back(R);
				}

				//	If we could not return enough commands, we are done.
				if(Records.size() < HowMany)
					Done = true;
			}

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s: Failed with: %s", std::string(__func__), E.displayText()));
        }
        return false;

    }

    bool Service::UpdateCommand(std::string &UUID, uCentralCommandDetails &Command) {

        try {
            Poco::Data::Session Sess = Pool_->get();
            /*
                "UUID           VARCHAR(30) PRIMARY KEY, "
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
                "AttachDate     BIGINT"
             */

            Poco::Data::Statement   Update(Sess);

			std::string St{"UPDATE CommandList SET Status=?,  Executed=?,  Completed=?,  Results=?,  ErrorText=?,  ErrorCode=?  WHERE UUID=?"};

            Update  << ConvertParams(St),
				Poco::Data::Keywords::use(Command.Status),
				Poco::Data::Keywords::use(Command.Executed),
				Poco::Data::Keywords::use(Command.Completed),
				Poco::Data::Keywords::use(Command.Results),
				Poco::Data::Keywords::use(Command.ErrorText),
				Poco::Data::Keywords::use(Command.ErrorCode),
				Poco::Data::Keywords::use(UUID);

            Update.execute();

            return true;

        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), UUID, E.displayText()));
        }
        return false;
    }

    bool Service::GetCommand(std::string &UUID, uCentralCommandDetails &Command) {

        try {
            Poco::Data::Session Sess = Pool_->get();
            /*
                "UUID           VARCHAR(30) PRIMARY KEY, "
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
                "AttachDate     BIGINT"
             */
            Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText, "
						   "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate FROM CommandList "
						   "WHERE UUID=?"};

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(Command.UUID),
                    Poco::Data::Keywords::into(Command.SerialNumber),
                    Poco::Data::Keywords::into(Command.Command),
                    Poco::Data::Keywords::into(Command.Status),
                    Poco::Data::Keywords::into(Command.SubmittedBy),
                    Poco::Data::Keywords::into(Command.Results),
                    Poco::Data::Keywords::into(Command.Details),
                    Poco::Data::Keywords::into(Command.ErrorText),
                    Poco::Data::Keywords::into(Command.Submitted),
                    Poco::Data::Keywords::into(Command.Executed),
                    Poco::Data::Keywords::into(Command.Completed),
                    Poco::Data::Keywords::into(Command.RunAt),
                    Poco::Data::Keywords::into(Command.ErrorCode),
                    Poco::Data::Keywords::into(Command.Custom),
                    Poco::Data::Keywords::into(Command.WaitingForFile),
                    Poco::Data::Keywords::into(Command.AttachDate),
                    Poco::Data::Keywords::use(UUID);

            Select.execute();

            return true;

        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), UUID, E.displayText()));
        }
        return false;
    }

    bool Service::DeleteCommand(std::string &UUID) {
        try {
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Delete(Sess);

			std::string St{"DELETE FROM CommandList WHERE UUID=?"};

            Delete << ConvertParams(St),
                        Poco::Data::Keywords::use(UUID);
            Delete.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", std::string(__func__), UUID, E.displayText()));
        }
        return false;
    }

    bool Service::GetReadyToExecuteCommands(uint64_t Offset, uint64_t HowMany,
                                            std::vector<uCentralCommandDetails> &Commands) {
        // todo: finish the GetReadyToExecuteCommands call...
        try {
            typedef std::vector<CommandDetailsRecordTuple> RecordList;
            uint64_t Now = time(nullptr);
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Select(Sess);

			std::string St{"SELECT UUID, SerialNumber, Command, Status, SubmittedBy, Results, Details, ErrorText, "
						   "Submitted, Executed, Completed, RunAt, ErrorCode, Custom, WaitingForFile, AttachDate FROM CommandList "
						   " WHERE RunAt < ? And Executed=0"};

            RecordList Records;

            Select << ConvertParams(St),
                    Poco::Data::Keywords::into(Records),
                    Poco::Data::Keywords::use(Now),
                    Poco::Data::Keywords::range(Offset, Offset + HowMany - 1);
            Select.execute();

            for (auto i: Records) {
                uCentralCommandDetails R{
                        .UUID = i.get<0>(),
                        .SerialNumber = i.get<1>(),
                        .Command = i.get<2>(),
                        .Status = i.get<3>(),
                        .SubmittedBy = i.get<4>(),
                        .Results = i.get<5>(),
                        .Details = i.get<6>(),
                        .ErrorText = i.get<7>(),
                        .Submitted = i.get<8>(),
                        .Executed = i.get<9>(),
                        .Completed = i.get<10>(),
                        .RunAt = i.get<11>(),
                        .ErrorCode = i.get<12>(),
                        .Custom = i.get<13>(),
                        .WaitingForFile = i.get<14>(),
                        .AttachDate = i.get<15>()};

                if (uCentral::DeviceRegistry::Connected(R.SerialNumber))
                    Commands.push_back(R);
            }

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(
                    Poco::format("GetReadyToExecuteCommands(): Failed to retrieve the list. %s", E.displayText()));
        }
        return false;
    }

    bool Service::CommandExecuted(std::string &UUID) {
        try {
            uint64_t Now = time(nullptr);

            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Update(Sess);

			std::string St{"UPDATE CommandList SET Executed=? WHERE UUID=?"};

            Update << ConvertParams(St),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(UUID);

            Update.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("Could not update field on command %s", UUID));
        }

        return false;
    }

    bool Service::CommandCompleted(std::string &UUID, Poco::DynamicStruct ReturnVars) {
        try {
            uint64_t Now = time(nullptr);

            // Parse the result to get the ErrorText and make sure that this is a JSON document
            auto ResultObj = ReturnVars["result"];
            Poco::DynamicStruct ResultFields = ResultObj.extract<Poco::DynamicStruct>();

            auto StatusObj = ResultFields["status"];
            Poco::DynamicStruct StatusInnerObj = StatusObj.extract<Poco::DynamicStruct>();
            uint64_t ErrorCode = StatusInnerObj["error"];
            auto ErrorText = StatusInnerObj["text"].toString();

            std::stringstream ResultText;
            Poco::JSON::Stringifier::stringify(ResultObj, ResultText);
            std::string ResultStr{ResultText.str()};

            // std::cout << ">>> UUID: " << UUID << " Errorcode: " << ErrorCode << " ErrorText: " << ErrorText << std::endl;
            Poco::Data::Session     Sess = Pool_->get();
            Poco::Data::Statement   Update(Sess);

            std::string StatusText{"completed"};

			std::string St{"UPDATE CommandList SET Completed=?, ErrorCode=?, ErrorText=?, Results=?, Status=? WHERE UUID=?"};

            Update  << ConvertParams(St),
						Poco::Data::Keywords::use(Now),
						Poco::Data::Keywords::use(ErrorCode),
						Poco::Data::Keywords::use(ErrorText),
						Poco::Data::Keywords::use(ResultStr),
						Poco::Data::Keywords::use(StatusText),
						Poco::Data::Keywords::use(UUID);
            Update.execute();

            return true;
        }
        catch (const Poco::Exception &E) {
            std::cout << "Could not update record" << E.displayText() << "  " << E.className() << " " << E.what()
                      << std::endl;
        }

        return false;
    }

    bool Service::AttachFileToCommand(std::string &UUID) {
        try {
			Poco::Data::Session Sess = Pool_->get();
			uint64_t Now = time(nullptr);
            uint64_t WaitForFile = 0;

            Poco::Data::Statement   Update(Sess);

			std::string St{"UPDATE CommandList SET WaitingForFile=?, AttachDate=? WHERE UUID=?"};

			Update << ConvertParams(St),
				Poco::Data::Keywords::use(WaitForFile),
				Poco::Data::Keywords::use(Now),
				Poco::Data::Keywords::use(UUID);
            Update.execute();

			Poco::Data::LOB<char>		L;
			Poco::Data::LOBOutputStream	OL(L);

			Poco::File	FileName = uCentral::ServiceConfig::getString("ucentral.fileuploader.path","/tmp") + "/" + UUID;

			if(FileName.getSize()<(1000*uCentral::ServiceConfig::getInt("ucentral.fileuploader.maxsize",10000))) {

				std::ifstream f(FileName.path(), std::ios::binary);
				Poco::StreamCopier::copyStream(f, OL);
				/*
							"UUID			VARCHAR(64) PRIMARY KEY, "
							"Type			VARCHAR(32), "
							"Created 		BIGINT, "
							"FileContent	BYTEA"
				*/
				Poco::Data::Statement Insert(Sess);
				std::string FileType{"trace"};

				std::string St2{ "INSERT INTO FileUploads (UUID,Type,Created,FileContent) VALUES(?,?,?,?)"};

				Insert << ConvertParams(St2),
					Poco::Data::Keywords::use(UUID),
					Poco::Data::Keywords::use(FileType),
					Poco::Data::Keywords::use(Now),
					Poco::Data::Keywords::use(L);
				Insert.execute();

				FileName.remove();

				return true;
			} else {
				Logger_.warning(Poco::format("File %s is too large.",FileName.path()));
			}
        }
        catch ( const Poco::Exception & E)
        {
			Logger_.log(E);
            Logger_.warning(Poco::format("Could not update outstanding command %s for file upload completion",UUID));
        }
        return false;
    }

	bool Service::GetAttachedFile(std::string &UUID, const std::string &FileName) {
		try {
			Poco::Data::LOB<char>		L;
			Poco::Data::LOBInputStream	IL(L);

			/*
						"UUID			VARCHAR(64) PRIMARY KEY, "
						"Type			VARCHAR(32), "
						"Created 		BIGINT, "
						"FileContent	BYTEA"
			*/
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Select(Sess);

			std::string St2{ "SELECT FileContent FROM FileUploads WHERE UUID=?"};

			Select << ConvertParams(St2),
				Poco::Data::Keywords::into(L),
				Poco::Data::Keywords::use(UUID);
			Select.execute();

			std::ofstream f(FileName, std::ios::binary);
			Poco::StreamCopier::copyStream(IL, f);

			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	/*
		Sess << "CREATE TABLE IF NOT EXISTS BlackList ("
					"SerialNumber	VARCHAR(30) PRIMARY KEY, "
					"Reason			TEXT, "
					"Created		BIGINT, "
					"Author			VARCHAR(64)"
					")", Poco::Data::Keywords::now;
	 */

	bool Service::AddBlackListDevices(std::vector<uCentralBlackListedDevice> &  Devices) {
		try {

			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Insert(Sess);

			for(auto &i:Devices) {
				std::string St{"INSERT INTO BlackList (SerialNumber, Reason, Author, Created) "
				"VALUES(?,?,?,?)"};
				std::cout << "STATEMENT:" << St << std::endl;
				Insert << ConvertParams(St),
					Poco::Data::Keywords::use(i.SerialNumber),
					Poco::Data::Keywords::use(i.Reason),
					Poco::Data::Keywords::use(i.Author),
					Poco::Data::Keywords::use(i.Created) ;
				Insert.execute();
			}
			return true;
		} catch ( const Poco::Exception & E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::DeleteBlackListDevice(std::string & SerialNumber)  {
		try {
			Poco::Data::Session Sess = Pool_->get();
			Poco::Data::Statement Delete(Sess);

			std::string St{"DELETE FROM BlackList WHERE SerialNumber=?"};

			Delete << ConvertParams(St),
				Poco::Data::Keywords::use(SerialNumber);
			Delete.execute();

			return true;
		} catch ( const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::GetBlackListDevices(uint64_t Offset, uint64_t HowMany, std::vector<uCentralBlackListedDevice> & Devices )  {
		try {
			using tuple_list = Poco::Tuple<
									std::string,
									std::string,
									std::string,
									uint64_t>;
			using record_list = std::vector<tuple_list>;

			record_list Records;

			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement	Select(Sess);

			Select << "SELECT SerialNumber, Reason, Author, Created FROM BlackList" ,
				Poco::Data::Keywords::into(Records),
				Poco::Data::Keywords::range(Offset, Offset + HowMany - 1);
			Select.execute();

			for (auto i: Records) {
				uCentralBlackListedDevice R{
					.SerialNumber = i.get<0>(),
					.Reason = i.get<1>(),
					.Author = i.get<2>(),
					.Created = i.get<3>()};
				Devices.push_back(R);
			}
			return true;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}

	bool Service::IsBlackListed(std::string & SerialNumber) {
		try {
			Poco::Data::Session 	Sess = Pool_->get();
			Poco::Data::Statement	Select(Sess);

			std::string TmpSerialNumber;

			std::string St{"SELECT SerialNumber FROM BlackList WHERE SerialNumber=?"};

			Select << ConvertParams(St),
				Poco::Data::Keywords::into(TmpSerialNumber),
				Poco::Data::Keywords::use(SerialNumber) ;
			Select.execute();

			return !TmpSerialNumber.empty();

		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		return false;
	}


};

// namespace