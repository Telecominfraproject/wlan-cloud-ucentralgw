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

uStorageService * uStorageService::instance_= nullptr;

uStorageService::uStorageService() noexcept:
        SubSystemServer("Storage","STORAGE-SVR","storage")
{
}

int uStorageService::start() {

    SubSystemServer::logger().information("Starting.");
    Poco::Data::SQLite::Connector::registerConnector();

    std::string TableLocation = uCentral::instance().config().getString("sqlite.location") + "/devices.db";

    session_ = std::shared_ptr<Poco::Data::Session>(new Poco::Data::Session("SQLite",TableLocation));

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
                     ") WITHOUT ROWID" , now ;

    *session_ << "CREATE TABLE IF NOT EXISTS Capabilities ("
                     "SerialNumber VARCHAR(30) PRIMARY KEY, "
                     "Capabilities BLOB, "
                     "FirstUpdate DATETIME, "
                     "LastUpdate DATETIME"
                     ") WITHOUT ROWID" , now ;

    Poco::Data::SQLite::Connector::registerConnector();

    return 0;
}

void uStorageService::stop() {
    SubSystemServer::logger().information("Stopping.");
}

bool uStorageService::AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats)
{
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

bool uStorageService::GetStatisticsData(std::string &SerialNumber, uint32_t From, uint32_t HowMany, std::vector<uCentralStatistics> &Stats)
{
    typedef Poco::Tuple<std::string, uint64_t, std::string, uint64_t >  StatRecord;
    typedef std::vector<StatRecord>     RecordList;

    RecordList Records;
    *session_ << "SELECT SerialNumber, UUID, Data, Recorded FROM Statistics WHERE SerialNumber=?" ,
        into(Records),
        use(SerialNumber),
        range(From,From+HowMany), now;

    for(auto i: Records)
    {
        uCentralStatistics  R{  .SerialNumber = i.get<0>(),
                                .UUID = i.get<1>(),
                                .Data = i.get<2>(),
                                .Recorded = i.get<3>()};
        Stats.push_back(R);
    }

    return true;
}

bool uStorageService::UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration ){
    std::lock_guard<std::mutex> guard(mutex_);

    return false;
}

void SetConfigurationUUID( uint64_t UUID, std::string &Configuration ) {
    Parser  parser;

    Poco::Dynamic::Var result = parser.parse(Configuration);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
    Poco::DynamicStruct ds = *object;

    ds["uuid"] = UUID;

    std::ostringstream NewConfig;

    Poco::JSON::Stringifier stringifier;

    stringifier.condense(ds,NewConfig );

    // std::cout << "New Configuration:" << NewConfig.str() << std::endl;

    Configuration = NewConfig.str();
}

bool uStorageService::CreateDevice(uCentralDevice & DeviceDetails){
    std::lock_guard<std::mutex> guard(mutex_);

    std::string SerialNumber;

    *session_ << "SELECT SerialNumber FROM Devices WHERE SerialNumber=?" ,
        into(SerialNumber),
        use(DeviceDetails.SerialNumber), now;

    if(SerialNumber.empty())
    {
        // We need to make sure that the UUID field of the configuration contains the proper UUID

        SetConfigurationUUID( DeviceDetails.UUID, DeviceDetails.Configuration );
        uint64_t Now = time(nullptr);

        *session_ << "INSERT INTO Devices VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            use( DeviceDetails.SerialNumber),
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

bool uStorageService::DeleteDevice(std::string &SerialNumber){

    std::lock_guard<std::mutex> guard(mutex_);

    *session_ << "DELETE FROM Devices WHERE SerialNumber=?",
        use(SerialNumber), now;

    return true;
}

bool uStorageService::GetDevice(std::string &SerialNumber, uCentralDevice &DeviceDetails)
{
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
                 " FROM Devices WHERE SerialNumber=?" ,
            into( DeviceDetails.SerialNumber),
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

    if(DeviceDetails.SerialNumber.empty())
        return false;

    return true;
}

bool uStorageService::UpdateDeviceCapabilities(std::string &SerialNumber, std::string &Capabs)
{
    std::lock_guard<std::mutex> guard(mutex_);

    std::string SS;

    *session_ << "SELECT SerialNumber FROM Capabilities WHERE SerialNumber=?" , into(SS), use(SerialNumber),now ;

    Poco::DateTime  Now;

    if(SS.empty()) {
        logger().information("Adding capabilities for " + SerialNumber);
        *session_ << "INSERT INTO Capabilities VALUES(?, ?, ?, ?)" ,
            use(SerialNumber),
            use(Capabs),
            use(Now),
            use(Now), now;
        logger().information("Done adding capabilities for " + SerialNumber);
    }
    else {
        logger().information("Updating capabilities for " + SerialNumber);
        *session_ << "UPDATE Capabilities SET Capabilities=?, LastUpdate=? WHERE SerialNumber=?" ,
            use(Capabs),
            use(Now),
            use(SerialNumber), now;
        logger().information("Done updating capabilities for " + SerialNumber);
    }

    return true;
}

bool uStorageService::GetDeviceCapabilities(std::string &SerialNUmber, uCentralCapabilities & Caps)
{
    *session_ << "SELECT SerialNumber, Capabilities, FirstUpdate, LastUpdate FROM Capabilities WHERE SerialNumber=?" ,
        into(Caps.SerialNumber),
        into(Caps.Capabilities),
        into(Caps.FirstUpdate),
        into(Caps.LastUpdate),
        use(SerialNUmber), now;

    if(Caps.SerialNumber.empty())
        return false;

    return true;
}

bool uStorageService::ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t & UUID){

    std::lock_guard<std::mutex> guard(mutex_);

    std::string SS;

    *session_ << "SELECT SerialNumber, UUID, Configuration FROM Devices WHERE SerialNumber=?" ,
        into(SS),
        into(UUID),
        into(NewConfig),
        use(SerialNumber),now ;

    if(SS.empty()) {
        return false;
    }

    return true;
}
