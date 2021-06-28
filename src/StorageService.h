//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_USTORAGESERVICE_H
#define UCENTRAL_USTORAGESERVICE_H

#include "Poco/Data/Session.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/SQLite/Connector.h"

#ifndef SMALL_BUILD
#include "Poco/Data/PostgreSQL/Connector.h"
#include "Poco/Data/MySQL/Connector.h"
#include "Poco/Data/ODBC/Connector.h"
#endif

#include "AuthService.h"
#include "RESTAPI_objects.h"
#include "SubSystemServer.h"

namespace uCentral {

    class Storage : public SubSystemServer {

    public:
		enum StorageType {
			sqlite,
			pgsql,
			mysql,
			odbc
		};

		enum CommandExecutionType {
			COMMAND_PENDING,
			COMMAND_EXECUTED,
			COMMAND_COMPLETED
		};

        static Storage *instance() {
            if (instance_ == nullptr) {
                instance_ = new Storage;
            }
            return instance_;
        }

		bool AddLog(std::string & SerialNumber, uCentral::Objects::DeviceLog & Log, bool CrashLog = false );
		bool AddLog(std::string & SerialNumber, uint64_t UUID, const std::string & Log );
		bool AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats);
		bool GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
							   std::vector<uCentral::Objects::Statistics> &Stats);
		bool DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );
		bool GetNewestStatisticsData(std::string &SerialNumber, uint64_t HowMany, std::vector<uCentral::Objects::Statistics> &Stats);

		bool AddHealthCheckData(std::string &SerialNumber, uCentral::Objects::HealthCheck & Check);
		bool GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
								std::vector<uCentral::Objects::HealthCheck> &Checks);
		bool DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );
		bool GetNewestHealthCheckData(std::string &SerialNumber, uint64_t HowMany,
									  std::vector<uCentral::Objects::HealthCheck> &Checks);

		bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t & NewUUID );

		bool CreateDevice(uCentral::Objects::Device &);
		bool CreateDefaultDevice(const std::string & SerialNumber, const std::string & Capabilities, std::string & Firmware, std::string & DevicePassword);

		bool GetDevice(std::string &SerialNumber, uCentral::Objects::Device &);
		bool GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::Device> &Devices);
		bool GetDevices(uint64_t From, uint64_t HowMany, const std::string & Select, std::vector<uCentral::Objects::Device> &Devices);
		bool DeleteDevice(std::string &SerialNumber);
		bool UpdateDevice(uCentral::Objects::Device &);
		bool DeviceExists(std::string & SerialNumber);
		bool SetOwner(std::string & SerialNumber, std::string & OwnerUUID);
		bool SetLocation(std::string & SerialNumber, std::string & LocationUUID);
		bool SetVenue(std::string & SerialNumber, std::string & OwnerUUID);
		bool SetConnectInfo(std::string &SerialNumber, std::string &Firmware, std::string &DevicePassword);
		bool GetDeviceCount(uint64_t & Count);
		bool GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> & SerialNumbers);
		bool SetDeviceCompatibility(std::string & SerialNumber, std::string & Compatible);
		bool GetDevicesWithoutFirmware(std::string &DeviceType, std::string &Version, std::vector<std::string> & SerialNumbers);
		bool GetDeviceFWUpdatePolicy(std::string & SerialNumber, std::string & Policy);


		bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);

		bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &State);
		bool GetDeviceCapabilities(std::string &SerialNumber, uCentral::Objects::Capabilities &);
		bool DeleteDeviceCapabilities(std::string & SerialNumber);

		bool GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
						std::vector<uCentral::Objects::DeviceLog> &Stats, uint64_t Type);
		bool DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Type);
		bool GetNewestLogData(std::string &SerialNumber, uint64_t HowMany, std::vector<uCentral::Objects::DeviceLog> &Stats, uint64_t Type);

		bool CreateDefaultConfiguration(std::string & name, uCentral::Objects::DefaultConfiguration & DefConfig);
		bool DeleteDefaultConfiguration(std::string & name);
		bool UpdateDefaultConfiguration(std::string & name, uCentral::Objects::DefaultConfiguration & DefConfig);
		bool GetDefaultConfiguration(std::string &name, uCentral::Objects::DefaultConfiguration & DefConfig);
		bool GetDefaultConfigurations(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::DefaultConfiguration> &Devices);
		bool FindDefaultConfigurationForModel(const std::string & Model, uCentral::Objects::DefaultConfiguration & DefConfig );

		bool AddCommand(std::string & SerialNumber, uCentral::Objects::CommandDetails & Command,CommandExecutionType Type);
		bool GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands);
		bool DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate);
		bool GetNonExecutedCommands( uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands );
		bool UpdateCommand( std::string &UUID, uCentral::Objects::CommandDetails & Command );
		bool GetCommand( std::string &UUID, uCentral::Objects::CommandDetails & Command );
		bool DeleteCommand( std::string &UUID );
		bool GetReadyToExecuteCommands( uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands );
		bool CommandExecuted(std::string & UUID);
		bool CommandCompleted(std::string & UUID, const Poco::JSON::Object::Ptr & ReturnVars, bool FullCommand);
		bool AttachFileToCommand(std::string & UUID);
		bool GetAttachedFile(std::string & UUID, std::string & SerialNumber, const std::string & FileName, std::string &Type);
		bool RemoveAttachedFile(std::string & UUID);
		bool SetCommandResult(std::string & UUID, std::string & Result);
		bool GetNewestCommands(std::string &SerialNumber, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands);
		bool SetCommandExecuted(std::string & CommandUUID);

		bool AddBlackListDevices(std::vector<uCentral::Objects::BlackListedDevice> &  Devices);
		bool DeleteBlackListDevice(std::string & SerialNumber);
		bool IsBlackListed(std::string & SerialNumber);
		bool GetBlackListDevices(uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::BlackListedDevice> & Devices );

		//	for usernames
		bool IdentityExists(std::string & Identity, uCentral::AuthService::ACCESS_TYPE Type);
		bool AddIdentity(std::string & Identity, std::string & Password, uCentral::AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		bool GetIdentity(std::string & Identity, std::string & Password, uCentral::AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		bool UpdateIdentity(std::string & Identity, std::string & Password, uCentral::AuthService::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		bool DeleteIdentity(std::string & Identity, uCentral::AuthService::ACCESS_TYPE Type);
		bool ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, uCentral::AuthService::ACCESS_TYPE Type);
		bool GetIdentityRights(std::string & Identity, uCentral::Objects::AclTemplate & ACL);

		bool SetLifetimeStats(std::string & SerialNumber, std::string & Stats);
		bool GetLifetimeStats(std::string & SerialNumber, std::string & Stats);
		bool ResetLifetimeStats(std::string & SerialNumber);

		int Create_Tables();

		int Create_Statistics();
		int Create_Devices();
		int Create_Capabilities();
		int Create_HealthChecks();
		int Create_Authentication();
		int Create_DeviceLogs();
		int Create_DefaultConfigs();
		int Create_CommandList();
		int Create_BlackList();
		int Create_FileUploads();
		int Create_FirmwareUpgrades();
		int Create_LifetimeStats();


		int 	Start() override;
		void 	Stop() override;
		int 	Setup_SQLite();
		[[nodiscard]] std::string ConvertParams(const std::string &S) const;

#ifndef SMALL_BUILD
		int 	Setup_MySQL();
		int 	Setup_PostgreSQL();
		int 	Setup_ODBC();
#endif

	  private:
		static Storage      								*instance_;
		std::unique_ptr<Poco::Data::SessionPool>        	Pool_= nullptr;
		StorageType 										dbType_ = sqlite;
		std::unique_ptr<Poco::Data::SQLite::Connector>  	SQLiteConn_= nullptr;
#ifndef SMALL_BUILD
		std::unique_ptr<Poco::Data::PostgreSQL::Connector>  PostgresConn_= nullptr;
		std::unique_ptr<Poco::Data::MySQL::Connector>       MySQLConn_= nullptr;
		std::unique_ptr<Poco::Data::ODBC::Connector>        ODBCConn_= nullptr;
#endif

		Storage() noexcept;
   };

   inline Storage * Storage() { return Storage::instance(); }

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
