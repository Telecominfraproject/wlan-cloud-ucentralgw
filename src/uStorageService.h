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
#include "Poco/Data/ODBC/ODBC.h"

#include "RESTAPI_Objects.h"

namespace uCentral::Storage {

    int Start();
    void Stop();

    class Service : public SubSystemServer {

    public:
        Service() noexcept;

        friend int uCentral::Storage::Start();
        friend void uCentral::Storage::Stop();

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

        friend bool AddStatisticsData(std::string &SerialNUmber, uint64_t CfgUUID, std::string &NewStats);
        friend bool GetStatisticsData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                               std::vector<uCentralStatistics> &Stats);
        friend bool DeleteStatisticsData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate );
        friend bool UpdateDeviceConfiguration(std::string &SerialNUmber, std::string &Configuration);
        friend bool CreateDevice(uCentralDevice &);
        friend bool GetDevice(std::string &SerialNUmber, uCentralDevice &);
        friend bool GetDevices(uint64_t From, uint64_t Howmany, std::vector<uCentralDevice> &Devices);
        friend bool DeleteDevice(std::string &SerialNUmber);
        friend bool UpdateDevice(uCentralDevice &);
        friend bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);
        friend bool UpdateDeviceCapabilities(std::string &SerialNUmber, std::string &State);
        friend bool GetDeviceCapabilities(std::string &SerialNUmber, uCentralCapabilities &);
        friend bool DeleteDeviceCapabilities(std::string & SerialNumber);
        friend bool GetLogData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                               std::vector<uCentralDeviceLog> &Stats);
        friend bool DeleteLogData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate);
        friend bool AddLog(std::string & SerialNumber, std::string & Log);

    private:

        bool AddLog_i(std::string & SerialNumber, std::string & Log);
        bool AddStatisticsData_i(std::string &SerialNUmber, uint64_t CfgUUID, std::string &NewStats);
        bool GetStatisticsData_i(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                               std::vector<uCentralStatistics> &Stats);
        bool DeleteStatisticsData_i(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate );

        bool UpdateDeviceConfiguration_i(std::string &SerialNUmber, std::string &Configuration);
        bool CreateDevice_i(uCentralDevice &);
        bool GetDevice_i(std::string &SerialNUmber, uCentralDevice &);
        bool GetDevices_i(uint64_t From, uint64_t Howmany, std::vector<uCentralDevice> &Devices);
        bool DeleteDevice_i(std::string &SerialNUmber);
        bool UpdateDevice_i(uCentralDevice &);

        bool ExistingConfiguration_i(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);

        bool UpdateDeviceCapabilities_i(std::string &SerialNUmber, std::string &State);
        bool GetDeviceCapabilities_i(std::string &SerialNUmber, uCentralCapabilities &);
        bool DeleteDeviceCapabilities_i(std::string & SerialNumber);

        bool GetLogData_i(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                        std::vector<uCentralDeviceLog> &Stats);
        bool DeleteLogData_i(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate);


        int Start() override;
        void Stop() override;
        int Setup_MySQL();
        int Setup_SQLite();
        int Setup_PostgreSQL();
        int Setup_ODBC();

        std::mutex          mutex_;
        static Service      *instance_;
        std::shared_ptr<Poco::Data::SessionPool>            Pool_;
        std::shared_ptr<Poco::Data::SQLite::Connector>      SQLiteConn_;
        std::shared_ptr<Poco::Data::PostgreSQL::Connector>  PostgresConn_;
        std::shared_ptr<Poco::Data::MySQL::Connector>       MySQLConn_;
        std::shared_ptr<Poco::Data::ODBC::Connector>        ODBCConn_;
    };

    bool AddLog(std::string & SerialNumber, std::string & Log);
    bool AddStatisticsData(std::string &SerialNUmber, uint64_t CfgUUID, std::string &NewStats);
    bool GetStatisticsData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentralStatistics> &Stats);
    bool DeleteStatisticsData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate );
    bool UpdateDeviceConfiguration(std::string &SerialNUmber, std::string &Configuration);
    bool CreateDevice(uCentralDevice &);
    bool GetDevice(std::string &SerialNUmber, uCentralDevice &);
    bool GetDevices(uint64_t From, uint64_t Howmany, std::vector<uCentralDevice> &Devices);
    bool DeleteDevice(std::string &SerialNUmber);
    bool UpdateDevice(uCentralDevice &);
    bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);
    bool UpdateDeviceCapabilities(std::string &SerialNUmber, std::string &State);
    bool GetDeviceCapabilities(std::string &SerialNUmber, uCentralCapabilities &);
    bool DeleteDeviceCapabilities(std::string & SerialNumber);
    bool GetLogData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentralDeviceLog> &Stats);
    bool DeleteLogData(std::string &SerialNUmber, uint64_t FromDate, uint64_t ToDate);

};  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
