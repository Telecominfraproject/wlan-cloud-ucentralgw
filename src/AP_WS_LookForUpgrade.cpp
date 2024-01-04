
#include <AP_WS_Connection.h>
#include "ConfigurationCache.h"
#include "UI_GW_WebSocketNotifications.h"
#include "CommandManager.h"

namespace OpenWifi {
	bool AP_WS_Connection::LookForUpgrade(Poco::Data::Session &Session, const uint64_t UUID, uint64_t &UpgradedUUID) {

		//	A UUID of zero means ignore updates for that connection.
		if (UUID == 0)
			return false;

		uint64_t GoodConfig = ConfigurationCache().CurrentConfig(SerialNumberInt_);
		std::cout << __LINE__ << ": " << SerialNumber_ << "  INT:" << SerialNumberInt_ << "  GoodConfig: " << GoodConfig << "   UUID:" << UUID << "  Pending:" << State_.PendingUUID << std::endl;
		if (GoodConfig && (GoodConfig == UUID || GoodConfig == State_.PendingUUID)) {
			UpgradedUUID = UUID;
			return false;
		}

		GWObjects::Device D;
		if (StorageService()->GetDevice(Session,SerialNumber_, D)) {
			std::cout << "D.pendingUUID: " << D.pendingUUID << "  UUID: " << D.UUID << " SerialNumber: " << D.SerialNumber << std::endl;
			if(D.pendingUUID!=0 && UUID==D.pendingUUID) {
				std::cout << __LINE__ << ": " << SerialNumber_  << "  GoodConfig: " << GoodConfig << "   UUID:" << UUID << "  Pending:" << State_.PendingUUID << std::endl;
				//	so we sent an upgrade to a device, and now it is completing now...
				UpgradedUUID = D.pendingUUID;
				StorageService()->CompleteDeviceConfigurationChange(Session, SerialNumber_);
				return true;
			}
			std::cout << __LINE__ << ": " << SerialNumber_ << "  GoodConfig: " << GoodConfig << "   UUID:" << UUID << "  Pending:" << State_.PendingUUID << " Device:" << D.UUID << std::endl;

			//	This is the case where the cache is empty after a restart. So GoodConfig will 0. If
			// the device already 	has the right UUID, we just return.
			if (D.UUID == UUID) {
				UpgradedUUID = UUID;
				ConfigurationCache().Add(SerialNumberInt_, UUID);
				std::cout << __LINE__ << ": " << SerialNumber_ << "  Set GoodConfig" << std::endl;
				return false;
			}

			Config::Config Cfg(D.Configuration);
			//	if this is a broken device (UUID==0) just fix it
			auto StoredConfigurationUUID = Cfg.UUID();
			if(D.UUID==0 && UUID == StoredConfigurationUUID) {
				D.UUID = UpgradedUUID = UUID;
				D.pendingUUID = 0;
				D.pendingConfiguration.clear();
				D.pendingConfigurationCmd.clear();
				StorageService()->UpdateDevice(Session, D);
				ConfigurationCache().Add(SerialNumberInt_, UUID);
				std::cout << __LINE__ << ": " << SerialNumber_ << "  GoodConfig: " << GoodConfig << "   UUID:" << UUID << "  Pending:" << State_.PendingUUID << std::endl;
				return false;
			}

			if (UUID > D.UUID) {
				//	so we have a problem, the device has a newer config than we have. So we need to
				// make sure our config 	is newer.
				D.UUID = UUID + 2;
				UpgradedUUID = D.UUID;
				std::cout << __LINE__ << ": " << SerialNumber_  << "  GoodConfig: " << GoodConfig << "   UUID:" << UUID << "  Pending:" << State_.PendingUUID << std::endl;
			}

			Cfg.SetUUID(D.UUID);
			D.Configuration = Cfg.get();
			State_.PendingUUID = UpgradedUUID = D.UUID;

			GWObjects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = MicroServiceCreateUUID();
			Cmd.SubmittedBy = uCentralProtocol::SUBMITTED_BY_SYSTEM;
			Cmd.Status = uCentralProtocol::PENDING;
			Cmd.Command = uCentralProtocol::CONFIGURE;
			Poco::JSON::Parser P;
			auto ParsedConfig = P.parse(D.Configuration).extract<Poco::JSON::Object::Ptr>();
			Poco::JSON::Object Params;
			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::UUID, D.UUID);
			Params.set(uCentralProtocol::WHEN, 0);
			Params.set(uCentralProtocol::CONFIG, ParsedConfig);

			std::ostringstream O;
			Poco::JSON::Stringifier::stringify(Params, O);
			Cmd.Details = O.str();
			poco_information(Logger_,
							 fmt::format("CFG-UPGRADE({}): Current ID: {}, newer configuration {}.",
										 CId_, UUID, D.UUID));
			bool Sent;

			StorageService()->AddCommand(SerialNumber_, Cmd,
										 Storage::CommandExecutionType::COMMAND_EXECUTED);
			CommandManager()->PostCommand(
				CommandManager()->Next_RPC_ID(), APCommands::to_apcommand(Cmd.Command.c_str()),
				SerialNumber_, Cmd.Command, Params, Cmd.UUID, Sent, false, false);

			GWWebSocketNotifications::SingleDeviceConfigurationChange_t Notification;
			Notification.content.serialNumber = D.SerialNumber;
			Notification.content.oldUUID = UUID;
			Notification.content.newUUID = UpgradedUUID;
			GWWebSocketNotifications::DeviceConfigurationChange(Notification);

			std::cout << __LINE__ << ": " << SerialNumber_ << "  GoodConfig: " << GoodConfig << "   UUID:" << UUID <<
				"  Pending:" << State_.PendingUUID << "  Upgraded:" << UpgradedUUID << std::endl;

			return true;
		}
		return false;
	}

}