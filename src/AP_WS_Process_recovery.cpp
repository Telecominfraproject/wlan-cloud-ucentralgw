//
// Created by stephane bourque on 2022-07-26.
//

#include "AP_WS_Connection.h"
#include "CommandManager.h"
#include "StorageService.h"

#include "fmt/format.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/ow_constants.h"

namespace OpenWifi {
	void AP_WS_Connection::Process_recovery(Poco::JSON::Object::Ptr ParamsObj) {
		if (ParamsObj->has(uCentralProtocol::SERIAL) &&
			ParamsObj->has(uCentralProtocol::FIRMWARE) && ParamsObj->has(uCentralProtocol::UUID) &&
			ParamsObj->has(uCentralProtocol::REBOOT) &&
			ParamsObj->has(uCentralProtocol::LOGLINES)) {

			auto LogLines = ParamsObj->get(uCentralProtocol::LOGLINES);
			std::string LogText;

			LogText = "Firmware: " + ParamsObj->get(uCentralProtocol::FIRMWARE).toString() + "\r\n";
			if (LogLines.isArray()) {
				auto LogLinesArray = LogLines.extract<Poco::JSON::Array::Ptr>();
				for (const auto &i : *LogLinesArray)
					LogText += i.toString() + "\r\n";
			}

			GWObjects::DeviceLog DeviceLog{.SerialNumber = SerialNumber_,
										   .Log = LogText,
										   .Data = "",
										   .Severity = GWObjects::DeviceLog::LOG_EMERG,
										   .Recorded = (uint64_t)time(nullptr),
										   .LogType = 1,
										   .UUID = 0};

			StorageService()->AddLog(*DbSession_, DeviceLog);

			if (ParamsObj->get(uCentralProtocol::REBOOT).toString() == "true") {
				GWObjects::CommandDetails Cmd;
				Cmd.SerialNumber = SerialNumber_;
				Cmd.UUID = MicroServiceCreateUUID();
				Cmd.SubmittedBy = uCentralProtocol::SUBMITTED_BY_SYSTEM;
				Cmd.Status = uCentralProtocol::PENDING;
				Cmd.Command = uCentralProtocol::REBOOT;
				Poco::JSON::Object Params;
				Params.set(uCentralProtocol::SERIAL, SerialNumber_);
				Params.set(uCentralProtocol::WHEN, 0);
				std::ostringstream O;
				Poco::JSON::Stringifier::stringify(Params, O);
				Cmd.Details = O.str();
				bool Sent;
				CommandManager()->PostCommand(CommandManager()->Next_RPC_ID(),
											  APCommands::Commands::reboot, SerialNumber_,
											  Cmd.Command, Params, Cmd.UUID, Sent, false, false);
				StorageService()->AddCommand(SerialNumber_, Cmd,
											 Storage::CommandExecutionType::COMMAND_EXECUTED);
				poco_information(
					Logger_,
					fmt::format("RECOVERY({}): Recovery mode received, need for a reboot.", CId_));
			} else {
				poco_information(
					Logger_,
					fmt::format("RECOVERY({}): Recovery mode received, no need for a reboot.",
								CId_));
			}
		} else {
			poco_warning(Logger_, fmt::format("RECOVERY({}): Recovery missing one of serialnumber, "
											  "firmware, uuid, loglines, reboot",
											  CId_));
		}
	}
} // namespace OpenWifi