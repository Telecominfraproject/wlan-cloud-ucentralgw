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
        SubSystemServer("Storage","storage","storage")
{
}

int uStorageService::start() {

    SubSystemServer::logger().information("Starting.");
    Poco::Data::SQLite::Connector::registerConnector();

    std::string TableLocation = uCentral::instance().config().getString("sqlite.location") + "/devices.db";

    session_ = std::shared_ptr<Poco::Data::Session>(new Poco::Data::Session("SQLite",TableLocation));

    *session_ << "CREATE TABLE IF NOT EXISTS Statistics (SerialNumber VARCHAR(30), CfgUUID INTEGER, Stats BLOB, Recorded DATETIME)", now;
    *session_ << "CREATE INDEX IF NOT EXISTS serial ON Statistics (SerialNumber ASC, Recorded ASC)", now;

    *session_ << "CREATE TABLE IF NOT EXISTS Devices (SerialNumber VARCHAR(30) UNIQUE PRIMARY KEY, MAC VARCHAR(30), CfgUUID INTEGER, Configuration BLOB, Created DATETIME, Updated DATETIME, LastDownloaded DATETIME ) WITHOUT ROWID" , now ;

    *session_ << "CREATE TABLE IF NOT EXISTS Capabilities (SerialNumber VARCHAR(30) PRIMARY KEY, Capabilities BLOB, Created DATETIME, Updated DATETIME) WITHOUT ROWID" , now ;

    Poco::Data::SQLite::Connector::registerConnector();

    return 0;
}

void uStorageService::stop() {
    SubSystemServer::logger().information("Stopping.");
}

bool uStorageService::AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats)
{
    std::lock_guard<std::mutex> guard(mutex_);

    Poco::DateTime Now;
    *session_ << "INSERT INTO Statistics VALUES(?, ?, ?, ?)",
        use(SerialNumber),
        use(CfgUUID),
        use(NewStats),
        use(Now), now;

    return true;
}

bool uStorageService::GetStatisticsData(std::string &SerialNumber, uint32_t From, uint32_t To, std::vector<uCentralStatistics> &Stats)
{
    return false;
}

bool uStorageService::UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration ){
    std::lock_guard<std::mutex> guard(mutex_);

    return false;
}

bool uStorageService::CreateDevice(uCentralDevice & DeviceDetails){
    std::lock_guard<std::mutex> guard(mutex_);

    std::string SerialNumber;

    *session_ << "SELECT SerialNumber FROM Devices WHERE SerialNumber=?" ,
        into(SerialNumber),
        use(DeviceDetails.SerialNumber), now;

    if(SerialNumber.empty())
    {
        *session_ << "INSERT INTO Devices VALUES(?, ?, ?, ?, ?, ?, ?)",
                use(DeviceDetails.SerialNumber),
                use(DeviceDetails.MAC),
                use(DeviceDetails.UUID),
                use(DeviceDetails.Configuration),
                use(DeviceDetails.CreationTimestamp),
                use(DeviceDetails.ModifiedTimestamp),
                use(DeviceDetails.LastDownloadTimeStamp), now;

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

bool uStorageService::GetDevice(std::string &SerialNUmber, uCentralDevice &DeviceDetails)
{
    std::lock_guard<std::mutex> guard(mutex_);

    *session_ << "SELECT SerialNumber, MAC, CfgUUID, Configuration, Created, Updated, LastDownloaded FROM Devices WHERE SerialNumber=?" ,
        into(DeviceDetails.SerialNumber),
        into(DeviceDetails.MAC),
        into(DeviceDetails.UUID),
        into(DeviceDetails.Configuration),
        into(DeviceDetails.CreationTimestamp),
        into(DeviceDetails.ModifiedTimestamp),
        into(DeviceDetails.LastDownloadTimeStamp),
        use(SerialNUmber), now;

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
        std::cout << "Adding capabilities for " << SerialNumber << std::endl;
        *session_ << "INSERT INTO Capabilities VALUES(?, ?, ?, ?)" ,
            use(SerialNumber),
            use(Capabs),
            use(Now),
            use(Now), now;
        std::cout << "Done adding capabilities for " << SerialNumber << std::endl;
    }
    else {
        std::cout << "Updating capabilities for " << SerialNumber << std::endl;
        *session_ << "UPDATE Capabilities SET Capabilities=?, Updated=? WHERE SerialNumber=?" ,
            use(Capabs),
            use(Now),
            use(SerialNumber), now;
        std::cout << "Done updating capabilities for " << SerialNumber << std::endl;
    }

    return true;
}

bool uStorageService::GetDeviceCapabilities(std::string &SerialNUmber, uCentralCapabilities & Caps)
{
    *session_ << "SELECT SerialNumber, Capabilities, Created, Updated FROM Capabilities WHERE SerialNumber=?" ,
        into(Caps.SerialNumber),
        into(Caps.Caps),
        into(Caps.CreationTimestamp),
        into(Caps.LastUpdatedTimestamp),
        use(SerialNUmber), now;

    if(Caps.SerialNumber.empty())
        return false;

    return true;
}

bool uStorageService::NewerConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t & UUID){

    std::lock_guard<std::mutex> guard(mutex_);

    std::string SS;

    *session_ << "SELECT SerialNumber, CfgUUID, Configuration  FROM Capabilities WHERE SerialNumber=?" ,
        into(SS),
        into(UUID),
        into(NewConfig),
        use(SerialNumber),now ;

    if(SS.empty() || CurrentConfig >= UUID)
        return false;

    return true;
}
