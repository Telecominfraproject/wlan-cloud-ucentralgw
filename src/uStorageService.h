//
// Created by stephane bourque on 2021-03-01.
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

#include "RESTAPI_objects.h"
#include "SubSystemServer.h"
#include "uAuthService.h"

namespace uCentral::Storage {

    int Start();
    void Stop();

    bool AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats);
    bool GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::Statistics> &Stats);
    bool DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );

    bool AddHealthCheckData(std::string &SerialNumber,uCentral::Objects::HealthCheck & Check);
    bool GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                            std::vector<uCentral::Objects::HealthCheck> &Checks);
    bool DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );

    bool AddLog(std::string & SerialNumber, uCentral::Objects::DeviceLog & Log, bool CrashLog = false );
    bool AddLog(std::string & SerialNumber, const std::string &Log );

    bool GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::DeviceLog> &Stats, uint64_t Type);
    bool DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Type);

    bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t & NewUUID );
    bool CreateDevice(uCentral::Objects::Device &);
    bool CreateDefaultDevice(const std::string & SerialNumber, const std::string & Capabilities);
    bool GetDevice(std::string &SerialNumber, uCentral::Objects::Device &);
    bool DeviceExists(std::string & SerialNumber);
    bool GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::Device> &Devices);
	bool GetDevices(uint64_t From, uint64_t HowMany, const std::string & Select, std::vector<uCentral::Objects::Device> &Devices);
    bool DeleteDevice(std::string &SerialNumber);
    bool UpdateDevice(uCentral::Objects::Device &);
    bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);
    bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &State);
    bool GetDeviceCapabilities(std::string &SerialNumber, uCentral::Objects::Capabilities &);
    bool DeleteDeviceCapabilities(std::string & SerialNumber);
	bool SetOwner(std::string & SerialNumber, std::string & OwnerUUID);
	bool SetLocation(std::string & SerialNumber, std::string & LocationUUID);
	bool SetFirmware(std::string & SerialNumber, std::string & Firmware );
	bool GetDeviceCount(uint64_t & Count);
	bool GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> & SerialNumbers);

    bool CreateDefaultConfiguration(std::string & name, uCentral::Objects::DefaultConfiguration & DefConfig);
    bool DeleteDefaultConfiguration(std::string & name);
    bool UpdateDefaultConfiguration(std::string & name, uCentral::Objects::DefaultConfiguration & DefConfig);
    bool GetDefaultConfiguration(std::string &name, uCentral::Objects::DefaultConfiguration & DefConfig);
    bool GetDefaultConfigurations(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::DefaultConfiguration> &Devices);

    bool AddCommand(std::string & SerialNumber, uCentral::Objects::CommandDetails & Command,bool AlreadyExecuted=false);
    bool GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands);
    bool DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate);
    bool GetNonExecutedCommands( uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands );
    bool UpdateCommand( std::string &UUID, uCentral::Objects::CommandDetails & Command );
    bool GetCommand( std::string &UUID, uCentral::Objects::CommandDetails & Command );
    bool DeleteCommand( std::string &UUID );
    bool GetReadyToExecuteCommands( uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands );
    bool CommandExecuted(std::string & UUID);
    bool CommandCompleted(std::string & UUID, Poco::DynamicStruct & ReturnVars, bool FullCommand);
    bool AttachFileToCommand(std::string & UUID);
	bool GetAttachedFile(std::string & UUID, const std::string & FileName);
	bool RemoveAttachedFile(std::string & UUID);
	bool SetCommandResult(std::string & UUID, std::string & Result);

	bool AddBlackListDevices(std::vector<uCentral::Objects::BlackListedDevice> &  Devices);
	bool DeleteBlackListDevice(std::string & SerialNumber);
	bool GetBlackListDevices(uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::BlackListedDevice> & Devices );
	bool IsBlackListed(std::string & SerialNumber);

	bool IdentityExists(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type);
	bool AddIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
	bool GetIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
	bool UpdateIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
	bool DeleteIdentity(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type);
	bool ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, uCentral::Auth::ACCESS_TYPE Type);
	bool GetIdentityRights(std::string & Identity, uCentral::Objects::AclTemplate & ACL);

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

        friend bool AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats);
        friend bool GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                               std::vector<uCentral::Objects::Statistics> &Stats);
        friend bool DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );

        friend bool AddHealthCheckData(std::string &SerialNumber, uCentral::Objects::HealthCheck & Check);
        friend bool GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                                      std::vector<uCentral::Objects::HealthCheck> &Checks);
        friend bool DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );

        friend bool CreateDefaultDevice(const std::string & SerialNumber, const std::string & Capabilities);
        friend bool DeviceExists(std::string & SerialNumber);

        friend bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t & NewUUID );
        friend bool CreateDevice(uCentral::Objects::Device &);
        friend bool GetDevice(std::string &SerialNumber, uCentral::Objects::Device &);
        friend bool GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::Device> &Devices);
		friend bool GetDevices(uint64_t From, uint64_t HowMany, const std::string & Select, std::vector<uCentral::Objects::Device> &Devices);
        friend bool DeleteDevice(std::string &SerialNumber);
        friend bool UpdateDevice(uCentral::Objects::Device &);
        friend bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);
        friend bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &State);
        friend bool GetDeviceCapabilities(std::string &SerialNumber, uCentral::Objects::Capabilities &);
        friend bool DeleteDeviceCapabilities(std::string & SerialNumber);
		friend bool SetOwner(std::string & SerialNumber, std::string & OwnerUUID);
		friend bool SetLocation(std::string & SerialNumber, std::string & LocationUUID);
		friend bool GetDeviceCount(uint64_t & Count);
		friend bool GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> & SerialNumbers);

		friend bool GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                               std::vector<uCentral::Objects::DeviceLog> &Stats, uint64_t Type);
        friend bool DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Type);
        friend bool AddLog(std::string & SerialNumber, const std::string & Log);
        friend bool AddLog(std::string & SerialNumber, uCentral::Objects::DeviceLog & Log, bool CrashLog);

        friend bool CreateDefaultConfiguration(std::string & name, uCentral::Objects::DefaultConfiguration & DefConfig);
        friend bool DeleteDefaultConfiguration(std::string & name);
        friend bool UpdateDefaultConfiguration(std::string & name, uCentral::Objects::DefaultConfiguration & DefConfig);
        friend bool GetDefaultConfiguration(std::string &name, uCentral::Objects::DefaultConfiguration & DefConfig);
        friend bool GetDefaultConfigurations(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::DefaultConfiguration> &Devices);

        friend bool AddCommand(std::string & SerialNumber, uCentral::Objects::CommandDetails & Command,bool AlreadyExecuted);
        friend bool GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands);
        friend bool DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate);
        friend bool GetNonExecutedCommands( uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands );
        friend bool UpdateCommand( std::string &UUID, uCentral::Objects::CommandDetails & Command );
        friend bool GetCommand( std::string &UUID, uCentral::Objects::CommandDetails & Command );
        friend bool DeleteCommand( std::string &UUID );
        friend bool GetReadyToExecuteCommands( uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands );
        friend bool CommandExecuted(std::string & UUID);
        friend bool CommandCompleted(std::string & UUID, Poco::DynamicStruct & ReturnVars, bool FullCommand);
        friend bool AttachFileToCommand(std::string & UUID);
		friend bool GetAttachedFile(std::string & UUID, const std::string & FileName);
		friend bool RemoveAttachedFile(std::string & UUID);

		friend bool AddBlackListDevices(std::vector<uCentral::Objects::BlackListedDevice> &  Devices);
		friend bool DeleteBlackListDevice(std::string & SerialNumber);
		friend bool GetBlackListDevices(uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::BlackListedDevice> & Devices );
		friend bool IsBlackListed(std::string & SerialNumber);
		friend bool SetCommandResult(std::string & UUID, std::string & Result);
		friend bool SetFirmware(std::string & SerialNumber, std::string & Firmware );

		friend bool IdentityExists(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type);
		friend bool AddIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		friend bool GetIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		friend bool UpdateIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		friend bool DeleteIdentity(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type);
		friend bool ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, uCentral::Auth::ACCESS_TYPE Type);
		friend bool GetIdentityRights(std::string & Identity, uCentral::Objects::AclTemplate & ACL);

	  private:
		static Service      							*instance_;
		std::unique_ptr<Poco::Data::SessionPool>        Pool_= nullptr;
		bool 											IsPSQL_ = false;
		std::unique_ptr<Poco::Data::SQLite::Connector>  SQLiteConn_= nullptr;
