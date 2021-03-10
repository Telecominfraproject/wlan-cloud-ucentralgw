//
// Created by stephane bourque on 2021-03-01.
//

#ifndef UCENTRAL_USTORAGESERVICE_H
#define UCENTRAL_USTORAGESERVICE_H

#include "SubSystemServer.h"

#include "Poco/Data/Session.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/SQLite/SQLite.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/PostgreSQL/SessionHandle.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/MySQL/SessionHandle.h"
#include "Poco/Data/ODBC/Connector.h"

#include "RESTAPI_Objects.h"

namespace uCentral::Storage {

    class Service : public SubSystemServer {

    public:
        Service() noexcept;

        int Start() override;

        void Stop() override;

        bool AddStatisticsData(std::string &SerialNUmber, uint64_t CfgUUID, std::string &NewStats);
        bool GetStatisticsData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                               std::vector<uCentralStatistics> &Stats);
        bool DeleteStatisticsData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate );

        bool UpdateDeviceConfiguration(std::string &SerialNUmber, std::string &Configuration);
        bool CreateDevice(uCentralDevice &);
        bool GetDevice(std::string &SerialNUmber, uCentralDevice &);
        uint64_t GetDevices(uint64_t From, uint64_t Howmany, std::vector<uCentralDevice> &Devices);
        bool DeleteDevice(std::string &SerialNUmber);
        bool UpdateDevice(uCentralDevice &);

        bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);

        bool UpdateDeviceCapabilities(std::string &SerialNUmber, std::string &State);
        bool GetDeviceCapabilities(std::string &SerialNUmber, uCentralCapabilities &);
        bool DeleteDeviceCapabilities(std::string & SerialNumber);

        bool AddLog(std::string & SerialNumber, std::string & Log);
        bool GetLogData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                               std::vector<uCentralDeviceLog> &Stats);
        bool DeleteLogData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate);

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

    private:
        std::mutex mutex_;
        static Service *instance_;
        std::shared_ptr<Poco::Data::SessionPool> Pool_;
        std::shared_ptr<Poco::Data::SQLite::Connector>      SQLiteConn_;
        std::shared_ptr<Poco::Data::PostgreSQL::Connector>  PostgresConn_;
        std::shared_ptr<Poco::Data::MySQL::Connector>       MySQLConn_;
    };

};  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
