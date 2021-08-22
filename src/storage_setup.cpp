//
// Created by stephane bourque on 2021-08-22.
//

#include "StorageService.h"
#include "Daemon.h"

namespace OpenWifi {

#ifdef	SMALL_BUILD
    int Service::Setup_MySQL() { Daemon()->exit(Poco::Util::Application::EXIT_CONFIG); return 0; }
    int Service::Setup_PostgreSQL() { Daemon()->exit(Poco::Util::Application::EXIT_CONFIG); return 0; }
#else

    int Storage::Setup_SQLite() {
        Logger_.notice("SQLite Storage enabled.");
        auto DBName = Daemon()->DataDir() + "/" + Daemon()->ConfigGetString("storage.type.sqlite.db");
        auto NumSessions = Daemon()->ConfigGetInt("storage.type.sqlite.maxsessions", 64);
        auto IdleTime = Daemon()->ConfigGetInt("storage.type.sqlite.idletime", 60);
        SQLiteConn_ = std::make_unique<Poco::Data::SQLite::Connector>();
        SQLiteConn_->registerConnector();
        Pool_ = std::make_unique<Poco::Data::SessionPool>(SQLiteConn_->name(), DBName, 4, NumSessions, IdleTime);
        return 0;
    }

    int Storage::Setup_MySQL() {

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

    int Storage::Setup_PostgreSQL() {
        Logger_.notice("PostgreSQL Storage enabled.");

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

