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

            Logger_.setLevel(Poco::Message::PRIO_NOTICE);
            Logger_.notice("Starting.");
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

        [[nodiscard]] inline std::string ComputeRange(uint64_t From, uint64_t HowMany) {
            if(dbType_==sqlite) {
                return " LIMIT " + std::to_string(From) + ", " + std::to_string(HowMany) + " ";
            } else if(dbType_==pgsql) {
                return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
            } else if(dbType_==mysql) {
                return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
            }
            return " LIMIT " + std::to_string(HowMany) + " OFFSET " + std::to_string(From) + " ";
        }

        inline std::string ConvertParams(const std::string & S) const {
            std::string R;
            R.reserve(S.size()*2+1);
            if(dbType_==pgsql) {
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

    private:
        inline int Setup_SQLite();
        inline int Setup_MySQL();
        inline int Setup_PostgreSQL();

    protected:
    	Poco::SharedPtr<Poco::Data::SessionPool>    Pool_;
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
        Logger_.notice("SQLite StorageClass enabled.");
        dbType_ = sqlite;
        auto DBName = MicroService::instance().DataDir() + "/" + MicroService::instance().ConfigGetString("storage.type.sqlite.db");
        auto NumSessions = MicroService::instance().ConfigGetInt("storage.type.sqlite.maxsessions", 64);
        auto IdleTime = MicroService::instance().ConfigGetInt("storage.type.sqlite.idletime", 60);
        SQLiteConn_.registerConnector();
        Pool_ = Poco::SharedPtr<Poco::Data::SessionPool>(new Poco::Data::SessionPool(SQLiteConn_.name(), DBName, 4, NumSessions, IdleTime));
        return 0;
    }

    inline int StorageClass::Setup_MySQL() {
        Logger_.notice("MySQL StorageClass enabled.");
        dbType_ = mysql;
        auto NumSessions = MicroService::instance().ConfigGetInt("storage.type.mysql.maxsessions", 64);
        auto IdleTime = MicroService::instance().ConfigGetInt("storage.type.mysql.idletime", 60);
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

        MySQLConn_.registerConnector();
        Pool_ = Poco::SharedPtr<Poco::Data::SessionPool>(new Poco::Data::SessionPool(MySQLConn_.name(), ConnectionStr, 4, NumSessions, IdleTime));

        return 0;
    }

    inline int StorageClass::Setup_PostgreSQL() {
        Logger_.notice("PostgreSQL StorageClass enabled.");
        dbType_ = pgsql;
        auto NumSessions = MicroService::instance().ConfigGetInt("storage.type.postgresql.maxsessions", 64);
        auto IdleTime = MicroService::instance().ConfigGetInt("storage.type.postgresql.idletime", 60);
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

        PostgresConn_.registerConnector();
        Pool_ = Poco::SharedPtr<Poco::Data::SessionPool>(new Poco::Data::SessionPool(PostgresConn_.name(), ConnectionStr, 4, NumSessions, IdleTime));

        return 0;
    }
#endif

}
