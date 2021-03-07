//
// Created by stephane bourque on 2021-03-01.
//

#include "uStorageService.h"
#include "Poco/Data/SQLite/Connector.h"
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
            SubSystemServer("Storage", "STORAGE-SVR", "storage") {
    }

    int Service::Start() {

        SubSystemServer::logger().information("Starting.");
        Poco::Data::SQLite::Connector::registerConnector();

        std::string TableLocation = uCentral::Daemon::instance().config().getString("sqlite.location") + "/devices.db";

        session_ = std::shared_ptr<Poco::Data::Session>(new Poco::Data::Session("SQLite", TableLocation));

        *session_ << "CREATE TABLE IF NOT EXISTS Statistics ("
                     "SerialNumber VARCHAR(30), "
                     "UUID INTEGER, "
                     "Data BLOB, "
                     "Recorded DATETIME"
                     ")", now;

        *session_ << "CREATE INDEX IF NOT EXISTS serial ON Statistics (SerialNumber ASC, Recorded ASC)", now;

        *session_ << "CREATE TABLE IF NOT EXISTS Devices ("
                     "SerialNumber  VARCHAR(30) UNIQUE PRIMARY KEY, "
                     "DeviceType    VARCHAR(10), "
                     "MACAddress    VARCHAR(30), "
                     "Manufacturer  VARCHAR(64), "
                     "UUID          INTEGER, "
                     "Configuration BLOB, "
                     "Notes         BLOB, "
                     "CreationTimestamp DATETIME, "
                     "LastConfigurationChange DATETIME, "
                     "LastConfigurationDownload DATETIME"
                     ") WITHOUT ROWID", now;

        *session_ << "CREATE TABLE IF NOT EXISTS Capabilities ("
                     "SerialNumber VARCHAR(30) PRIMARY KEY, "
                     "Capabilities BLOB, "
                     "FirstUpdate DATETIME, "
                     "LastUpdate DATETIME"
                     ") WITHOUT ROWID", now;

        Poco::Data::SQLite::Connector::registerConnector();

        return 0;
    }

    void Service::Stop() {
        SubSystemServer::logger().information("Stopping.");
    }

    bool Service::AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats) {

        uCentral::DeviceRegistry::Service::instance()->SetStatistics(SerialNumber,NewStats);

        std::lock_guard<std::mutex> guard(mutex_);

        try {
            logger().information("Device:" + SerialNumber + " Stats size:" + std::to_string(NewStats.size()));

            // std::cout << "STATS:" << NewStats << std::endl;

            Poco::DateTime Now;
            *session_ << "INSERT INTO Statistics VALUES(?, ?, ?, ?)",
                    use(SerialNumber),
                    use(CfgUUID),
                    use(NewStats),
                    use(Now), now;

            return true;
        }
        catch (const Poco::Exception &E) {
            logger_.warning(Poco::format("%s(%s): Failed with: %s", __FUNCTION__, SerialNumber, E.displayText()));
        }
        return false;
    }

    bool Service::GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                                    std::vector<uCentralStatistics> &Stats) {

        typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t> StatRecord;
        typedef std::vector<StatRecord> RecordList;

        std::lock_guard<std::mutex> guard(mutex_);

        // std::cout << "GS:" << SerialNumber << " " << FromDate << " " << ToDate << " " << Offset << " " << HowMany << std::endl;

        try {
            RecordList Records;
            if(FromDate && ToDate) {
                *session_
                        << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber=? AND Recorded>=? AND Recorded<=?",
                        into(Records),
                        use(SerialNumber),
                        use(FromDate),
                        use(ToDate),
                        range(Offset, Offset + HowMany - 1), now;
            } else if (FromDate) {
                *session_
                        << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber=? AND Recorded>=?",
                        into(Records),
                        use(SerialNumber),
                        use(FromDate),
                        range(Offset, Offset + HowMany - 1), now;
            } else if (ToDate) {
                *session_
                        << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber=? AND Recorded<=?",
                        into(Records),
                        use(SerialNumber),
                        use(ToDate),
                        range(Offset, Offset + HowMany - 1), now;
            }
            else {
                // range(Offset, Offset + HowMany - 1)
                *session_
                        << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber=?",
                        into(Records),
                        use(SerialNumber),
                        range(Offset, Offset + HowMany - 1), now;
            }

            for (auto i: Records) {
                uCentralStatistics R{.SerialNumber = i.get<0>(),
                        .UUID = i.get<1>(),
                        .Values = i.get<2>(),
                        .Recorded = i.get<3>()};
                Stats.push_back(R);
            }
            return true;
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany) {
        std::lock_guard<std::mutex> guard(mutex_);

        try {
            if(FromDate && ToDate) {
                *session_
                        << "DELETE FROM Statistics WHERE SerialNumber=? AND Recorded>=? AND Recorded<=?",
                        use(SerialNumber),
                        use(FromDate),
                        use(ToDate),
                        range(Offset, Offset + HowMany - 1), now;
            } else if (FromDate) {
                *session_
                        << "DELETE FROM Statistics WHERE SerialNumber=? AND Recorded>=?",
                        use(SerialNumber),
                        use(FromDate),
                        range(Offset, Offset + HowMany - 1), now;
            } else if (ToDate) {
                *session_
                        << "DELETE FROM Statistics WHERE SerialNumber=? AND Recorded<=?",
                        use(SerialNumber),
                        use(ToDate),
                        range(Offset, Offset + HowMany - 1), now;
            }
            else {
                *session_
                        << "DELETE FROM Statistics WHERE SerialNumber=?",
                        use(SerialNumber),
                        range(Offset, Offset + HowMany - 1), now;
            }
            return true;
        }
        catch (const Poco::Exception & E ) {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::UpdateDeviceConfiguration(std::string &SerialNumber, std::string & Configuration) {
        std::lock_guard<std::mutex> guard(mutex_);

        try {
            uCentral::Config::Config    Cfg(Configuration);

            if(!Cfg.Valid())
                return false;

            uint64_t CurrentUUID;

            *session_ << "SELECT UUID FROM Devices WHERE SerialNumber=?",
                into(CurrentUUID),
                use(SerialNumber), now;

            CurrentUUID++;

            if(Cfg.SetUUID(CurrentUUID)) {
                uint64_t Now = time(nullptr);

                std::string NewConfig = Cfg.get();

                *session_
                        << "UPDATE Devices SET Configuration=?, UUID=?, LastConfigurationChange=? WHERE SerialNumber=?",
                        use(NewConfig),
                        use(CurrentUUID),
                        use(Now),
                        use(SerialNumber), now;

                return true;
            }
            return false;
        }
        catch (const Poco::Exception &E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::CreateDevice(uCentralDevice &DeviceDetails) {
        std::lock_guard<std::mutex> guard(mutex_);

        std::string SerialNumber;
        try {
            *session_ << "SELECT SerialNumber FROM Devices WHERE SerialNumber=?",
                    into(SerialNumber),
                    use(DeviceDetails.SerialNumber), now;

            if (SerialNumber.empty()) {
                uCentral::Config::Config    Cfg(DeviceDetails.Configuration);

                if(Cfg.Valid() && Cfg.SetUUID(DeviceDetails.UUID)) {
                    DeviceDetails.Configuration = Cfg.get();
                    uint64_t Now = time(nullptr);

                    *session_ << "INSERT INTO Devices VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                            use(DeviceDetails.SerialNumber),
                            use(DeviceDetails.DeviceType),
                            use(DeviceDetails.MACAddress),
                            use(DeviceDetails.Manufacturer),
                            use(DeviceDetails.UUID),
                            use(DeviceDetails.Configuration),
                            use(DeviceDetails.Notes),
                            use(Now),
                            use(Now),
                            use(Now), now;

                    return true;
                }
                else
                {
                    logger_.warning("Cannot create device: invalid configuration.");
                    return false;
                }
            }
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::DeleteDevice(std::string &SerialNumber) {

        std::lock_guard<std::mutex> guard(mutex_);

        try {
            *session_ << "DELETE FROM Devices WHERE SerialNumber=?",
                    use(SerialNumber), now;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::GetDevice(std::string &SerialNumber, uCentralDevice &DeviceDetails) {
        std::lock_guard<std::mutex> guard(mutex_);

        try {
            *session_ << "SELECT "
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
                         " FROM Devices WHERE SerialNumber=?",
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
                    use(SerialNumber), now;

            if (DeviceDetails.SerialNumber.empty())
                return false;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    uint64_t Service::GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentralDevice> &Devices) {

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

        std::lock_guard<std::mutex> guard(mutex_);

        RecordList Records;

        try {
            *session_ << "SELECT "
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
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s: Failed with: %s",__FUNCTION__,E.displayText() ));
        }
        return Devices.size();
    }

    bool Service::UpdateDeviceCapabilities(std::string &SerialNumber, std::string &Capabs) {
        std::lock_guard<std::mutex> guard(mutex_);

        try {
            std::string SS;

            *session_ << "SELECT SerialNumber FROM Capabilities WHERE SerialNumber=?", into(SS), use(SerialNumber), now;

            Poco::DateTime Now;

            if (SS.empty()) {
                logger().information("Adding capabilities for " + SerialNumber);
                *session_ << "INSERT INTO Capabilities VALUES(?, ?, ?, ?)",
                        use(SerialNumber),
                        use(Capabs),
                        use(Now),
                        use(Now), now;
                logger().information("Done adding capabilities for " + SerialNumber);
            } else {
                logger().information("Updating capabilities for " + SerialNumber);
                *session_ << "UPDATE Capabilities SET Capabilities=?, LastUpdate=? WHERE SerialNumber=?",
                        use(Capabs),
                        use(Now),
                        use(SerialNumber), now;
                logger().information("Done updating capabilities for " + SerialNumber);
            }
            return true;
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::GetDeviceCapabilities(std::string &SerialNumber, uCentralCapabilities &Caps) {
        std::lock_guard<std::mutex> guard(mutex_);


        try {
            *session_
                    << "SELECT SerialNumber, Capabilities, FirstUpdate, LastUpdate FROM Capabilities WHERE SerialNumber=?",
                    into(Caps.SerialNumber),
                    into(Caps.Capabilities),
                    into(Caps.FirstUpdate),
                    into(Caps.LastUpdate),
                    use(SerialNumber), now;

            if (Caps.SerialNumber.empty())
                return false;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::DeleteDeviceCapabilities(std::string &SerialNumber) {
        std::lock_guard<std::mutex> guard(mutex_);

        try {
            *session_ <<
                    "DELETE FROM Capabilities WHERE SerialNumber=?" ,
                    use(SerialNumber), now;
            return true;
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

    bool Service::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &UUID) {
        std::lock_guard<std::mutex> guard(mutex_);
        std::string SS;
        try {
            *session_ << "SELECT SerialNumber, UUID, Configuration FROM Devices WHERE SerialNumber=?",
                    into(SS),
                    into(UUID),
                    into(NewConfig),
                    use(SerialNumber), now;

            if (SS.empty()) {
                return false;
            }

            //  Let's update the last downloaded time
            uint64_t Now = time(nullptr);
            *session_ << "UPDATE Devices SET LastConfigurationDownload=? WHERE SerialNumber=?",
                    use(Now),
                    use(SerialNumber), now;

            return true;
        }
        catch( const Poco::Exception & E)
        {
            logger_.warning(Poco::format("%s(%s): Failed with: %s",__FUNCTION__,SerialNumber,E.displayText() ));
        }
        return false;
    }

};      // namespace