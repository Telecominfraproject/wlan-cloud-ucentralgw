//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <algorithm>
#include <chrono>

#include "Poco/JSON/Parser.h"

#include "AP_WS_Server.h"
#include "CentralConfig.h"
#include "CommandManager.h"
#include "FileUploader.h"
#include "RESTAPI_RPC.h"
#include "RESTAPI_device_commandHandler.h"
#include "RESTObjects/RESTAPI_GWobjects.h"
#include "StorageService.h"
#include "TelemetryStream.h"

#include "SignatureMgr.h"

#include "framework/ConfigurationValidator.h"
#include "framework/KafkaManager.h"
#include "framework/KafkaTopics.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/ow_constants.h"
#include "framework/utils.h"

#include "rttys/RTTYS_server.h"

namespace OpenWifi {

	void RESTAPI_device_commandHandler::CallCanceled(const char *Cmd,
													 const OpenWifi::RESTAPI::Errors::msg &Err,
													 const std::string &Details) {
		poco_warning(Logger_,
					 fmt::format("{},{}: TID={} Canceled. Error:{} Reason:{} Details={}", Cmd,
								 SerialNumber_, TransactionId_, Err.err_num, Err.err_txt, Details));
	}

	void RESTAPI_device_commandHandler::DoGet() {
		if (!ValidateParameters()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (!Utils::NormalizeMac(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		GWObjects::Device TheDevice;
		if (!StorageService()->GetDevice(SerialNumber_, TheDevice)) {
			return NotFound();
		}

		auto Command = APCommands::to_apcommand(Command_.c_str());
		if (Command == APCommands::Commands::unknown) {
			return BadRequest(RESTAPI::Errors::InvalidCommand);
		}

		SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);
		Poco::Thread::current()->setName(
			fmt::format("{}:{}:{}", Command_, TransactionId_, SerialNumber_));

		switch (Command) {
		case APCommands::Commands::capabilities:
			return GetCapabilities();
		case APCommands::Commands::logs:
			return GetLogs();
		case APCommands::Commands::healthchecks:
			return GetChecks();
		case APCommands::Commands::statistics:
			return GetStatistics();
		case APCommands::Commands::status:
			return GetStatus();
		case APCommands::Commands::rtty: {
			GWObjects::DeviceRestrictions Restrictions;
			if (!AP_WS_Server()->Connected(SerialNumberInt_, Restrictions)) {
				CallCanceled(Command_.c_str(), RESTAPI::Errors::DeviceNotConnected);
				return BadRequest(RESTAPI::Errors::DeviceNotConnected);
			}
			auto UUID = MicroServiceCreateUUID();
			auto RPC = CommandManager()->Next_RPC_ID();
			poco_debug(
				Logger_,
				fmt::format(
					"Command rtty TID={} can proceed. Identified as {} and RPCID as {}. thr_id={}",
					TransactionId_, UUID, RPC, Poco::Thread::current()->id()));
			return Rtty(UUID, RPC, 60000ms, Restrictions);
		};
		default:
			return BadRequest(RESTAPI::Errors::InvalidCommand);
		}
	}

	void RESTAPI_device_commandHandler::DoDelete() {
		if (!ValidateParameters()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (!Utils::NormalizeMac(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		GWObjects::Device TheDevice;
		if (!StorageService()->GetDevice(SerialNumber_, TheDevice)) {
			return NotFound();
		}

		auto Command = APCommands::to_apcommand(Command_.c_str());
		if (Command == APCommands::Commands::unknown) {
			return BadRequest(RESTAPI::Errors::InvalidCommand);
		}

		SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);
		Poco::Thread::current()->setName(
			fmt::format("{}:{}:{}", Command_, TransactionId_, SerialNumber_));

		switch (Command) {
		case APCommands::Commands::capabilities:
			return DeleteCapabilities();
		case APCommands::Commands::logs:
			return DeleteLogs();
		case APCommands::Commands::healthchecks:
			return DeleteChecks();
		case APCommands::Commands::statistics:
			return DeleteStatistics();
		default:
			return BadRequest(RESTAPI::Errors::InvalidCommand);
		}
	}

	struct PostDeviceCommand {
		APCommands::Commands Command = APCommands::Commands::unknown;
		bool AllowParallel = false;
		bool RequireConnection = true;
		void (RESTAPI_device_commandHandler::*funPtr)(
			const std::string &, std::uint64_t, std::chrono::milliseconds,
			const GWObjects::DeviceRestrictions &Restrictions);
		std::chrono::milliseconds Timeout = 120ms;
	};

	static const std::vector<PostDeviceCommand> PostCommands = {
		{APCommands::Commands::configure, false, false, &RESTAPI_device_commandHandler::Configure,
		 120000ms},
		{APCommands::Commands::upgrade, false, false, &RESTAPI_device_commandHandler::Upgrade,
		 30000ms},
		{APCommands::Commands::reboot, false, true, &RESTAPI_device_commandHandler::Reboot,
		 30000ms},
		{APCommands::Commands::factory, false, false, &RESTAPI_device_commandHandler::Factory,
		 30000ms},
		{APCommands::Commands::leds, false, true, &RESTAPI_device_commandHandler::LEDs, 120000ms},
		{APCommands::Commands::trace, false, true, &RESTAPI_device_commandHandler::Trace, 300000ms},
		{APCommands::Commands::request, false, true, &RESTAPI_device_commandHandler::MakeRequest,
		 120000ms},
		{APCommands::Commands::wifiscan, false, true, &RESTAPI_device_commandHandler::WifiScan,
		 120000ms},
		{APCommands::Commands::eventqueue, false, true, &RESTAPI_device_commandHandler::EventQueue,
		 30000ms},
		{APCommands::Commands::telemetry, false, true, &RESTAPI_device_commandHandler::Telemetry,
		 30000ms},
		{APCommands::Commands::ping, false, true, &RESTAPI_device_commandHandler::Ping, 60000ms},
		{APCommands::Commands::rrm, false, true, &RESTAPI_device_commandHandler::RRM, 60000ms},
		{APCommands::Commands::certupdate, false, true, &RESTAPI_device_commandHandler::CertUpdate, 60000ms},
		{APCommands::Commands::transfer, false, true, &RESTAPI_device_commandHandler::Transfer, 60000ms}
	};

	void RESTAPI_device_commandHandler::DoPost() {
		if (!ValidateParameters()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}
		if (!Utils::NormalizeMac(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		auto Command = APCommands::to_apcommand(Command_.c_str());
		if (Command == APCommands::Commands::unknown) {
			return BadRequest(RESTAPI::Errors::InvalidCommand);
		}

		SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);
		GWObjects::Device TheDevice;
		if (!StorageService()->GetDevice(SerialNumber_, TheDevice)) {
			return NotFound();
		}

		for (const auto &PostCommand : PostCommands) {
			if (Command == PostCommand.Command) {
				Poco::Thread::current()->setName(
					fmt::format("{}:{}:{}", Command_, TransactionId_, SerialNumber_));
				GWObjects::DeviceRestrictions Restrictions;
				if (PostCommand.RequireConnection &&
					!AP_WS_Server()->Connected(SerialNumberInt_, Restrictions)) {
					CallCanceled(Command_.c_str(), RESTAPI::Errors::DeviceNotConnected);
					return BadRequest(RESTAPI::Errors::DeviceNotConnected);
				}
				std::string Command_UUID;
				APCommands::Commands CommandName;
				if (!PostCommand.AllowParallel &&
					CommandManager()->CommandRunningForDevice(SerialNumberInt_, Command_UUID,
															  CommandName)) {
					auto Extra = fmt::format("UUID={} Command={}", Command_UUID,
											 APCommands::to_string(CommandName));
					CallCanceled(Command_.c_str(), RESTAPI::Errors::DeviceIsAlreadyBusy, Extra);
					return BadRequest(RESTAPI::Errors::DeviceIsAlreadyBusy, Extra);
				}
				auto UUID = MicroServiceCreateUUID();
				auto RPC = CommandManager()->Next_RPC_ID();
				poco_debug(Logger_, fmt::format("Command {} TID={} can proceed. Identified as {} "
												"and RPCID as {}. thr_id={}",
												Command_, TransactionId_, UUID, RPC,
												Poco::Thread::current()->id()));
				return (*this.*PostCommand.funPtr)(UUID, RPC, PostCommand.Timeout, Restrictions);
			}
		}
		return BadRequest(RESTAPI::Errors::InvalidCommand);
	}

	void RESTAPI_device_commandHandler::GetCapabilities() {
		poco_debug(Logger_, fmt::format("GET-CAPABILITIES: TID={} user={} serial={}. thr_id={}",
										TransactionId_, Requester(), SerialNumber_,
										Poco::Thread::current()->id()));
		GWObjects::Capabilities Caps;
		if (StorageService()->GetDeviceCapabilities(SerialNumber_, Caps)) {
			Poco::JSON::Object RetObj;
			Caps.to_json(RetObj);
			RetObj.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
			return ReturnObject(RetObj);
		}
		NotFound();
	}

	void RESTAPI_device_commandHandler::DeleteCapabilities() {
		poco_debug(Logger_, fmt::format("DELETE-CAPABILITIES: TID={} user={} serial={}. thr_id={}",
										TransactionId_, Requester(), SerialNumber_,
										Poco::Thread::current()->id()));
		if (StorageService()->DeleteDeviceCapabilities(SerialNumber_)) {
			return OK();
		}
		NotFound();
	}

	void RESTAPI_device_commandHandler::GetStatistics() {
		std::string StatsType =
			QB_.LastOnly ? "LastOnly"
						 : (QB_.Newest ? "Newest" : (QB_.CountOnly ? "CountOnly" : "Timed"));
		poco_debug(Logger_,
				   fmt::format("GET-STATISTICS: TID={} user={} serial={}. thr_id={}, TYPE={}",
							   TransactionId_, Requester(), SerialNumber_,
							   Poco::Thread::current()->id(), StatsType));
		if (QB_.LastOnly) {
			std::string Stats;
			if (AP_WS_Server()->GetStatistics(SerialNumber_, Stats) && !Stats.empty()) {
				return ReturnRawJSON(Stats);
			}
			if (AP_WS_Server()->Connected(SerialNumberInt_)) {
				return BadRequest(RESTAPI::Errors::NoDeviceStatisticsYet);
			}
			return BadRequest(RESTAPI::Errors::DeviceNotConnected);
		}

		std::vector<GWObjects::Statistics> Stats;
		if (QB_.Newest) {
			StorageService()->GetNewestStatisticsData(SerialNumber_, QB_.Limit, Stats);
		} else {
			if (QB_.CountOnly) {
				std::uint64_t Count = 0;
				StorageService()->GetNumberOfStatisticsDataRecords(SerialNumber_, QB_.StartDate,
																   QB_.EndDate, Count);
				return ReturnCountOnly(Count);
			}
			if (QB_.Limit > 100)
				QB_.Limit = 100;

			StorageService()->GetStatisticsData(SerialNumber_, QB_.StartDate, QB_.EndDate,
												QB_.Offset, QB_.Limit, Stats);
		}

		Poco::JSON::Array::Ptr ArrayObj = Poco::SharedPtr<Poco::JSON::Array>(new Poco::JSON::Array);
		for (const auto &i : Stats) {
			Poco::JSON::Object::Ptr Obj =
				Poco::SharedPtr<Poco::JSON::Object>(new Poco::JSON::Object);
			i.to_json(*Obj);
			ArrayObj->add(Obj);
		}

		Poco::JSON::Object RetObj;
		RetObj.set(RESTAPI::Protocol::DATA, ArrayObj);
		RetObj.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
		return ReturnObject(RetObj);
	}

	void RESTAPI_device_commandHandler::DeleteStatistics() {
		poco_debug(Logger_, fmt::format("DELETE-STATISTICS: TID={} user={} serial={}. thr_id={}",
										TransactionId_, Requester(), SerialNumber_,
										Poco::Thread::current()->id()));
		if (StorageService()->DeleteStatisticsData(SerialNumber_, QB_.StartDate, QB_.EndDate)) {
			return OK();
		}
		NotFound();
	}

	void RESTAPI_device_commandHandler::GetStatus() {
		poco_debug(Logger_,
				   fmt::format("GET-STATUS: TID={} user={} serial={}. thr_id={}", TransactionId_,
							   Requester(), SerialNumber_, Poco::Thread::current()->id()));
		GWObjects::ConnectionState State;

		if (AP_WS_Server()->GetState(SerialNumber_, State)) {
			Poco::JSON::Object RetObject;
			State.to_json(SerialNumber_, RetObject);
			return ReturnObject(RetObject);
		} else {
			Poco::JSON::Object RetObject;
			RetObject.set("serialNumber", SerialNumber_);
			RetObject.set("connected", false);
			return ReturnObject(RetObject);
		}
	}

	void RESTAPI_device_commandHandler::GetLogs() {
		poco_debug(Logger_,
				   fmt::format("GET-LOGS: TID={} user={} serial={}. thr_id={}", TransactionId_,
							   Requester(), SerialNumber_, Poco::Thread::current()->id()));
		std::vector<GWObjects::DeviceLog> Logs;
		if (QB_.Newest) {
			StorageService()->GetNewestLogData(SerialNumber_, QB_.Limit, Logs, QB_.LogType);
		} else {
			StorageService()->GetLogData(SerialNumber_, QB_.StartDate, QB_.EndDate, QB_.Offset,
										 QB_.Limit, Logs, QB_.LogType);
		}

		Poco::JSON::Array ArrayObj;
		for (const auto &i : Logs) {
			Poco::JSON::Object Obj;
			i.to_json(Obj);
			ArrayObj.add(Obj);
		}
		Poco::JSON::Object RetObj;
		RetObj.set(RESTAPI::Protocol::VALUES, ArrayObj);
		RetObj.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
		ReturnObject(RetObj);
	}

	void RESTAPI_device_commandHandler::DeleteLogs() {
		poco_debug(Logger_,
				   fmt::format("DELETE-LOGS: TID={} user={} serial={}. thr_id={}", TransactionId_,
							   Requester(), SerialNumber_, Poco::Thread::current()->id()));
		if (StorageService()->DeleteLogData(SerialNumber_, QB_.StartDate, QB_.EndDate,
											QB_.LogType)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::NoRecordsDeleted);
	}

	void RESTAPI_device_commandHandler::GetChecks() {
		poco_debug(Logger_, fmt::format("GET-HEALTHCHECKS: TID={} user={} serial={}. thr_id={}",
										TransactionId_, Requester(), SerialNumber_,
										Poco::Thread::current()->id()));

		if (QB_.LastOnly) {
			GWObjects::HealthCheck HC;
			if (AP_WS_Server()->GetHealthcheck(SerialNumber_, HC)) {
				Poco::JSON::Object Answer;
				HC.to_json(Answer);
				return ReturnObject(Answer);
			} else {
				return NotFound();
			}
		} else {
			std::vector<GWObjects::HealthCheck> Checks;
			if (QB_.Newest) {
				StorageService()->GetNewestHealthCheckData(SerialNumber_, QB_.Limit, Checks);
			} else {
				StorageService()->GetHealthCheckData(SerialNumber_, QB_.StartDate, QB_.EndDate,
													 QB_.Offset, QB_.Limit, Checks);
			}

			Poco::JSON::Array ArrayObj;
			for (auto i : Checks) {
				Poco::JSON::Object Obj;
				i.to_json(Obj);
				ArrayObj.add(Obj);
			}

			Poco::JSON::Object RetObj;
			RetObj.set(RESTAPI::Protocol::VALUES, ArrayObj);
			RetObj.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
			ReturnObject(RetObj);
		}
	}

	void RESTAPI_device_commandHandler::DeleteChecks() {
		poco_debug(Logger_, fmt::format("DELETE-HEALTHCHECKS: TID={} user={} serial={}. thr_id={}",
										TransactionId_, Requester(), SerialNumber_,
										Poco::Thread::current()->id()));
		if (StorageService()->DeleteHealthCheckData(SerialNumber_, QB_.StartDate, QB_.EndDate)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::NoRecordsDeleted);
	}

	void RESTAPI_device_commandHandler::Ping(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("PING({},{}): TID={} user={} serial={}", CMD_UUID, CMD_RPC,
										TransactionId_, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {
			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("PING", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			GWObjects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = Requester();
			Cmd.Command = uCentralProtocol::PING;
			Cmd.RunAt = 0;

			Poco::JSON::Object Params;
			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			std::stringstream ParamStream;

			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::ping, false, Cmd, Params,
										*Request, *Response, timeout, nullptr, nullptr, Logger_);

			GWObjects::CommandDetails Cmd2;
			if (StorageService()->GetCommand(CMD_UUID, Cmd2)) {
				Poco::JSON::Object Answer;
				// Answer.set("latency", Cmd2.executionTime);
				Answer.set("latency", fmt::format("{:.3f}ms.", Cmd.executionTime));
				Answer.set("serialNumber", SerialNumber_);
				Answer.set("currentUTCTime",
						   std::chrono::duration_cast<std::chrono::milliseconds>(
							   std::chrono::system_clock::now().time_since_epoch())
							   .count());
				try {
					Poco::JSON::Parser P;
					auto ResponseObj = P.parse(Cmd2.Results).extract<Poco::JSON::Object::Ptr>();
					if (ResponseObj->has("results")) {
						auto Results =
							ResponseObj->get("results").extract<Poco::JSON::Object::Ptr>();
						if (Results->has("deviceUTCTime"))
							Answer.set("deviceUTCTime", Results->has("deviceUTCTime"));
					}
				} catch (...) {
				}
				return ReturnObject(Answer);
			}
			return NotFound();
		}
		return BadRequest(RESTAPI::Errors::MissingSerialNumber);
	}

	bool RESTAPI_device_commandHandler::IsDeviceSimulated(std::string &Serial) {
		GWObjects::Device	Device;
		if(StorageService()->GetDevice(Serial,Device)) {
			return Device.simulated;
		}
		return false;
	}

	void RESTAPI_device_commandHandler::CallCanceled(const char *Cmd, const std::string &UUID,
													 uint64_t RPC,
													 const OpenWifi::RESTAPI::Errors::msg &Err) {
		poco_warning(Logger_, fmt::format("{}({},{}): Canceled. Error:{} Reason:{}", Cmd, UUID, RPC,
										  Err.err_num, Err.err_txt));
	}

	static bool ValidateScriptType(const std::string &t) { return t == "shell" || t == "bundle"; }

	void RESTAPI_device_commandHandler::Script(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("SCRIPT({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		if(IsDeviceSimulated(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		const auto &Obj = ParsedBody_;
		GWObjects::ScriptRequest SCR;
		if (!SCR.from_json(Obj)) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC, RESTAPI::Errors::InvalidJSONDocument);
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		bool DiagnosticScript = (SCR.type == "diagnostic");
		if (!SCR.script.empty() && !SCR.scriptId.empty()) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC, RESTAPI::Errors::InvalidScriptSelection);
			return UnAuthorized(RESTAPI::Errors::InvalidScriptSelection);
		}

		if (!Internal_ && UserInfo_.userinfo.userRole != SecurityObjects::ROOT &&
			SCR.scriptId.empty()) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC, RESTAPI::Errors::ACCESS_DENIED);
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		if (SCR.script.empty() && SCR.scriptId.empty() && !DiagnosticScript) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC, RESTAPI::Errors::InvalidScriptSelection);
			return BadRequest(RESTAPI::Errors::InvalidScriptSelection);
		}

		if (DiagnosticScript && (!SCR.scriptId.empty() || !SCR.script.empty())) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC, RESTAPI::Errors::InvalidScriptSelection);
			return BadRequest(RESTAPI::Errors::InvalidScriptSelection);
		}