#ifndef SMALL_BUILD
		std::unique_ptr<Poco::Data::PostgreSQL::Connector>  PostgresConn_= nullptr;
		std::unique_ptr<Poco::Data::MySQL::Connector>       MySQLConn_= nullptr;
		std::unique_ptr<Poco::Data::ODBC::Connector>        ODBCConn_= nullptr;
#endif

        bool AddLog(std::string & SerialNumber, uCentral::Objects::DeviceLog & Log, bool CrashLog );
        bool AddLog(std::string & SerialNumber, const std::string & Log );
        bool AddStatisticsData(std::string &SerialNumber, uint64_t CfgUUID, std::string &NewStats);
        bool GetStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                               std::vector<uCentral::Objects::Statistics> &Stats);
        bool DeleteStatisticsData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );

        bool AddHealthCheckData(std::string &SerialNumber, uCentral::Objects::HealthCheck & Check);
        bool GetHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                                       std::vector<uCentral::Objects::HealthCheck> &Checks);
        bool DeleteHealthCheckData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate );

        bool UpdateDeviceConfiguration(std::string &SerialNumber, std::string &Configuration, uint64_t & NewUUID );
        
        bool CreateDevice(uCentral::Objects::Device &);
        bool CreateDefaultDevice(const std::string & SerialNumber, const std::string & Capabilities);
        
        bool GetDevice(std::string &SerialNumber, uCentral::Objects::Device &);
        bool GetDevices(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::Device> &Devices);
		bool GetDevices(uint64_t From, uint64_t HowMany, const std::string & Select, std::vector<uCentral::Objects::Device> &Devices);
        bool DeleteDevice(std::string &SerialNumber);
        bool UpdateDevice(uCentral::Objects::Device &);
        bool DeviceExists(std::string & SerialNumber);
		bool SetOwner(std::string & SerialNumber, std::string & OwnerUUID);
		bool SetLocation(std::string & SerialNumber, std::string & LocationUUID);
		bool SetFirmware(std::string & SerialNumber, std::string & Firmware );
		bool GetDeviceCount(uint64_t & Count);
		bool GetDeviceSerialNumbers(uint64_t From, uint64_t HowMany, std::vector<std::string> & SerialNumbers);

        bool ExistingConfiguration(std::string &SerialNumber, uint64_t CurrentConfig, std::string &NewConfig, uint64_t &);

        bool UpdateDeviceCapabilities(std::string &SerialNumber, std::string &State);
        bool GetDeviceCapabilities(std::string &SerialNumber, uCentral::Objects::Capabilities &);
        bool DeleteDeviceCapabilities(std::string & SerialNumber);

        bool GetLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany,
                        std::vector<uCentral::Objects::DeviceLog> &Stats, uint64_t Type);
        bool DeleteLogData(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Type);

        bool CreateDefaultConfiguration(std::string & name, uCentral::Objects::DefaultConfiguration & DefConfig);
        bool DeleteDefaultConfiguration(std::string & name);
        bool UpdateDefaultConfiguration(std::string & name, uCentral::Objects::DefaultConfiguration & DefConfig);
        bool GetDefaultConfiguration(std::string &name, uCentral::Objects::DefaultConfiguration & DefConfig);
        bool GetDefaultConfigurations(uint64_t From, uint64_t HowMany, std::vector<uCentral::Objects::DefaultConfiguration> &Devices);
        bool FindDefaultConfigurationForModel(const std::string & Model, uCentral::Objects::DefaultConfiguration & DefConfig );

        bool AddCommand(std::string & SerialNumber, uCentral::Objects::CommandDetails & Command,bool AlreadyExecuted=false);
        bool GetCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate, uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands);
        bool DeleteCommands(std::string &SerialNumber, uint64_t FromDate, uint64_t ToDate);
        bool GetNonExecutedCommands( uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands );
        bool UpdateCommand( std::string &UUID, uCentral::Objects::CommandDetails & Command );
        bool GetCommand( std::string &UUID, uCentral::Objects::CommandDetails & Command );
        bool DeleteCommand( std::string &UUID );
        bool GetReadyToExecuteCommands( uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::CommandDetails> & Commands );
        bool CommandExecuted(std::string & UUID);
        bool CommandCompleted(std::string & UUID, Poco::DynamicStruct & ReturnVars, bool FullCommand);
        bool AttachFileToCommand(std::string & UUID);
		bool GetAttachedFile(std::string & UUID, const std::string & FileName);
		bool RemoveAttachedFile(std::string & UUID);
		bool SetCommandResult(std::string & UUID, std::string & Result);

		bool AddBlackListDevices(std::vector<uCentral::Objects::BlackListedDevice> &  Devices);
		bool DeleteBlackListDevice(std::string & SerialNumber);
		bool IsBlackListed(std::string & SerialNumber);
		bool GetBlackListDevices(uint64_t Offset, uint64_t HowMany, std::vector<uCentral::Objects::BlackListedDevice> & Devices );

		//	for usernames
		bool IdentityExists(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type);
		bool AddIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		bool GetIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		bool UpdateIdentity(std::string & Identity, std::string & Password, uCentral::Auth::ACCESS_TYPE Type, uCentral::Objects::AclTemplate & ACL);
		bool DeleteIdentity(std::string & Identity, uCentral::Auth::ACCESS_TYPE Type);
		bool ListIdentities(uint64_t Offset, uint64_t HowMany, std::vector<std::string> & Identities, uCentral::Auth::ACCESS_TYPE Type);
		bool GetIdentityRights(std::string & Identity, uCentral::Objects::AclTemplate & ACL);

        int 	Start() override;
        void 	Stop() override;
        int 	Setup_SQLite();
		[[nodiscard]] std::string ConvertParams(const std::string &S) const;

#ifndef SMALL_BUILD
        int 	Setup_MySQL();
        int 	Setup_PostgreSQL();
        int 	Setup_ODBC();
#endif
   };

}  // namespace

#endif //UCENTRAL_USTORAGESERVICE_H
