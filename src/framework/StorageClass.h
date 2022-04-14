//
// Created by stephane bourque on 2021-10-06.
//

#pragma once

#include "Poco/Data/Session.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/JSON/Object.h"

#ifndef SMALL_BUILD
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/MySQL/Connector.h"
#endif

#include "framework/MicroService.h"

namespace OpenWifi {
    enum DBType {
        sqlite,
        pgsql,
        mysql
    };

    class StorageClass : public SubSystemServer {
    public:
        StorageClass() noexcept:
            SubSystemServer("StorageClass", "STORAGE-SVR", "storage")
        {
        }

        int Start() override {
            std::lock_guard		Guard(Mutex_);

            Logger().setLevel(Poco::Message::PRIO_INFORMATION);
            Logger().notice("Starting.");
            std::string DBType = MicroService::instance().ConfigGetString("storage.type");

            if (DBType == "sqlite") {
                Setup_SQLite();
            } else if (DBType == "postgresql") {
                Setup_PostgreSQL();
            } else if (DBType == "mysql") {
                Setup_MySQL();
            }
            return 0;
        }

        void Stop() override {
            Pool_->shutdown();
        }

        DBType Type() const { return dbType_; };
    private:
        inline int Setup_SQLite();
        inline int Setup_MySQL();
        inline int Setup_PostgreSQL();

    protected:
    	std::unique_ptr<Poco::Data::SessionPool>    Pool_;
        Poco::Data::SQLite::Connector  	            SQLiteConn_;
        Poco::Data::PostgreSQL::Connector           PostgresConn_;
        Poco::Data::MySQL::Connector                MySQLConn_;
        DBType                                      dbType_ = sqlite;
    };

#ifdef	SMALL_BUILD
    int Service::Setup_MySQL() { Daemon()->exit(Poco::Util::Application::EXIT_CONFIG); return 0; }
    int Service::Setup_PostgreSQL() { Daemon()->exit(Poco::Util::Application::EXIT_CONFIG); return 0; }
#else

    inline int StorageClass::Setup_SQLite() {
        Logger().notice("SQLite StorageClass enabled.");
        dbType_ = sqlite;
        auto DBName = MicroService::instance().DataDir() + "/" + MicroService::instance().ConfigGetString("storage.type.sqlite.db");
        int NumSessions = (int) MicroService::instance().ConfigGetInt("storage.type.sqlite.maxsessions", 64);
        int IdleTime = (int) MicroService::instance().ConfigGetInt("storage.type.sqlite.idletime", 60);

        Poco::Data::SQLite::Connector::registerConnector();
//        Pool_ = std::make_unique<Poco::Data::SessionPool>(new Poco::Data::SessionPool(SQLiteConn_.name(), DBName, 8,
//                                                                                     (int)NumSessions, (int)IdleTime));
        Pool_ = std::make_unique<Poco::Data::SessionPool>(SQLiteConn_.name(), DBName, 8,
                                                                                     (int)NumSessions, (int)IdleTime);
        return 0;
    }

    inline int StorageClass::Setup_MySQL() {
        Logger().notice("MySQL StorageClass enabled.");
        dbType_ = mysql;
        int NumSessions = (int) MicroService::instance().ConfigGetInt("storage.type.mysql.maxsessions", 64);
        int IdleTime = (int) MicroService::instance().ConfigGetInt("storage.type.mysql.idletime", 60);
        auto Host = MicroService::instance().ConfigGetString("storage.type.mysql.host");
        auto Username = MicroService::instance().ConfigGetString("storage.type.mysql.username");
        auto Password = MicroService::instance().ConfigGetString("storage.type.mysql.password");
        auto Database = MicroService::instance().ConfigGetString("storage.type.mysql.database");
        auto Port = MicroService::instance().ConfigGetString("storage.type.mysql.port");

        std::string ConnectionStr =
                "host=" + Host +
                ";user=" + Username +
                ";password=" + Password +
                ";db=" + Database +
                ";port=" + Port +
                ";compress=true;auto-reconnect=true";

        Poco::Data::MySQL::Connector::registerConnector();
        Pool_ = std::make_unique<Poco::Data::SessionPool>(MySQLConn_.name(), ConnectionStr, 8, NumSessions, IdleTime);

        return 0;
    }

    inline int StorageClass::Setup_PostgreSQL() {
        Logger().notice("PostgreSQL StorageClass enabled.");
        dbType_ = pgsql;
        int NumSessions = (int) MicroService::instance().ConfigGetInt("storage.type.postgresql.maxsessions", 64);
        int IdleTime = (int) MicroService::instance().ConfigGetInt("storage.type.postgresql.idletime", 60);
        auto Host = MicroService::instance().ConfigGetString("storage.type.postgresql.host");
        auto Username = MicroService::instance().ConfigGetString("storage.type.postgresql.username");
        auto Password = MicroService::instance().ConfigGetString("storage.type.postgresql.password");
        auto Database = MicroService::instance().ConfigGetString("storage.type.postgresql.database");
        auto Port = MicroService::instance().ConfigGetString("storage.type.postgresql.port");
        auto ConnectionTimeout = MicroService::instance().ConfigGetString("storage.type.postgresql.connectiontimeout");

        std::string ConnectionStr =
                "host=" + Host +
                " user=" + Username +
                " password=" + Password +
                " dbname=" + Database +
                " port=" + Port +
                " connect_timeout=" + ConnectionTimeout;

        Poco::Data::PostgreSQL::Connector::registerConnector();
        Pool_ = std::make_unique<Poco::Data::SessionPool>(PostgresConn_.name(), ConnectionStr, 8, NumSessions, IdleTime);

        return 0;
    }
#endif

}
