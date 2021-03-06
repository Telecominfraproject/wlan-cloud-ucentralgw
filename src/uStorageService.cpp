//
// Created by stephane bourque on 2021-03-01.
//

#include "uStorageService.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/DateTime.h"

#include "uCentral.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
using Poco::Data::RecordSet;

namespace uCentral::Storage {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("Storage", "STORAGE-SVR", "storage") {
    }

    int Service::start() {

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

    void Service::stop() {
        SubSystemServer::logger().information("Stopping.");
    }

    bool Service::AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats) {
        std::lock_guard<std::mutex> guard(mutex_);

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

    bool Service::GetStatisticsData(std::string &SerialNumber, std::string & FromDate, std::string & ToDate, uint64_t Offset, uint64_t HowMany,
                                    std::vector<uCentralStatistics> &Stats) {

        std::lock_guard<std::mutex> guard(mutex_);

        typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t> StatRecord;
        typedef std::vector<StatRecord> RecordList;

        try {
            RecordList Records;
            *session_
                    << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber=? AND Recorded>=? AND Recorded<=?",
                    into(Records),
                    use(SerialNumber),
                    use(FromDate),
                    use(ToDate),
                    range(Offset, Offset + HowMany - 1), now;

            for (auto i: Records) {
                uCentralStatistics R{.SerialNumber = i.get<0>(),
                        .UUID = i.get<1>(),
                        .Data = i.get<2>(),
                        .Recorded = i.get<3>()};
                Stats.push_back(R);
            }
            return true;
        }
        catch (const Poco::Exception & Except ) {
            logger_.warning( "Invalid request to retrieve statistcis for " + SerialNumber);
        }

        return false;
    }

    bool Service::UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration) {
        std::lock_guard<std::mutex> guard(mutex_);

        return false;
    }

    void SetConfigurationUUID(uint64_t UUID, std::string &Configuration) {
        Parser parser;

        Poco::Dynamic::Var result = parser.parse(Configuration);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *object;

        ds["uuid"] = UUID;

        std::ostringstream NewConfig;

        Poco::JSON::Stringifier stringifier;

        stringifier.condense(ds, NewConfig);

        // std::cout << "New Configuration:" << NewConfig.str() << std::endl;

        Configuration = NewConfig.str();
    }

    bool Service::CreateDevice(uCentralDevice &DeviceDetails) {
        std::lock_guard<std::mutex> guard(mutex_);

        std::string SerialNumber;

        *session_ << "SELECT SerialNumber FROM Devices WHERE SerialNumber=?",
                into(SerialNumber),
                use(DeviceDetails.SerialNumber), now;

        if (SerialNumber.empty()) {
            SetConfigurationUUID(DeviceDetails.UUID, DeviceDetails.Configuration);
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

        return false;
    }

    bool Service::DeleteDevice(std::string &SerialNumber) {

        std::lock_guard<std::mutex> guard(mutex_);

        *session_ << "DELETE FROM Devices WHERE SerialNumber=?",
                use(SerialNumber), now;

        return true;
    }

    bool Service::GetDevice(std::string &SerialNumber, uCentralDevice &DeviceDetails) {
        std::lock_guard<std::mutex> guard(mutex_);

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
        catch (const Poco::Exception &Exc) {
            std::cout << Exc.displayText() << std::endl;
        }

        return Devices.size();

    }

    bool Service::UpdateDeviceCapabilities(std::string &SerialNumber, std::string &Capabs) {
        std::lock_guard<std::mutex> guard(mutex_);

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

    bool Service::GetDeviceCapabilities(std::string &SerialNUmber, uCentralCapabilities &Caps) {

        std::lock_guard<std::mutex> guard(mutex_);

        *session_
                << "SELECT SerialNumber, Capabilities, FirstUpdate, LastUpdate FROM Capabilities WHERE SerialNumber=?",
                into(Caps.SerialNumber),
                into(Caps.Capabilities),
                into(Caps.FirstUpdate),
                into(Caps.LastUpdate),
                use(SerialNUmber), now;

        if (Caps.SerialNumber.empty())
            return false;

        return true;
    }

    bool
    Service::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig,
                                           uint64_t &UUID) {

        std::lock_guard<std::mutex> guard(mutex_);
        std::string SS;
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

};      // namespace