		if (SerialNumber_ != SCR.serialNumber) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
			return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
		}

		if (!SCR.uri.empty() && !Utils::ValidateURI(SCR.uri)) {
			return BadRequest(RESTAPI::Errors::InvalidURI);
		}

		GWObjects::Device D;
		if (!StorageService()->GetDevice(SerialNumber_, D)) {
			return NotFound();
		}

		std::string EncodedScript;
		if (!SCR.scriptId.empty()) {
			GWObjects::ScriptEntry Existing;
			if(Utils::ValidUUID(SCR.scriptId)) {
				if (!StorageService()->ScriptDB().GetRecord("id", SCR.scriptId, Existing)) {
					CallCanceled("SCRIPT", CMD_UUID, CMD_RPC,
								 RESTAPI::Errors::MissingOrInvalidParameters);
					return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
				}
			} else if(!StorageService()->ScriptDB().GetRecord("name", SCR.scriptId, Existing)) {
				CallCanceled("SCRIPT", CMD_UUID, CMD_RPC,
							 RESTAPI::Errors::MissingOrInvalidParameters);
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}

			//	verify the role...
			if (Existing.restricted.empty() &&
				UserInfo_.userinfo.userRole != SecurityObjects::ROOT) {
				CallCanceled("SCRIPT", CMD_UUID, CMD_RPC, RESTAPI::Errors::ACCESS_DENIED);
				return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
			}

			if (UserInfo_.userinfo.userRole != SecurityObjects::ROOT) {
				if (std::find(Existing.restricted.begin(), Existing.restricted.end(),
							  SecurityObjects::UserTypeToString(UserInfo_.userinfo.userRole)) ==
					end(Existing.restricted)) {
					CallCanceled("SCRIPT", CMD_UUID, CMD_RPC, RESTAPI::Errors::ACCESS_DENIED);
					return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
				}
			}
			poco_debug(Logger_, fmt::format("SCRIPT({},{}): TID={} Name={}", CMD_UUID, CMD_RPC,
											TransactionId_, Existing.name));
			SCR.script = Existing.content;
			SCR.type = Existing.type;
			if (!ParsedBody_->has("deferred"))
				SCR.deferred = Existing.deferred;
			if (!ParsedBody_->has("timeout"))
				SCR.timeout = Existing.timeout;
			EncodedScript =
				Utils::base64encode((const unsigned char *)SCR.script.c_str(), SCR.script.size());
		} else {
			if (!DiagnosticScript && !ValidateScriptType(SCR.type)) {
				CallCanceled("SCRIPT", CMD_UUID, CMD_RPC,
							 RESTAPI::Errors::MissingOrInvalidParameters);
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}
			if(!DiagnosticScript) {
				EncodedScript = SCR.script;
			}
		}

		uint64_t ap_timeout = SCR.timeout == 0 ? 30 : SCR.timeout;

		GWObjects::CommandDetails Cmd;
		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = CMD_UUID;
		Cmd.SubmittedBy = Requester();
		Cmd.Command = uCentralProtocol::SCRIPT;
		Cmd.RunAt = 0;
		Cmd.WaitingForFile = SCR.deferred ? 1 : 0;

		Poco::JSON::Object Params;
		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		if (SCR.deferred && SCR.uri.empty()) {
			SCR.uri = FileUploader()->FullName() + CMD_UUID;
		}

		if (SCR.deferred) {
			Cmd.deferred = true;
			Params.set(uCentralProtocol::URI, SCR.uri);
		} else {
			Params.set(uCentralProtocol::TIMEOUT, ap_timeout);
			Cmd.deferred = false;
		}

		if (!SCR.signature.empty()) {
			Params.set(uCentralProtocol::SIGNATURE, SCR.signature);
		}

		if (!Restrictions.developer && D.restrictedDevice && SCR.signature.empty()) {
			SCR.signature = SignatureManager()->Sign(Restrictions, SCR.script);
		}

		if (!Restrictions.developer && D.restrictedDevice && SCR.signature.empty()) {
			return BadRequest(RESTAPI::Errors::DeviceRequiresSignature);
		}

		// convert script to base64 ...
		Params.set(uCentralProtocol::TYPE, SCR.type);
		if (!DiagnosticScript) {
			Params.set(uCentralProtocol::SCRIPT, EncodedScript);
		}
		Params.set(uCentralProtocol::WHEN, SCR.when);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();
		FileUploader()->AddUUID(CMD_UUID, 15min, SCR.type == "shell" ? "txt" : "tgz" );

		return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::script, false, Cmd,
										   Params, *Request, *Response, timeout, nullptr, this,
										   Logger_, Cmd.deferred);
	}

	void RESTAPI_device_commandHandler::Configure(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("CONFIGURE({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) && Obj->has(RESTAPI::Protocol::UUID) &&
			Obj->has(RESTAPI::Protocol::CONFIGURATION)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("CONFIGURE", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Configuration =
				GetS(RESTAPI::Protocol::CONFIGURATION, Obj, uCentralProtocol::EMPTY_JSON_DOC);
			std::vector<std::string> Error;
			if (!ValidateUCentralConfiguration(Configuration, Error,
											   GetBoolParameter("strict", false))) {
				CallCanceled("CONFIGURE", CMD_UUID, CMD_RPC, RESTAPI::Errors::ConfigBlockInvalid);
				return BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
			}

			auto When = GetWhen(Obj);
			uint64_t NewUUID=0;
			if (StorageService()->SetPendingDeviceConfiguration(SerialNumber_, Configuration,
															NewUUID)) {
				GWObjects::CommandDetails Cmd;

				Cmd.SerialNumber = SerialNumber_;
				Cmd.UUID = CMD_UUID;
				Cmd.SubmittedBy = Requester();
				Cmd.Command = uCentralProtocol::CONFIGURE;
				Cmd.RunAt = When;

				Config::Config Cfg(Configuration);

				Poco::JSON::Object Params;
				Params.set(uCentralProtocol::SERIAL, SerialNumber_);
				Params.set(uCentralProtocol::UUID, NewUUID);
				Params.set(uCentralProtocol::WHEN, When);
				Params.set(uCentralProtocol::CONFIG, Cfg.to_json());
				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				// AP_WS_Server()->SetPendingUUID(SerialNumber_, NewUUID);
				RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::configure, true,
												   Cmd, Params, *Request, *Response, timeout,
												   nullptr, this, Logger_);

				if(!Cmd.Executed) {
					return;
				}

				if(Cmd.ErrorCode==2) {
					StorageService()->RollbackDeviceConfigurationChange(SerialNumber_);
				} else {
					StorageService()->CompleteDeviceConfigurationChange(SerialNumber_);
				}

				return;
			}
			return BadRequest(RESTAPI::Errors::RecordNotUpdated);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::Upgrade(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("UPGRADE({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;

		if (Obj->has(RESTAPI::Protocol::URI) && Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("UPGRADE", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			GWObjects::Device DeviceInfo;
			if (!StorageService()->GetDevice(SerialNumber_, DeviceInfo)) {
				return NotFound();
			}

			std::string FWSignature = GetParameter("FWsignature", "");
			auto URI = GetS(RESTAPI::Protocol::URI, Obj);
			auto When = GetWhen(Obj);

			auto KeepRedirector = GetB(RESTAPI::Protocol::KEEPREDIRECTOR, Obj, true);

			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = Requester();
			Cmd.Command = uCentralProtocol::UPGRADE;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::URI, URI);
			Params.set(uCentralProtocol::KEEP_REDIRECTOR, KeepRedirector ? 1 : 0);

			if (DeviceInfo.restrictionDetails.upgrade && FWSignature.empty()) {
				Poco::URI uri(URI);
				FWSignature = SignatureManager()->Sign(DeviceInfo.restrictionDetails, uri);
			}

			if (!Restrictions.developer && FWSignature.empty() && DeviceInfo.restrictionDetails.upgrade) {
				return BadRequest(RESTAPI::Errors::DeviceRequiresSignature);
			}

			if (!FWSignature.empty()) {
				Params.set(uCentralProtocol::SIGNATURE, FWSignature);
			}

			Params.set(uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::upgrade, true, Cmd,
											   Params, *Request, *Response, timeout, nullptr, this,
											   Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::Reboot(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("REBOOT({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {
			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("REBOOT", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			uint64_t When = GetWhen(Obj);
			GWObjects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = Requester();
			Cmd.Command = uCentralProtocol::REBOOT;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::reboot, false, Cmd,
											   Params, *Request, *Response, timeout, nullptr, this,
											   Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingSerialNumber);
	}

	void RESTAPI_device_commandHandler::Factory(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("FACTORY-RESET({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::KEEPREDIRECTOR) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();

			if (SerialNumber_ != SNum) {
				CallCanceled("FACTORY-RESET", CMD_UUID, CMD_RPC,
							 RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto KeepRedirector = GetB(RESTAPI::Protocol::KEEPREDIRECTOR, Obj, true);
			uint64_t When = GetWhen(Obj);

			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = Requester();
			Cmd.Command = uCentralProtocol::FACTORY;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::KEEP_REDIRECTOR, KeepRedirector ? 1 : 0);
			Params.set(uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::factory, true, Cmd,
											   Params, *Request, *Response, timeout, nullptr, this,
											   Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::LEDs(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("LEDS({},{}): TID={} user={} serial={}", CMD_UUID, CMD_RPC,
										TransactionId_, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;

		if (Obj->has(uCentralProtocol::PATTERN) && Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("LEDS", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Pattern = GetS(uCentralProtocol::PATTERN, Obj, uCentralProtocol::BLINK);
			if (Pattern != uCentralProtocol::ON && Pattern != uCentralProtocol::OFF &&
				Pattern != uCentralProtocol::BLINK) {
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}

			auto Duration = Get(uCentralProtocol::DURATION, Obj, 30);
			auto When = GetWhen(Obj);

			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = Requester();
			Cmd.Command = uCentralProtocol::LEDS;
			Cmd.RunAt = When;
			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::DURATION, Duration);
			Params.set(uCentralProtocol::WHEN, When);
			Params.set(uCentralProtocol::PATTERN, Pattern);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::leds, false, Cmd,
											   Params, *Request, *Response, timeout, nullptr, this,
											   Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::Trace(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("TRACE({},{}): TID={} user={} serial={}", CMD_UUID, CMD_RPC,
										TransactionId_, Requester(), SerialNumber_));

		if(IsDeviceSimulated(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		const auto &Obj = ParsedBody_;

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			(Obj->has(RESTAPI::Protocol::NETWORK) || Obj->has(RESTAPI::Protocol::INTERFACE))) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("TRACE", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Network = GetS(RESTAPI::Protocol::NETWORK, Obj);
			auto Interface = GetS(RESTAPI::Protocol::INTERFACE, Obj);
			auto URI = FileUploader()->FullName() + CMD_UUID;

			GWObjects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = Requester();
			Cmd.Command = uCentralProtocol::TRACE;
			Cmd.RunAt = 0;
			Cmd.WaitingForFile = 1;
			Cmd.AttachType = RESTAPI::Protocol::PCAP_FILE_TYPE;

			Poco::JSON::Object Params;

			if (Obj->has(RESTAPI::Protocol::DURATION))
				Params.set(uCentralProtocol::DURATION, Get(RESTAPI::Protocol::DURATION, Obj, 30));
			else if (Obj->has(RESTAPI::Protocol::NUMBEROFPACKETS))
				Params.set(uCentralProtocol::PACKETS,
						   Get(RESTAPI::Protocol::NUMBEROFPACKETS, Obj, 100));
			else
				Params.set(uCentralProtocol::DURATION, 30);

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::WHEN, 0);
			Params.set(uCentralProtocol::NETWORK, Network);
			Params.set(uCentralProtocol::INTERFACE, Interface);
			Params.set(uCentralProtocol::URI, URI);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			FileUploader()->AddUUID(CMD_UUID, 10min, "pcap");
			return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::trace, false, Cmd,
											   Params, *Request, *Response, timeout, nullptr, this,
											   Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::WifiScan(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("WIFISCAN({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));


		if(IsDeviceSimulated(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		const auto &Obj = ParsedBody_;

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			CallCanceled("WIFISCAN", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
			return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
		}

		bool OverrideDFS = GetB(RESTAPI::Protocol::OVERRIDEDFS, Obj, true);
		bool ActiveScan = GetB(RESTAPI::Protocol::ACTIVESCAN, Obj, false);
		uint64_t Bandwidth = Get(RESTAPI::Protocol::BANDWIDTH, Obj, (uint64_t)0);

		Poco::JSON::Array::Ptr ies;
		if (Obj->has("ies") && Obj->isArray("ies")) {
			ies = Obj->getArray("ies");
		}

		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = CMD_UUID;
		Cmd.SubmittedBy = Requester();
		Cmd.Command = uCentralProtocol::WIFISCAN;

		Poco::JSON::Object Params;

		Params.set(uCentralProtocol::SERIAL, SerialNumber_);

		if (Restrictions.developer == 0 && Restrictions.dfs && OverrideDFS) {
			return BadRequest(RESTAPI::Errors::DeviceIsRestricted);
		}

		Params.set(uCentralProtocol::OVERRIDEDFS, OverrideDFS);
		Params.set(uCentralProtocol::ACTIVE, ActiveScan);
		if (ies)
			Params.set(uCentralProtocol::IES, ies);
		if (Bandwidth != 0)
			Params.set(uCentralProtocol::BANDWIDTH, Bandwidth);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();
		RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::wifiscan, false, Cmd, Params,
									*Request, *Response, timeout, nullptr, this, Logger_);
		if (Cmd.ErrorCode == 0) {
			KafkaManager()->PostMessage(KafkaTopics::WIFISCAN, SerialNumber_, Cmd.Results);
		}
	}

	void RESTAPI_device_commandHandler::EventQueue(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("EVENT-QUEUE({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		if(IsDeviceSimulated(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) && Obj->isArray(RESTAPI::Protocol::TYPES)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("EVENT-QUEUE", CMD_UUID, CMD_RPC,
							 RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Types = Obj->getArray(RESTAPI::Protocol::TYPES);
			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = Requester();
			Cmd.Command = uCentralProtocol::EVENT;

			Poco::JSON::Object Params;
			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::TYPES, Types);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::eventqueue, false, Cmd,
										Params, *Request, *Response, timeout, nullptr, this,
										Logger_);
			if (Cmd.ErrorCode == 0) {
				KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, SerialNumber_,
											Cmd.Results);
			}
			return;
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::MakeRequest(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("FORCE-REQUEST({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		if(IsDeviceSimulated(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) && Obj->has(uCentralProtocol::MESSAGE)) {

			auto SNum = GetS(RESTAPI::Protocol::SERIALNUMBER, Obj);
			auto MessageType = GetS(uCentralProtocol::MESSAGE, Obj);

			if ((SerialNumber_ != SNum) || (MessageType != uCentralProtocol::STATE &&
											MessageType != uCentralProtocol::HEALTHCHECK)) {
				CallCanceled("FORCE-REQUEST", CMD_UUID, CMD_RPC,
							 RESTAPI::Errors::MissingOrInvalidParameters);
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}

			auto When = GetWhen(Obj);
			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.SubmittedBy = Requester();
			Cmd.UUID = CMD_UUID;
			Cmd.Command = uCentralProtocol::REQUEST;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::WHEN, When);
			Params.set(uCentralProtocol::MESSAGE, MessageType);
			Params.set(uCentralProtocol::REQUEST_UUID, CMD_UUID);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::request, false, Cmd,
											   Params, *Request, *Response, timeout, nullptr, this,
											   Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::Rtty(
		const std::string &CMD_UUID, uint64_t CMD_RPC, std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("RTTY({},{}): TID={} user={} serial={}", CMD_UUID, CMD_RPC,
										TransactionId_, Requester(), SerialNumber_));

		if(IsDeviceSimulated(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		if (!Restrictions.developer && Restrictions.rtty) {
			return BadRequest(RESTAPI::Errors::DeviceIsRestricted);
		}

		if (MicroServiceConfigGetBool("rtty.enabled", false)) {
			GWObjects::Device Device;

			if (StorageService()->GetDevice(SerialNumber_, Device)) {

				if(Device.simulated) {
					return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
				}

				static std::uint64_t rtty_sid = 0;
				rtty_sid += std::rand();
				GWObjects::RttySessionDetails Rtty{
					.SerialNumber = SerialNumber_,
					.Server = MicroServiceConfigGetString("rtty.server", "localhost"),
					.Port = MicroServiceConfigGetInt("rtty.port", 5912),
					.Token = MicroServiceConfigGetString("rtty.token", "nothing"),
					.TimeOut = MicroServiceConfigGetInt("rtty.timeout", 60),
					.ConnectionId = Utils::ComputeHash(SerialNumber_, Utils::Now(), rtty_sid)
										.substr(0, RTTY_DEVICE_TOKEN_LENGTH),
					.Started = Utils::Now(),
					.CommandUUID = CMD_UUID,
					.ViewPort = MicroServiceConfigGetInt("rtty.viewport", 5913),
					.DevicePassword = ""};

				if (RTTYS_server()->UseInternal()) {
					std::uint64_t SN = Utils::SerialNumberToInt(SerialNumber_);
					bool mTLS = AP_WS_Server()->DeviceRequiresSecureRtty(SN);
					auto Hash =  Utils::ComputeHash(UserInfo_.webtoken.refresh_token_, Utils::Now());
					Rtty.Token = Hash.substr(0, RTTY_DEVICE_TOKEN_LENGTH);
					if (!RTTYS_server()->CreateEndPoint(Rtty.ConnectionId, Rtty.Token, Requester(),
														SerialNumber_, mTLS)) {
						return BadRequest(RESTAPI::Errors::MaximumRTTYSessionsReached);
					}
				}

				Poco::JSON::Object ReturnedObject;
				Rtty.to_json(ReturnedObject);

				//	let's create the command for this request
				GWObjects::CommandDetails Cmd;
				Cmd.SerialNumber = SerialNumber_;
				Cmd.SubmittedBy = Requester();
				Cmd.UUID = CMD_UUID;
				Cmd.Command = uCentralProtocol::RTTY;

				Poco::JSON::Object Params;

				Params.set(uCentralProtocol::METHOD, uCentralProtocol::RTTY);
				Params.set(uCentralProtocol::SERIAL, SerialNumber_);
				Params.set(uCentralProtocol::ID, Rtty.ConnectionId);
				Params.set(uCentralProtocol::TOKEN, Rtty.Token);
				Params.set(uCentralProtocol::SERVER, Rtty.Server);
				Params.set(uCentralProtocol::PORT, Rtty.Port);
				Params.set(uCentralProtocol::USER, UserInfo_.webtoken.username_);
				Params.set(uCentralProtocol::TIMEOUT, Rtty.TimeOut);
				Params.set(uCentralProtocol::PASSWORD, Device.DevicePassword);

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();
				poco_debug(Logger_,
						   fmt::format("RTTY: user={} serial={} rttyid={} token={} cmd={}.",
									   Requester(), SerialNumber_, Rtty.ConnectionId, Rtty.Token,
									   CMD_UUID));
				return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::rtty, false, Cmd,
												   Params, *Request, *Response, timeout,
												   &ReturnedObject, this, Logger_);
			}
			return NotFound();
		}
		poco_debug(Logger_, fmt::format("RTTY: user={} serial={}. Internal error.", Requester(),
										SerialNumber_));
		return ReturnStatus(Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
	}

	void RESTAPI_device_commandHandler::Telemetry(
		const std::string &CMD_UUID, uint64_t CMD_RPC,
		[[maybe_unused]] std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {
		poco_debug(Logger_, fmt::format("TELEMETRY({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		if(IsDeviceSimulated(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		const auto &Obj = ParsedBody_;

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) && Obj->has(RESTAPI::Protocol::INTERVAL) &&
			Obj->has(RESTAPI::Protocol::TYPES) && Obj->isArray(RESTAPI::Protocol::TYPES)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("TELEMETRY", CMD_UUID, CMD_RPC, RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			std::stringstream oooss;
			Obj->stringify(oooss);

			std::uint64_t Lifetime = 60 * 60; // 1 hour
			std::uint64_t Interval = 5;
			bool KafkaOnly = false;

			if (Obj->has("kafka")) {
				KafkaOnly = Obj->get("kafka").toString() == "true";
			}

			auto StatusOnly = GetBoolParameter("statusOnly", false);

			AssignIfPresent(Obj, RESTAPI::Protocol::INTERVAL, Interval);
			AssignIfPresent(Obj, RESTAPI::Protocol::LIFETIME, Lifetime);

			std::vector<std::string> TelemetryTypes;
			auto Types = Obj->getArray(RESTAPI::Protocol::TYPES);
			for (const auto &type : *Types) {
				TelemetryTypes.push_back(type);
			}

			Poco::JSON::Object Answer;
			auto IntSerialNumber = Utils::SerialNumberToInt(SerialNumber_);

			if (!StatusOnly) {
				if (KafkaOnly) {
					if (Interval) {
						AP_WS_Server()->SetKafkaTelemetryReporting(
							CMD_RPC, IntSerialNumber, Interval, Lifetime, TelemetryTypes);
						Answer.set("action", "Kafka telemetry started.");
						Answer.set("uuid", CMD_UUID);
					} else {
						AP_WS_Server()->StopKafkaTelemetry(CMD_RPC, IntSerialNumber);
						Answer.set("action", "Kafka telemetry stopped.");
					}
				} else {
					if (Interval) {
						AP_WS_Server()->SetWebSocketTelemetryReporting(
							CMD_RPC, IntSerialNumber, Interval, Lifetime, TelemetryTypes);
						std::string EndPoint;
						if (TelemetryStream()->CreateEndpoint(
								Utils::SerialNumberToInt(SerialNumber_), EndPoint, CMD_UUID)) {
							Answer.set("action", "WebSocket telemetry started.");
							Answer.set("serialNumber", SerialNumber_);
							Answer.set("uuid", CMD_UUID);
							Answer.set("uri", EndPoint);
						} else {
							return BadRequest(RESTAPI::Errors::InternalError);
						}
					} else {
						Answer.set("action", "WebSocket telemetry stopped.");
						AP_WS_Server()->StopWebSocketTelemetry(CMD_RPC, IntSerialNumber);
					}
				}

				GWObjects::CommandDetails Cmd;
				Cmd.SerialNumber = SerialNumber_;
				Cmd.SubmittedBy = Requester();
				Cmd.UUID = CMD_UUID;
				Cmd.Command = uCentralProtocol::TELEMETRY;
				Poco::JSON::Object Params;
				Params.set("kafkaOnly", KafkaOnly);
				Params.set("interval", Interval);
				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();
				Cmd.RunAt = 0;
				Cmd.ErrorCode = 0;
				Cmd.WaitingForFile = 0;
				Cmd.Status = "completed";
				Cmd.Submitted = Cmd.Completed = Cmd.Executed = Utils::Now();
				Cmd.Results = R"foo( { "result" : "success" } )foo";
				Cmd.executionTime = 0.0;
				StorageService()->AddCommand(SerialNumber_, Cmd,
											 Storage::CommandExecutionType::COMMAND_COMPLETED);
			} else {
				Answer.set("action", "Telemetry status only.");
			}

			bool TelemetryRunning;
			std::uint64_t TelemetryWebSocketCount, TelemetryKafkaCount, TelemetryInterval,
				TelemetryWebSocketTimer, TelemetryKafkaTimer, TelemetryWebSocketPackets,
				TelemetryKafkaPackets;
			AP_WS_Server()->GetTelemetryParameters(
				IntSerialNumber, TelemetryRunning, TelemetryInterval, TelemetryWebSocketTimer,
				TelemetryKafkaTimer, TelemetryWebSocketCount, TelemetryKafkaCount,
				TelemetryWebSocketPackets, TelemetryKafkaPackets);
			Poco::JSON::Object TelemetryStatus;
			TelemetryStatus.set("running", TelemetryRunning);
			TelemetryStatus.set("interval", TelemetryInterval);
			TelemetryStatus.set("websocketTimer", TelemetryWebSocketTimer);
			TelemetryStatus.set("kafkaTimer", TelemetryKafkaTimer);
			TelemetryStatus.set("websocketClients", TelemetryWebSocketCount);
			TelemetryStatus.set("kafkaClients", TelemetryKafkaCount);
			TelemetryStatus.set("kafkaPackets", TelemetryKafkaPackets);
			TelemetryStatus.set("websocketPackets", TelemetryWebSocketPackets);
			Answer.set("status", TelemetryStatus);
			return ReturnObject(Answer);
		}
		return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::RRM(
		const std::string &CMD_UUID, uint64_t CMD_RPC,
		[[maybe_unused]] std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {

		poco_debug(Logger_, fmt::format("RRM({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		if(IsDeviceSimulated(SerialNumber_)) {
			CallCanceled("RRM", CMD_UUID, CMD_RPC, RESTAPI::Errors::SimulatedDeviceNotSupported);
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		if(UserInfo_.userinfo.userRole != SecurityObjects::ROOT &&
			UserInfo_.userinfo.userRole != SecurityObjects::ADMIN) {
			CallCanceled("RRM", CMD_UUID, CMD_RPC, RESTAPI::Errors::ACCESS_DENIED);
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		if(!ParsedBody_->has("actions") || !ParsedBody_->isArray("actions")) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		const auto &Actions = *ParsedBody_->getArray("actions");
		//	perform some validation on the commands.
		for(const auto &action:Actions) {
			auto ActionDetails = action.extract<Poco::JSON::Object::Ptr>();
			if(!ActionDetails->has("action")) {
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}
			auto ActionStr = ActionDetails->get("action").toString();
			if(	ActionStr != "kick"
				&& ActionStr != "channel_switch"
				&& ActionStr != "tx_power"
				&& ActionStr != "beacon_request"
				&& ActionStr != "bss_transition"
				&& ActionStr != "neighbors" ) {
				return BadRequest(RESTAPI::Errors::InvalidRRMAction);
			}
		}

		Poco::JSON::Object Params;
		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::ACTIONS, Actions);

		GWObjects::CommandDetails Cmd;
		Cmd.SerialNumber = SerialNumber_;
		Cmd.SubmittedBy = Requester();
		Cmd.UUID = CMD_UUID;
		Cmd.Command = uCentralProtocol::RRM;
		std::ostringstream os;
		Params.stringify(os);
		Cmd.Details = os.str();
		Cmd.RunAt = 0;
		Cmd.ErrorCode = 0;
		Cmd.WaitingForFile = 0;

		return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::rrm, false, Cmd,
										   Params, *Request, *Response, timeout, nullptr, this,
										   Logger_);
	}

	void RESTAPI_device_commandHandler::Transfer(
		const std::string &CMD_UUID, uint64_t CMD_RPC,
		[[maybe_unused]] std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {

		if(UserInfo_.userinfo.userRole != SecurityObjects::ROOT &&
			UserInfo_.userinfo.userRole != SecurityObjects::ADMIN) {
			CallCanceled("RRM", CMD_UUID, CMD_RPC, RESTAPI::Errors::ACCESS_DENIED);
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		poco_debug(Logger_, fmt::format("TRANSFER({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		if(IsDeviceSimulated(SerialNumber_)) {
			CallCanceled("RRM", CMD_UUID, CMD_RPC, RESTAPI::Errors::SimulatedDeviceNotSupported);
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		GWObjects::DeviceTransferRequest	TR;
		if(!TR.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::CommandDetails Cmd;
		Cmd.SerialNumber = SerialNumber_;
		Cmd.SubmittedBy = Requester();
		Cmd.UUID = CMD_UUID;
		Cmd.Command = uCentralProtocol::TRANSFER;
		std::ostringstream os;
		ParsedBody_->stringify(os);
		Cmd.Details = os.str();
		Cmd.RunAt = 0;
		Cmd.ErrorCode = 0;
		Cmd.WaitingForFile = 0;

		return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::transfer, false, Cmd,
										   *ParsedBody_, *Request, *Response, timeout, nullptr, this,
										   Logger_);
	}

	void RESTAPI_device_commandHandler::CertUpdate(
		const std::string &CMD_UUID, uint64_t CMD_RPC,
		[[maybe_unused]] std::chrono::milliseconds timeout,
		[[maybe_unused]] const GWObjects::DeviceRestrictions &Restrictions) {

		poco_debug(Logger_, fmt::format("CERTUPDATE({},{}): TID={} user={} serial={}", CMD_UUID,
										CMD_RPC, TransactionId_, Requester(), SerialNumber_));

		if(UserInfo_.userinfo.userRole != SecurityObjects::ROOT &&
			UserInfo_.userinfo.userRole != SecurityObjects::ADMIN) {
			CallCanceled("RRM", CMD_UUID, CMD_RPC, RESTAPI::Errors::ACCESS_DENIED);
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		if(IsDeviceSimulated(SerialNumber_)) {
			CallCanceled("RRM", CMD_UUID, CMD_RPC, RESTAPI::Errors::SimulatedDeviceNotSupported);
			return BadRequest(RESTAPI::Errors::SimulatedDeviceNotSupported);
		}

		GWObjects::DeviceCertificateUpdateRequest	CR;
		if(!CR.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::DeviceTransferRequest	TR;
		if(!TR.from_json(ParsedBody_)) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		GWObjects::CommandDetails Cmd;
		Cmd.SerialNumber = SerialNumber_;
		Cmd.SubmittedBy = Requester();
		Cmd.UUID = CMD_UUID;
		Cmd.Command = uCentralProtocol::CERTUPDATE;
		std::ostringstream os;
		ParsedBody_->stringify(os);
		Cmd.Details = os.str();
		Cmd.RunAt = 0;
		Cmd.ErrorCode = 0;
		Cmd.WaitingForFile = 0;

		return RESTAPI_RPC::WaitForCommand(CMD_RPC, APCommands::Commands::certupdate, false, Cmd,
										   *ParsedBody_, *Request, *Response, timeout, nullptr, this,
										   Logger_);


	}

} // namespace OpenWifi
