//
// Created by stephane bourque on 2021-03-01.
//

#include "uStorageService.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/PostgreSQL/SessionHandle.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/MySQL/SessionHandle.h"
#include "Poco/Data/ODBC/Connector.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/DateTime.h"

#include "uCentral.h"
#include "uCentralConfig.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
using Poco::Data::RecordSet;

namespace uCentral::Storage {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("Storage", "STORAGE-SVR", "storage"),
            Pool_(nullptr),
            SQLiteConn_(nullptr),
            PostgresConn_(nullptr),
            MySQLConn_(nullptr)
            {
    }

    int Start() {
        return uCentral::Storage::Service::instance()->Start();
    }

    void Stop() {
        uCentral::Storage::Service::instance()->Stop();
    }

    bool AddLog(std::string & SerialNumber, const std::string & Log) {
        return uCentral::Storage::Service::instance()->AddLog(SerialNumber,Log);
    }

    bool AddLog(std::string & SerialNumber, const uCentralDeviceLog & DeviceLog) {
        return uCentral::Storage::Service::instance()->AddLog(SerialNumber,DeviceLog);
    }

    bool AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats) {
        return uCentral::Storage::Service::instance()->AddStatisticsData(SerialNumber, CfgUUID, NewStats);
    }

    bool GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentralStatistics> &Stats) {
        return uCentral::Storage::Service::instance()->GetStatisticsData(SerialNumber, FromDate, ToDate, Offset, HowMany, Stats);
    }

    bool DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate ) {
        return uCentral::Storage::Service::instance()->DeleteStatisticsData(SerialNumber, FromDate, ToDate );
    }

    bool AddHealthCheckData(std::string &SerialNumber, const uCentralHealthcheck & Check) {
        return uCentral::Storage::Service::instance()->AddHealthCheckData(SerialNumber, Check);
    }

    bool GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                                   std::vector<uCentralHealthcheck> &Checks) {
        return uCentral::Storage::Service::instance()->GetHealthCheckData(SerialNumber, FromDate, ToDate, Offset, HowMany,
                Checks);
    }
    bool DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate ) {
        return uCentral::Storage::Service::instance()->DeleteHealthCheckData(SerialNumber, FromDate, ToDate );
    }

    bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration) {
        return uCentral::Storage::Service::instance()->UpdateDeviceConfiguration(SerialNumber, Configuration);
    }

    bool CreateDevice(uCentralDevice &Device) {
        return uCentral::Storage::Service::instance()->CreateDevice(Device);
    }

    bool CreateDefaultDevice(const std::string & SerialNumber, const std::string & Capabilities) {
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

    bool DeviceExists(const std::string & SerialNumber) {
        return uCentral::Storage::Service::instance()->DeviceExists(SerialNumber);
    }

    bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &NewerUUID) {
        return uCentral::Storage::Service::instance()->ExistingConfiguration(SerialNumber, CurrentConfig, NewConfig, NewerUUID);
    }

    bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &State) {
        return uCentral::Storage::Service::instance()->UpdateDeviceCapabilities(SerialNumber, State);
    }

    bool GetDeviceCapabilities(std::string &SerialNumber, uCentralCapabilities & Capabilities) {
        return uCentral::Storage::Service::instance()->GetDeviceCapabilities(SerialNumber, Capabilities);
    }

    bool DeleteDeviceCapabilities(std::string & SerialNumber) {
        return uCentral::Storage::Service::instance()->DeleteDeviceCapabilities(SerialNumber);
    }

    bool GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentralDeviceLog> &Stats) {
        return uCentral::Storage::Service::instance()->GetLogData(SerialNumber, FromDate, ToDate, Offset, HowMany, Stats);
    }

    bool DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        return uCentral::Storage::Service::instance()->DeleteLogData(SerialNumber, FromDate, ToDate);
    }

    std::string SerialToMAC(const std::string & Serial) {
        std::string Result;

        if(Serial.size()==12)
            Result= Serial[0] + Serial [1] + ':' + Serial[2] + Serial[3] + ':' + Serial[4] + Serial[5] + ':' +
                    Serial[6] + Serial [7] + ':' + Serial[8] + Serial[9] + ':' + Serial[10] + Serial[11];

        return Result;
    }

    int Service::Setup_MySQL() {

        auto NumSessions = uCentral::ServiceConfig::getInt("storage.type.mysql.maxsessions",64);
        auto IdleTime = uCentral::ServiceConfig::getInt("storage.type.mysql.idletime",60);
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

        MySQLConn_ = std::shared_ptr<Poco::Data::MySQL::Connector>(new Poco::Data::MySQL::Connector);
        MySQLConn_->registerConnector();
        Pool_ = std::shared_ptr<Poco::Data::SessionPool>(
                new Poco::Data::SessionPool(MySQLConn_->name(), ConnectionStr,4,NumSessions,IdleTime));

        Session session_ = Pool_->get();

        session_ << "CREATE TABLE IF NOT EXISTS Statistics ("
                    "SerialNumber VARCHAR(30), "
                    "UUID INTEGER, "
                    "Data TEXT, "
                    "Recorded BIGINT, "
                    "INDEX StatSerial (SerialNumber ASC, Recorded ASC))", now;

        session_ << "CREATE TABLE IF NOT EXISTS Devices ("
                    "SerialNumber  VARCHAR(30) UNIQUE PRIMARY KEY, "
                    "DeviceType    VARCHAR(10), "
                    "MACAddress    VARCHAR(30), "
                    "Manufacturer  VARCHAR(64), "
                    "UUID          BIGINT, "
                    "Configuration TEXT, "
                    "Notes         TEXT, "
                    "CreationTimestamp BIGINT, "
                    "LastConfigurationChange BIGINT, "
                    "LastConfigurationDownload BIGINT"
                    ")", now;

        session_ << "CREATE TABLE IF NOT EXISTS Capabilities ("
                    "SerialNumber VARCHAR(30) PRIMARY KEY, "
                    "Capabilities TEXT, "
                    "FirstUpdate BIGINT, "
                    "LastUpdate BIGINT"
                    ")", now;

        session_ << "CREATE TABLE IF NOT EXISTS DeviceLogs ("
                    "SerialNumber VARCHAR(30), "
                    "Log TEXT, "
                    "Severity BIGINT , "
                    "Data TEXT , "
                    "Recorded BIGINT, "
                    "INDEX LogSerial (SerialNumber ASC, Recorded ASC)"
                    ")", now;

        session_ << "CREATE TABLE IF NOT EXISTS HealthChecks ("
                    "SerialNumber VARCHAR(30), "
                    "UUID          BIGINT, "
                    "Data TEXT, "
                    "Sanity BIGINT , "
                    "Recorded BIGINT, "
                    "INDEX HealthSerial (SerialNumber ASC, Recorded ASC)"
                    ")", now;
        return 0;
    }

    int Service::Setup_SQLite() {
        auto DBName = uCentral::ServiceConfig::getString("storage.type.sqlite.db");
        auto NumSessions = uCentral::ServiceConfig::getInt("storage.type.sqlite.maxsessions",64);
        auto IdleTime = uCentral::ServiceConfig::getInt("storage.type.sqlite.idletime",60);

        SQLiteConn_ = std::shared_ptr<Poco::Data::SQLite::Connector>(new Poco::Data::SQLite::Connector);
        SQLiteConn_->registerConnector();
        Pool_ = std::shared_ptr<Poco::Data::SessionPool>(
                new Poco::Data::SessionPool(SQLiteConn_->name(), DBName,4,NumSessions,IdleTime));

        Session session_ = Pool_->get();

        session_ << "CREATE TABLE IF NOT EXISTS Statistics ("
                    "SerialNumber VARCHAR(30), "
                    "UUID INTEGER, "
                    "Data TEXT, "
                    "Recorded BIGINT)", now;
        session_ << "CREATE INDEX IF NOT EXISTS StatsSerial ON Statistics (SerialNumber ASC, Recorded ASC)", now;

        session_ << "CREATE TABLE IF NOT EXISTS Devices ("
                    "SerialNumber  VARCHAR(30) UNIQUE PRIMARY KEY, "
                    "DeviceType    VARCHAR(10), "
                    "MACAddress    VARCHAR(30), "
                    "Manufacturer  VARCHAR(64), "
                    "UUID          BIGINT, "
                    "Configuration TEXT, "
                    "Notes         TEXT, "
                    "CreationTimestamp BIGINT, "
                    "LastConfigurationChange BIGINT, "
                    "LastConfigurationDownload BIGINT"
                    ")", now;

        session_ << "CREATE TABLE IF NOT EXISTS Capabilities ("
                    "SerialNumber VARCHAR(30) PRIMARY KEY, "
                    "Capabilities TEXT, "
                    "FirstUpdate BIGINT, "
                    "LastUpdate BIGINT"
                    ")", now;

        session_ << "CREATE TABLE IF NOT EXISTS DeviceLogs ("
                    "SerialNumber VARCHAR(30), "
                    "Log TEXT, "
                    "Severity BIGINT , "
                    "Data TEXT , "
                    "Recorded BIGINT)", now;

        session_ << "CREATE INDEX IF NOT EXISTS LogSerial ON DeviceLogs (SerialNumber ASC, Recorded ASC)", now;

        session_ << "CREATE TABLE IF NOT EXISTS HealthChecks ("
                    "SerialNumber VARCHAR(30), "
                    "UUID          BIGINT, "
                    "Data TEXT, "
                    "Sanity BIGINT , "
                    "Recorded BIGINT) ", now;

        session_ << "CREATE INDEX IF NOT EXISTS HealthSerial ON HealthChecks (SerialNumber ASC, Recorded ASC)", now;

        return 0;
    }

    int Service::Setup_PostgreSQL() {
        auto NumSessions = uCentral::ServiceConfig::getInt("storage.type.postgresql.maxsessions",64);
        auto IdleTime = uCentral::ServiceConfig::getInt("storage.type.postgresql.idletime",60);
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

        PostgresConn_ = std::shared_ptr<Poco::Data::PostgreSQL::Connector>(new Poco::Data::PostgreSQL::Connector);
        PostgresConn_->registerConnector();
        Pool_ = std::shared_ptr<Poco::Data::SessionPool>(
                new Poco::Data::SessionPool(PostgresConn_->name(), ConnectionStr,4,NumSessions,IdleTime));

        Session session_ = Pool_->get();

        session_ << "CREATE TABLE IF NOT EXISTS Statistics ("
                    "SerialNumber VARCHAR(30), "
                    "UUID INTEGER, "
                    "Data TEXT, "
                    "Recorded BIGINT)", now;

        session_ << "CREATE INDEX IF NOT EXISTS StatsSerial ON Statistics (SerialNumber ASC, Recorded ASC)", now;

        session_ << "CREATE TABLE IF NOT EXISTS Devices ("
                    "SerialNumber  VARCHAR(30) UNIQUE PRIMARY KEY, "
                    "DeviceType    VARCHAR(10), "
                    "MACAddress    VARCHAR(30), "
                    "Manufacturer  VARCHAR(64), "
                    "UUID          BIGINT, "
                    "Configuration TEXT, "
                    "Notes         TEXT, "
                    "CreationTimestamp BIGINT, "
                    "LastConfigurationChange BIGINT, "
                    "LastConfigurationDownload BIGINT"
                    ")", now;

        session_ << "CREATE TABLE IF NOT EXISTS Capabilities ("
                    "SerialNumber VARCHAR(30) PRIMARY KEY, "
                    "Capabilities TEXT, "
                    "FirstUpdate BIGINT, "
                    "LastUpdate BIGINT"
                    ")", now;

        session_ << "CREATE TABLE IF NOT EXISTS DeviceLogs ("
                    "SerialNumber VARCHAR(30), "
                    "Log TEXT, "
                    "Severity BIGINT , "
                    "Data TEXT , "
                    "Recorded BIGINT)", now;

        session_ << "CREATE INDEX IF NOT EXISTS LogSerial ON DeviceLogs (SerialNumber ASC, Recorded ASC)", now;

        session_ << "CREATE TABLE IF NOT EXISTS HealthChecks ("
                    "SerialNumber VARCHAR(30), "
                    "UUID          BIGINT, "
                    "Data TEXT, "
                    "Sanity BIGINT , "
                    "Recorded BIGINT)", now;

        session_ << "CREATE INDEX IF NOT EXISTS HealthSerial ON HealthChecks (SerialNumber ASC, Recorded ASC)", now;

        return 0;
    }

    int Service::Setup_ODBC() {
        auto NumSessions = uCentral::ServiceConfig::getInt("storage.type.postgresql.maxsessions",64);
        auto IdleTime = uCentral::ServiceConfig::getInt("storage.type.postgresql.idletime",60);
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

        ODBCConn_ = std::shared_ptr<Poco::Data::ODBC::Connector>(new Poco::Data::ODBC::Connector);
        ODBCConn_->registerConnector();
        Pool_ = std::shared_ptr<Poco::Data::SessionPool>(
                new Poco::Data::SessionPool(ODBCConn_->name(), ConnectionStr,4,NumSessions,IdleTime));

        Session session_ = Pool_->get();

        return 0;
    }

    int Service::Start() {
        std::lock_guard<std::mutex> guard(mutex_);

        Logger_.information("Starting.");
        std::string DBType = uCentral::ServiceConfig::getString("storage.type");

        if(DBType == "sqlite") {
            return Setup_SQLite();
        }
        else if(DBType == "postgresql") {
            return Setup_PostgreSQL();
        }
        else if(DBType == "mysql") {
            return Setup_MySQL();
        }
        else if(DBType == "odbc") {
            return Setup_ODBC();
        }
        return 0;
    }

    void Service::Stop() {
        Logger_.information("Stopping.");
    }

    bool Service::AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats) {

        uCentral::DeviceRegistry::SetStatistics(SerialNumber,NewStats);

        try {
            Logger_.information("Device:" + SerialNumber + " Stats size:" + std::to_string(NewStats.size()));

            // std::cout << "STATS:" << NewStats << std::endl;

            uint64_t Now = time(nullptr);
            Session session_ = Pool_->get();

            session_ << "INSERT INTO Statistics VALUES( '%s', '%Lu', '%s', '%Lu')",
                    SerialNumber.c_str(),
                    CfgUUID,
                    NewStats.c_str(),
                    Now, now;

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", __func__ , SerialNumber.c_str(), E.displayText()));
        }
        return false;
    }

    bool Service::GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                                    std::vector<uCentralStatistics> &Stats) {

        typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t> StatRecord;
        typedef std::vector<StatRecord> RecordList;

        // std::lock_guard<std::mutex> guard(mutex_);
        Session session_ = Pool_->get();

        try {
            RecordList Records;
            if(FromDate && ToDate) {
                session_
                        << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber='%s' AND Recorded>=%Lu AND Recorded<=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        FromDate,
                        ToDate,
                        range(Offset, Offset + HowMany - 1), now;
            } else if (FromDate) {
                session_
                        << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber='%s' AND Recorded>=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        FromDate,
                        range(Offset, Offset + HowMany - 1), now;
            } else if (ToDate) {
                session_
                        << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber='%s' AND Recorded<=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        ToDate,
                        range(Offset, Offset + HowMany - 1), now;
            }
            else {
                // range(Offset, Offset + HowMany - 1)
                session_
                        << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber='%s'",
                        into(Records),
                        SerialNumber.c_str(),
                        range(Offset, Offset + HowMany - 1), now;
            }

            for (auto i: Records) {
                uCentralStatistics R{
                        .UUID = i.get<1>(),
                        .Values = i.get<2>(),
                        .Recorded = i.get<3>()};
                Stats.push_back(R);
            }
            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__func__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        try {
            Session session_ = Pool_->get();

            if(FromDate && ToDate) {
                session_
                        << "DELETE FROM Statistics WHERE SerialNumber='%s' AND Recorded>=%Lu AND Recorded<=%Lu",
                        SerialNumber.c_str(),
                        FromDate,
                        ToDate, now;
            } else if (FromDate) {
                session_
                        << "DELETE FROM Statistics WHERE SerialNumber='%s' AND Recorded>=%Lu",
                        SerialNumber.c_str(),
                        FromDate, now;
            } else if (ToDate) {
                session_
                        << "DELETE FROM Statistics WHERE SerialNumber='%s' AND Recorded<=%Lu",
                        SerialNumber.c_str(),
                        ToDate, now;
            }
            else {
                session_
                        << "DELETE FROM Statistics WHERE SerialNumber='%s'",
                        SerialNumber.c_str(), now;
            }
            return true;
        }
        catch (const Poco::Exception & E ) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::AddHealthCheckData(std::string &SerialNumber, const uCentralHealthcheck & Check) {
        try {
            Logger_.information("Device:" + SerialNumber + " HealthCheck: sanity " + std::to_string(Check.Sanity));

            Session session_ = Pool_->get();

            session_ << "INSERT INTO HealthChecks VALUES( '%s', '%Lu' , '%s' , '%Lu' , '%Lu')",
                    SerialNumber.c_str(),
                    Check.UUID,
                    Check.Data.c_str(),
                    Check.Sanity,
                    Check.Recorded, now;

            return true;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", __func__ , SerialNumber.c_str(), E.displayText()));
        }
        return false;
    }

    bool Service::GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                            std::vector<uCentralHealthcheck> &Checks) {

        typedef Poco::Tuple<std::string, uint64_t , std::string, uint64_t, uint64_t> Record;
        typedef std::vector<Record> RecordList;

        // std::lock_guard<std::mutex> guard(mutex_);
        Session session_ = Pool_->get();

        try {
            RecordList Records;
            if (FromDate && ToDate) {
                session_
                        << "SELECT SerialNumber, UUID, Data, Sanity, Recorded FROM HealthChecks WHERE SerialNumber='%s' AND Recorded>=%Lu AND Recorded<=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        FromDate,
                        ToDate,
                        range(Offset, Offset + HowMany - 1), now;
            } else if (FromDate) {
                session_
                        << "SELECT SerialNumber, UUID, Data, Sanity, Recorded FROM HealthChecks WHERE SerialNumber='%s' AND Recorded>=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        FromDate,
                        range(Offset, Offset + HowMany - 1), now;
            } else if (ToDate) {
                session_
                        << "SELECT SerialNumber, UUID, Data, Sanity, Recorded FROM HealthChecks WHERE SerialNumber='%s' AND Recorded<=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        ToDate,
                        range(Offset, Offset + HowMany - 1), now;
            } else {
                // range(Offset, Offset + HowMany - 1)
                session_
                        << "SELECT SerialNumber, UUID, Data, Sanity, Recorded FROM HealthChecks WHERE SerialNumber='%s'",
                        into(Records),
                        SerialNumber.c_str(),
                        range(Offset, Offset + HowMany - 1), now;
            }

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
            Logger_.warning(Poco::format("%s(%s): Failed with: %s", __func__ , SerialNumber.c_str(), E.displayText()));
        }
        return false;
    }

    bool Service::DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate ) {
        try {
            Session session_ = Pool_->get();

            if(FromDate && ToDate) {
                session_
                        << "DELETE FROM HealthChecks WHERE SerialNumber='%s' AND Recorded>=%Lu AND Recorded<=%Lu",
                        SerialNumber.c_str(),
                        FromDate,
                        ToDate, now;
            } else if (FromDate) {
                session_
                        << "DELETE FROM HealthChecks WHERE SerialNumber='%s' AND Recorded>=%Lu",
                        SerialNumber.c_str(),
                        FromDate, now;
            } else if (ToDate) {
                session_
                        << "DELETE FROM HealthChecks WHERE SerialNumber='%s' AND Recorded<=%Lu",
                        SerialNumber.c_str(),
                        ToDate, now;
            }
            else {
                session_
                        << "DELETE FROM HealthChecks WHERE SerialNumber='%s'",
                        SerialNumber.c_str(), now;
            }
            return true;
        }
        catch (const Poco::Exception & E ) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::AddLog(std::string &SerialNumber, const uCentralDeviceLog &Log) {
        Session session_ = Pool_->get();

        try {
            session_ << "INSERT INTO DeviceLogs VALUES( '%s' , '%s' , '%Lu', '%s', '%Lu')",
                    SerialNumber.c_str(),
                    Log.Log.c_str(),
                    Log.Severity,
                    Log.Data.c_str(),
                    Log.Recorded, now;
            return true;
        }
        catch (const Poco::Exception & E ) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::AddLog(std::string &SerialNumber, const std::string &Log)
    {
        uCentralDeviceLog   DeviceLog;

        DeviceLog.Log = Log;
        DeviceLog.Data = "" ;
        DeviceLog.Severity = uCentralDeviceLog::Level::LOG_INFO;
        DeviceLog.Recorded = time(nullptr);

        return AddLog(SerialNumber,DeviceLog);
    }

    bool Service::GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                    std::vector<uCentralDeviceLog> &Stats) {
        typedef Poco::Tuple<std::string, uint64_t, uint64_t, std::string > StatRecord;
        typedef std::vector<StatRecord> RecordList;

        Session session_ = Pool_->get();

        try {
            RecordList Records;
            if(FromDate && ToDate) {
                session_
                        << "SELECT Log,Recorded,Severity,Data FROM DeviceLogs WHERE SerialNumber='%s' AND Recorded>=%Lu AND Recorded<=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        FromDate,
                        ToDate,
                        range(Offset, Offset + HowMany - 1), now;
            } else if (FromDate) {
                session_
                        << "SELECT Log,Recorded,Severity,Data FROM DeviceLogs WHERE SerialNumber='%s' AND Recorded>=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        FromDate,
                        range(Offset, Offset + HowMany - 1), now;
            } else if (ToDate) {
                session_
                        << "SELECT Log,Recorded,Severity,Data FROM DeviceLogs WHERE SerialNumber='%s' AND Recorded<=%Lu",
                        into(Records),
                        SerialNumber.c_str(),
                        ToDate,
                        range(Offset, Offset + HowMany - 1), now;
            }
            else {
                // range(Offset, Offset + HowMany - 1)
                session_
                        << "SELECT Log,Recorded,Severity,Data FROM DeviceLogs WHERE SerialNumber='%s'",
                        into(Records),
                        SerialNumber.c_str(),
                        range(Offset, Offset + HowMany - 1), now;
            }

            for (auto i: Records) {
                uCentralDeviceLog R{.Log = i.get<0>(),
                                    .Recorded = i.get<1>(),
                                    .Severity = i.get<2>(),
                                    .Data = i.get<3>() };
                Stats.push_back(R);
            }
            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate) {
        try {
            Session session_ = Pool_->get();

            if(FromDate && ToDate) {
                session_
                        << "DELETE FROM DeviceLogs WHERE SerialNumber='%s' AND Recorded>=%Lu AND Recorded<=%Lu",
                        SerialNumber.c_str(),
                        FromDate,
                        ToDate, now;
            } else if (FromDate) {
                session_
                        << "DELETE FROM DeviceLogs WHERE SerialNumber='%s' AND Recorded>=%Lu",
                        SerialNumber.c_str(),
                        FromDate, now;
            } else if (ToDate) {
                session_
                        << "DELETE FROM DeviceLogs WHERE SerialNumber='%s' AND Recorded<=%Lu",
                        SerialNumber.c_str(),
                        ToDate, now;
            }
            else {
                session_
                        << "DELETE FROM DeviceLogs WHERE SerialNumber='%s'",
                        SerialNumber.c_str(), now;
            }
            return true;
        }
        catch (const Poco::Exception & E ) {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::UpdateDeviceConfiguration(std::string &SerialNumber, std::string & Configuration) {
        try {
            uCentral::Config::Config    Cfg(Configuration);

            if(!Cfg.Valid())
                return false;

            Session session_ = Pool_->get();

            uint64_t CurrentUUID;

            session_ << "SELECT UUID FROM Devices WHERE SerialNumber='%s'",
                into(CurrentUUID),
                SerialNumber.c_str(), now;

            CurrentUUID++;

            if(Cfg.SetUUID(CurrentUUID)) {
                uint64_t Now = time(nullptr);

                std::string NewConfig = Cfg.get();

                session_
                        << "UPDATE Devices SET Configuration='%s', UUID=%Lu, LastConfigurationChange=%Lu WHERE SerialNumber='%s'",
                        NewConfig.c_str(),
                        CurrentUUID,
                        Now,
                        SerialNumber.c_str(), now;

                return true;
            }
            return false;
        }
        catch (const Poco::Exception &E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::CreateDevice(uCentralDevice &DeviceDetails) {
        // std::lock_guard<std::mutex> guard(mutex_);

        std::string SerialNumber;
        try {

            Session session_ = Pool_->get();

            session_ << "SELECT SerialNumber FROM Devices WHERE SerialNumber='%s'",
                    into(SerialNumber),
                    DeviceDetails.SerialNumber.c_str(), now;

            if (SerialNumber.empty()) {
                uCentral::Config::Config    Cfg(DeviceDetails.Configuration);

                if(Cfg.Valid() && Cfg.SetUUID(DeviceDetails.UUID)) {
                    DeviceDetails.Configuration = Cfg.get();
                    uint64_t Now = time(nullptr);

                    DeviceDetails.Print();

                    session_ << "INSERT INTO Devices VALUES('%s', '%s', '%s', '%s', %Lu, '%s', '%s', %Lu, %Lu, %Lu)",
                            DeviceDetails.SerialNumber.c_str(),
                            DeviceDetails.DeviceType.c_str(),
                            DeviceDetails.MACAddress.c_str(),
                            DeviceDetails.Manufacturer.c_str(),
                            DeviceDetails.UUID,
                            DeviceDetails.Configuration.c_str(),
                            DeviceDetails.Notes.c_str(),
                            Now,
                            Now,
                            Now, now;

                    return true;
                }
                else
                {
                    Logger_.warning("Cannot create device: invalid configuration.");
                    return false;
                }
            }
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::CreateDefaultDevice(const std::string & SerialNumber, const std::string & Capabilities) {

        uCentralDevice  D;
        uCentral::Config::Config    NewConfig;

        Logger_.information(Poco::format("AUTO-CREATION(%s)",SerialNumber.c_str()));
        uint64_t Now = time(nullptr);
        NewConfig.SetUUID(Now);

        uCentral::Config::Capabilities  Caps(Capabilities);

        D.SerialNumber = SerialNumber;
        D.DeviceType = Caps.DeviceType();
        D.MACAddress = SerialToMAC(SerialNumber);
        D.Manufacturer = Caps.Manufacturer();
        D.UUID = Now;
        D.Configuration = NewConfig.get();
        D.Notes = "auto created device.";
        D.CreationTimestamp = D.LastConfigurationDownload = D.LastConfigurationChange = Now;

        return CreateDevice(D);
    }


    bool Service::DeleteDevice(std::string &SerialNumber) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Session session_ = Pool_->get();

            session_ << "DELETE FROM Devices WHERE SerialNumber='%s'",
                    SerialNumber.c_str(), now;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::GetDevice(std::string &SerialNumber, uCentralDevice &DeviceDetails) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Session session_ = Pool_->get();

            session_ << "SELECT "
                         "SerialNumber, "
                         "DeviceType, "
                         "MACAddress, "
                         "Manufacturer, "
                         "UUID, "
                         "Configuration, "
                         "Notes, "
                         "CreationTimestamp, "
                         "LastConfigurationChange, "
                         "LastConfigurationDownload "
                         " FROM Devices WHERE SerialNumber='%s'",
                    into(DeviceDetails.SerialNumber),
                    into(DeviceDetails.DeviceType),
                    into(DeviceDetails.MACAddress),
                    into(DeviceDetails.Manufacturer),
                    into(DeviceDetails.UUID),
                    into(DeviceDetails.Configuration),
                    into(DeviceDetails.Notes),
                    into(DeviceDetails.CreationTimestamp),
                    into(DeviceDetails.LastConfigurationChange),
                    into(DeviceDetails.LastConfigurationDownload),
                    SerialNumber.c_str(), now;

            if (DeviceDetails.SerialNumber.empty())
                return false;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::DeviceExists(const std::string & SerialNumber) {
        try {
            Session session_ = Pool_->get();

            std::string Serial;

            session_ << "SELECT "
                        "SerialNumber "
                        " FROM Devices WHERE SerialNumber='%s'",
                    into(Serial),
                    SerialNumber.c_str(),
                    now;

            if (Serial.empty())
                return false;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::UpdateDevice(uCentralDevice &NewConfig) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Session session_ = Pool_->get();

            uint64_t Now = time(nullptr);

            session_ << "UPDATE Devices SET Manufacturer='%s', DeviceType='%s', MACAddress='%s', Notes='%s', LastConfigurationChange=%Lu  WHERE SerialNumber='%s'",
                    NewConfig.Manufacturer.c_str(),
                    NewConfig.DeviceType.c_str(),
                    NewConfig.MACAddress.c_str(),
                    NewConfig.Notes.c_str(),
                    Now,
                    NewConfig.SerialNumber.c_str(), now;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,NewConfig.SerialNumber.c_str(),E.displayText() ));
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
                uint64_t> DeviceRecord;
        typedef std::vector<DeviceRecord> RecordList;

        RecordList Records;

        try {
            Session session_ = Pool_->get();

            session_ << "SELECT "
                         "SerialNumber, "
                         "DeviceType, "
                         "MACAddress, "
                         "Manufacturer, "
                         "UUID, "
                         "Configuration, "
                         "Notes, "
                         "CreationTimestamp, "
                         "LastConfigurationChange, "
                         "LastConfigurationDownload "
                         " FROM Devices",
                    into(Records),
                    range(From, From + HowMany - 1), now;

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
                        .LastConfigurationDownload = i.get<9>()};

                Devices.push_back(R);
            }
            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s: Failed with: %s",__FUNCTION__,E.displayText() ));
        }
        return false;
    }

    bool Service::UpdateDeviceCapabilities(std::string &SerialNumber, std::string &Capabs) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            std::string SS;
            Session session_ = Pool_->get();

            session_ << "SELECT SerialNumber FROM Capabilities WHERE SerialNumber='%s'",
                into(SS),
                SerialNumber.c_str(), now;

            uint64_t Now = time(nullptr);

            if (SS.empty()) {
                logger().information("Adding capabilities for " + SerialNumber);
                session_ << "INSERT INTO Capabilities VALUES('%s', '%s', %Lu, %Lu)",
                        SerialNumber.c_str(),
                        Capabs.c_str(),
                        Now,
                        Now, now;
                Logger_.information("Done adding capabilities for " + SerialNumber);
            } else {
                Logger_.information("Updating capabilities for " + SerialNumber);
                session_ << "UPDATE Capabilities SET Capabilities='%s', LastUpdate=%Lu WHERE SerialNumber='%s'",
                        Capabs.c_str(),
                        Now,
                        SerialNumber.c_str(), now;
                Logger_.information("Done updating capabilities for " + SerialNumber);
            }
            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::GetDeviceCapabilities(std::string &SerialNumber, uCentralCapabilities &Caps) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Session session_ = Pool_->get();

            std::string SerialNumber;

            session_
                    << "SELECT SerialNumber, Capabilities, FirstUpdate, LastUpdate FROM Capabilities WHERE SerialNumber='%s'",
                    into(SerialNumber),
                    into(Caps.Capabilities),
                    into(Caps.FirstUpdate),
                    into(Caps.LastUpdate),
                    SerialNumber.c_str(), now;

            if (SerialNumber.empty())
                return false;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::DeleteDeviceCapabilities(std::string &SerialNumber) {
        // std::lock_guard<std::mutex> guard(mutex_);

        try {
            Session session_ = Pool_->get();

            session_ <<
                    "DELETE FROM Capabilities WHERE SerialNumber='%s'" ,
                    SerialNumber.c_str(), now;
            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

    bool Service::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &UUID) {
        // std::lock_guard<std::mutex> guard(mutex_);
        std::string SS;
        try {
            Session session_ = Pool_->get();

            session_ << "SELECT SerialNumber, UUID, Configuration FROM Devices WHERE SerialNumber='%s'",
                    into(SS),
                    into(UUID),
                    into(NewConfig),
                    SerialNumber.c_str(), now;

            if (SS.empty()) {
                return false;
            }

            //  Let's update the last downloaded time
            uint64_t Now = time(nullptr);
            session_ << "UPDATE Devices SET LastConfigurationDownload=%Lu WHERE SerialNumber='%s'",
                    Now,
                    SerialNumber.c_str(), now;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            Logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber.c_str(),E.displayText() ));
        }
        return false;
    }

};      // namespace