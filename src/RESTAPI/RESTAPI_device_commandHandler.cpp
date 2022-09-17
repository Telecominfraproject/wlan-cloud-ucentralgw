//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <chrono>
#include <algorithm>

#include "Poco/JSON/Parser.h"

#include "AP_WS_Server.h"
#include "CentralConfig.h"
#include "DeviceRegistry.h"
#include "FileUploader.h"
#include "RESTAPI_RPC.h"
#include "RESTAPI_device_commandHandler.h"
#include "RESTObjects/RESTAPI_GWobjects.h"
#include "StorageService.h"
#include "TelemetryStream.h"
#include "CommandManager.h"
#include "framework/ConfigurationValidator.h"
#include "framework/KafkaTopics.h"
#include "framework/ow_constants.h"
#include "rttys/RTTYS_server.h"

namespace OpenWifi {

	void RESTAPI_device_commandHandler::CallCanceled(const char * Cmd, const OpenWifi::RESTAPI::Errors::msg &Err) {
		Logger_.warning(fmt::format("{},{}: Canceled. Error:{} Reason:{}", Cmd, SerialNumber_, Err.err_num, Err.err_txt));
	}

	void RESTAPI_device_commandHandler::DoGet() {
		if(!ValidateParameters()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if(!Utils::ValidSerialNumber(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}

		SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);

		GWObjects::Device	TheDevice;
		if(!StorageService()->GetDevice(SerialNumber_,TheDevice)) {
			return NotFound();
		}

		Poco::Thread::current()->setName(fmt::format("{}: {}",SerialNumber_,Command_));
		if (Command_ == RESTAPI::Protocol::CAPABILITIES){
			return GetCapabilities();
		} else if (Command_ == RESTAPI::Protocol::LOGS) {
			return GetLogs();
		} else if (Command_ == RESTAPI::Protocol::HEALTHCHECKS) {
			return GetChecks();
		} else if (Command_ == RESTAPI::Protocol::STATISTICS) {
			return GetStatistics();
		} else if (Command_ == RESTAPI::Protocol::STATUS) {
			return GetStatus();
		} else if (Command_ == RESTAPI::Protocol::RTTY) {
			if(!DeviceRegistry()->Connected(SerialNumberInt_)) {
				CallCanceled(Command_.c_str(), RESTAPI::Errors::DeviceNotConnected);
				return BadRequest(RESTAPI::Errors::DeviceNotConnected);
			}			auto UUID = MicroService::CreateUUID();
			auto RPC = CommandManager()->NextRPCId();
			return Rtty(UUID,RPC);
		} else {
			return BadRequest(RESTAPI::Errors::InvalidCommand);
		}
	}


	void RESTAPI_device_commandHandler::DoDelete() {
		if(!ValidateParameters()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if(!Utils::ValidSerialNumber(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}
		SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);

		Poco::Thread::current()->setName(fmt::format("{}: {}",SerialNumber_,Command_));

		GWObjects::Device	TheDevice;
		if(!StorageService()->GetDevice(SerialNumber_,TheDevice)) {
			return NotFound();
		}

		if (Command_ == RESTAPI::Protocol::CAPABILITIES) {
			return DeleteCapabilities();
		} else if (Command_ == RESTAPI::Protocol::LOGS){
			return DeleteLogs();
		} else if (Command_ == RESTAPI::Protocol::HEALTHCHECKS){
			return DeleteChecks();
		} else if (Command_ == RESTAPI::Protocol::STATISTICS) {
			return DeleteStatistics();
		} else {
			return BadRequest(RESTAPI::Errors::InvalidCommand);
		}
	}

	struct PostDeviceCommand {
		const char *		Command;
		bool 				AllowParallel=false;
		bool 				RequireConnection = true;
		void (RESTAPI_device_commandHandler::*funPtr)(const std::string &, std::uint64_t);
	};

	static std::vector<PostDeviceCommand>	PostCommands
		{
			{ RESTAPI::Protocol::PERFORM, false, true, &RESTAPI_device_commandHandler::ExecuteCommand },
			{ RESTAPI::Protocol::CONFIGURE, false, false, &RESTAPI_device_commandHandler::Configure },
			{ RESTAPI::Protocol::UPGRADE, false, false, &RESTAPI_device_commandHandler::Upgrade },
			{ RESTAPI::Protocol::REBOOT, false, true, &RESTAPI_device_commandHandler::Reboot },
			{ RESTAPI::Protocol::FACTORY, false, false, &RESTAPI_device_commandHandler::Factory },
			{ RESTAPI::Protocol::LEDS, false, true, &RESTAPI_device_commandHandler::LEDs },
			{ RESTAPI::Protocol::TRACE, false, true, &RESTAPI_device_commandHandler::Trace },
			{ RESTAPI::Protocol::REQUEST, false, true, &RESTAPI_device_commandHandler::MakeRequest },
			{ RESTAPI::Protocol::WIFISCAN, false, true, &RESTAPI_device_commandHandler::WifiScan },
			{ RESTAPI::Protocol::EVENTQUEUE, false, true, &RESTAPI_device_commandHandler::EventQueue },
			{ RESTAPI::Protocol::TELEMETRY, false, true, &RESTAPI_device_commandHandler::Telemetry },
			{ RESTAPI::Protocol::PING, false, true, &RESTAPI_device_commandHandler::Ping },
			{ RESTAPI::Protocol::SCRIPT, false, true, &RESTAPI_device_commandHandler::Script }
		};

	void RESTAPI_device_commandHandler::DoPost() {
		if(!ValidateParameters()) {
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if(!Utils::ValidSerialNumber(SerialNumber_)) {
			return BadRequest(RESTAPI::Errors::MissingSerialNumber);
		}
		SerialNumberInt_ = Utils::SerialNumberToInt(SerialNumber_);

		Poco::Thread::current()->setName(fmt::format("{}: {}",SerialNumber_,Command_));

		GWObjects::Device	TheDevice;
		if(!StorageService()->GetDevice(SerialNumber_,TheDevice)) {
			return NotFound();
		}

		for(const auto &Command:PostCommands) {
			if(Command_==Command.Command) {
				if(Command.RequireConnection && !DeviceRegistry()->Connected(SerialNumberInt_)) {
					CallCanceled(Command.Command, RESTAPI::Errors::DeviceNotConnected);
					return BadRequest(RESTAPI::Errors::DeviceNotConnected);
				}
				if(!Command.AllowParallel && CommandManager()->CommandRunningForDevice(SerialNumberInt_)) {
					CallCanceled(Command.Command, RESTAPI::Errors::DeviceIsAlreadyBusy);
					return BadRequest(RESTAPI::Errors::DeviceIsAlreadyBusy);
				}
				auto UUID = MicroService::CreateUUID();
				auto RPC = CommandManager()->NextRPCId();
				return (*this.*Command.funPtr)(UUID,RPC);
			}
		}
		return BadRequest(RESTAPI::Errors::InvalidCommand);
	}

	void RESTAPI_device_commandHandler::GetCapabilities() {
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
		Logger_.information(fmt::format("DELETE-CAPABILITIES: user={} serial={}", Requester(), SerialNumber_));
		if (StorageService()->DeleteDeviceCapabilities(SerialNumber_)) {
			return OK();
		}
		NotFound();
	}

	void RESTAPI_device_commandHandler::GetStatistics() {
		if (QB_.LastOnly) {
			std::string Stats;
			if (DeviceRegistry()->GetStatistics(SerialNumber_, Stats)) {
				Poco::JSON::Parser P;
				if (Stats.empty())
					Stats = uCentralProtocol::EMPTY_JSON_DOC;
				auto Obj = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
				return ReturnObject(*Obj);
			} else {
				return NotFound();
			}
		} else {
			std::vector<GWObjects::Statistics> Stats;
			if (QB_.Newest) {
				StorageService()->GetNewestStatisticsData(SerialNumber_, QB_.Limit, Stats);
			} else {
				StorageService()->GetStatisticsData(SerialNumber_, QB_.StartDate, QB_.EndDate,
													 QB_.Offset, QB_.Limit, Stats);
			}
			Poco::JSON::Array ArrayObj;
			for (const auto &i : Stats) {
				Poco::JSON::Object Obj;
				i.to_json(Obj);
				ArrayObj.add(Obj);
			}
			Poco::JSON::Object RetObj;
			RetObj.set(RESTAPI::Protocol::DATA, ArrayObj);
			RetObj.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
			return ReturnObject(RetObj);
		}
	}

	void RESTAPI_device_commandHandler::DeleteStatistics() {
		Logger_.information(fmt::format("DELETE-STATISTICS: user={} serial={}", Requester(), SerialNumber_));
		if (StorageService()->DeleteStatisticsData(SerialNumber_, QB_.StartDate, QB_.EndDate)) {
			return OK();
		}
		NotFound();
	}

	void RESTAPI_device_commandHandler::Ping(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("PING({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));
		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {
			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("PING", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			GWObjects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
			Cmd.Command = uCentralProtocol::PING;
			Cmd.RunAt = 0;

			Poco::JSON::Object Params;
			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			std::stringstream ParamStream;

			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 60000ms, nullptr, nullptr, Logger_);

			GWObjects::CommandDetails Cmd2;
			if(StorageService()->GetCommand(CMD_UUID,Cmd2)) {
				Poco::JSON::Object	Answer;
				// Answer.set("latency", Cmd2.executionTime);
				Answer.set("latency", fmt::format("{:.3f}ms.",Cmd.executionTime));
				Answer.set("serialNumber", SerialNumber_);
				Answer.set("currentUTCTime", std::chrono::duration_cast<std::chrono::milliseconds>(
												 std::chrono::system_clock::now().time_since_epoch()).count());
				try {
					Poco::JSON::Parser	P;
					auto ResponseObj = P.parse(Cmd2.Results).extract<Poco::JSON::Object::Ptr>();
					if(ResponseObj->has("results")) {
						auto Results = ResponseObj->get("results").extract<Poco::JSON::Object::Ptr>();
						if(Results->has("deviceUTCTime"))
							Answer.set("deviceUTCTime",Results->has("deviceUTCTime"));
					}
				} catch (...) {

				}
				return ReturnObject(Answer);
			}
			return NotFound();
		}
		return BadRequest(RESTAPI::Errors::MissingSerialNumber);
	}

	void RESTAPI_device_commandHandler::CallCanceled(const char * Cmd, const std::string &UUID, uint64_t RPC, const OpenWifi::RESTAPI::Errors::msg &Err) {
		Logger_.warning(fmt::format("{}({},{}): Canceled. Error:{} Reason:{}", Cmd, UUID, RPC, Err.err_num, Err.err_txt));
	}

	void RESTAPI_device_commandHandler::Script(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("SCRIPT({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));
		if(!Internal_ && UserInfo_.userinfo.userRole!=SecurityObjects::ROOT) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC,RESTAPI::Errors::ACCESS_DENIED);
			return UnAuthorized(RESTAPI::Errors::ACCESS_DENIED);
		}

		const auto &Obj = ParsedBody_;
		GWObjects::ScriptRequest	SCR;
		if(!SCR.from_json(Obj)) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC,RESTAPI::Errors::InvalidJSONDocument);
			return BadRequest(RESTAPI::Errors::InvalidJSONDocument);
		}

		if (SCR.serialNumber.empty() ||
			SCR.script.empty() ||
			SCR.type.empty() ||
			SCR.scriptId.empty() ||
			(SCR.type!="shell" && SCR.type!="ucode")) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC,RESTAPI::Errors::MissingOrInvalidParameters);
			return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
		}

		if (SerialNumber_ != SCR.serialNumber) {
			CallCanceled("SCRIPT", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
			return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
		}

		uint64_t timeout = SCR.timeout==0 ? 30 : SCR.timeout;

		GWObjects::CommandDetails Cmd;
		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = CMD_UUID;
		Cmd.SubmittedBy = UserInfo_.webtoken.username_;
		Cmd.Command = uCentralProtocol::SCRIPT;
		Cmd.RunAt = 0;

		Poco::JSON::Object Params;
		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::TIMEOUT, timeout);
		Params.set(uCentralProtocol::TYPE, SCR.type);
		Params.set(uCentralProtocol::SCRIPT, SCR.script);
		Params.set(uCentralProtocol::WHEN, SCR.when);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();

		return RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 120000ms, nullptr, this, Logger_);
	}

	void RESTAPI_device_commandHandler::GetStatus() {
		GWObjects::ConnectionState State;

		if (DeviceRegistry()->GetState(SerialNumber_, State)) {
			Poco::JSON::Object RetObject;
			State.to_json(RetObject);
			return ReturnObject(RetObject);
		} else {
			Poco::JSON::Object RetObject;
			RetObject.set("serialNumber", SerialNumber_);
			RetObject.set("connected", false);
			return ReturnObject(RetObject);
		}
	}

	void RESTAPI_device_commandHandler::Configure(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("CONFIGURE({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(RESTAPI::Protocol::UUID) &&
			Obj->has(RESTAPI::Protocol::CONFIGURATION)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("CONFIGURE", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Configuration = GetS(RESTAPI::Protocol::CONFIGURATION, Obj,uCentralProtocol::EMPTY_JSON_DOC);
			std::string Error;
			if (!ValidateUCentralConfiguration(Configuration, Error)) {
				CallCanceled("CONFIGURE", CMD_UUID, CMD_RPC,RESTAPI::Errors::ConfigBlockInvalid);
				return BadRequest(RESTAPI::Errors::ConfigBlockInvalid);
			}

			auto When = GetWhen(Obj);
			uint64_t NewUUID;

			if (StorageService()->UpdateDeviceConfiguration(SerialNumber_, Configuration, NewUUID)) {
				GWObjects::CommandDetails Cmd;

				Cmd.SerialNumber = SerialNumber_;
				Cmd.UUID = CMD_UUID;
				Cmd.SubmittedBy = UserInfo_.webtoken.username_;
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

				DeviceRegistry()->SetPendingUUID(SerialNumber_, NewUUID);
				return RESTAPI_RPC::WaitForCommand(CMD_RPC,true,Cmd, Params, *Request, *Response, 60000ms, nullptr, this, Logger_);
			}
			return BadRequest(RESTAPI::Errors::RecordNotUpdated);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::Upgrade(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("UPGRADE({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;

		if (Obj->has(RESTAPI::Protocol::URI) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("UPGRADE", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto URI = GetS(RESTAPI::Protocol::URI, Obj);
			auto When = GetWhen(Obj);

			auto KeepRedirector = GetB(RESTAPI::Protocol::KEEPREDIRECTOR, Obj, true);

			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
			Cmd.Command = uCentralProtocol::UPGRADE;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::URI, URI);
			Params.set(uCentralProtocol::KEEP_REDIRECTOR, KeepRedirector ? 1 : 0);
			Params.set(uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC,true,Cmd,Params, *Request, *Response, 60000ms, nullptr, this, Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::GetLogs() {
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
		Logger_.information(fmt::format("DELETE-LOGS: user={} serial={}", Requester(), SerialNumber_));
		if (StorageService()->DeleteLogData(SerialNumber_, QB_.StartDate, QB_.EndDate,
											 QB_.LogType)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::NoRecordsDeleted);
	}

	void RESTAPI_device_commandHandler::GetChecks() {
		std::vector<GWObjects::HealthCheck> Checks;

		if (QB_.LastOnly) {
			GWObjects::HealthCheck	HC;
			if (DeviceRegistry()->GetHealthcheck(SerialNumber_, HC)) {
				Poco::JSON::Object	Answer;
				HC.to_json(Answer);
				return ReturnObject(Answer);
			} else {
				return NotFound();
			}
		} else {
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
		Logger_.information(fmt::format("DELETE-HEALTHCHECKS: user={} serial={}", Requester(), SerialNumber_));
		if (StorageService()->DeleteHealthCheckData(SerialNumber_, QB_.StartDate, QB_.EndDate)) {
			return OK();
		}
		BadRequest(RESTAPI::Errors::NoRecordsDeleted);
	}

	void RESTAPI_device_commandHandler::ExecuteCommand(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("EXECUTE({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::COMMAND) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(RESTAPI::Protocol::PAYLOAD)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("EXECUTE", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Command = GetS(RESTAPI::Protocol::COMMAND, Obj);
			auto Payload = GetS(RESTAPI::Protocol::PAYLOAD, Obj);
			auto When = GetWhen(Obj);

			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
			Cmd.Command = Command;
			Cmd.Custom = 1;
			Cmd.RunAt = When;

			Poco::JSON::Parser parser2;

			Poco::Dynamic::Var result = parser2.parse(Payload);
			const auto &PayloadObject = result.extract<Poco::JSON::Object::Ptr>();

			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::COMMAND, Command);
			Params.set(uCentralProtocol::WHEN, When);
			Params.set(uCentralProtocol::PAYLOAD, PayloadObject);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 60000ms, nullptr, this, Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::Reboot(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("REBOOT({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {
			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("REBOOT", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			uint64_t When = GetWhen(Obj);
			GWObjects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
			Cmd.Command = uCentralProtocol::REBOOT;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 60000ms, nullptr, this, Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingSerialNumber);
	}

	void RESTAPI_device_commandHandler::Factory(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("FACTORY-RESET({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::KEEPREDIRECTOR) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();

			if (SerialNumber_ != SNum) {
				CallCanceled("FACTORY-RESET", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto KeepRedirector = GetB(RESTAPI::Protocol::KEEPREDIRECTOR, Obj, true);
			uint64_t When = GetWhen(Obj);

			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
			Cmd.Command = uCentralProtocol::FACTORY;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::KEEP_REDIRECTOR, KeepRedirector ? 1 : 0);
			Params.set(uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			return RESTAPI_RPC::WaitForCommand(CMD_RPC,true,Cmd, Params, *Request, *Response, 60000ms, nullptr, this, Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::LEDs(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("LEDS({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;

		if (Obj->has(uCentralProtocol::PATTERN) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("LEDS", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Pattern =
				GetS(uCentralProtocol::PATTERN, Obj, uCentralProtocol::BLINK);
			if (Pattern != uCentralProtocol::ON &&
				Pattern != uCentralProtocol::OFF &&
				Pattern != uCentralProtocol::BLINK) {
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}

			auto Duration = Get(uCentralProtocol::DURATION, Obj, 30);
			auto When = GetWhen(Obj);

			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
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

			return RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 60000ms, nullptr, this, Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::Trace(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("TRACE({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;

		if 	(Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			(Obj->has(RESTAPI::Protocol::NETWORK) ||
			 Obj->has(RESTAPI::Protocol::INTERFACE))) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("TRACE", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Network = GetS(RESTAPI::Protocol::NETWORK, Obj);
			auto Interface = GetS(RESTAPI::Protocol::INTERFACE, Obj);
			auto URI = FileUploader()->FullName() + CMD_UUID;

			GWObjects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
			Cmd.Command = uCentralProtocol::TRACE;
			Cmd.RunAt = 0;
			Cmd.WaitingForFile = 1;
			Cmd.AttachType = RESTAPI::Protocol::PCAP_FILE_TYPE;

			Poco::JSON::Object Params;

			if(Obj->has(RESTAPI::Protocol::DURATION))
				Params.set(uCentralProtocol::DURATION, Get(RESTAPI::Protocol::DURATION, Obj, 30) );
			else if(Obj->has(RESTAPI::Protocol::NUMBEROFPACKETS))
				Params.set(uCentralProtocol::PACKETS,Get(RESTAPI::Protocol::NUMBEROFPACKETS, Obj, 100));
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

			FileUploader()->AddUUID(CMD_UUID);
			return RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 160000ms, nullptr, this, Logger_);
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::WifiScan(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("WIFISCAN({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));
		const auto &Obj = ParsedBody_;

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			CallCanceled("WIFISCAN", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
			return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
		}

		bool OverrideDFS = GetB(RESTAPI::Protocol::OVERRIDEDFS, Obj, true);
		bool ActiveScan = GetB(RESTAPI::Protocol::ACTIVESCAN, Obj, false);
		uint64_t Bandwidth = Get(RESTAPI::Protocol::BANDWIDTH, Obj, (uint64_t) 0);

		Poco::JSON::Array::Ptr ies;
		if(Obj->has("ies") && Obj->isArray("ies")) {
			ies = Obj->getArray("ies");
		}

		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = CMD_UUID;
		Cmd.SubmittedBy = UserInfo_.webtoken.username_;
		Cmd.Command = uCentralProtocol::WIFISCAN;

		Poco::JSON::Object Params;

		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::OVERRIDEDFS, OverrideDFS);
		Params.set(uCentralProtocol::ACTIVE, ActiveScan);
		if(ies)
			Params.set(uCentralProtocol::IES, ies);
		if(Bandwidth!=0)
			Params.set(uCentralProtocol::BANDWIDTH, Bandwidth);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();
		RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 30000ms, nullptr, this, Logger_);
		if (Cmd.ErrorCode == 0) {
			KafkaManager()->PostMessage(KafkaTopics::WIFISCAN, SerialNumber_, Cmd.Results);
		}
	}

	void RESTAPI_device_commandHandler::EventQueue(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("EVENT-QUEUE({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->isArray(RESTAPI::Protocol::TYPES)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("EVENT-QUEUE", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			auto Types = Obj->getArray(RESTAPI::Protocol::TYPES);
			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = CMD_UUID;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
			Cmd.Command = uCentralProtocol::EVENT;

			Poco::JSON::Object Params;
			Params.set(uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentralProtocol::TYPES, Types);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 60000ms, nullptr, this, Logger_);
			if(Cmd.ErrorCode==0) {
				KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, SerialNumber_,
											Cmd.Results);
			}
			return;
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::MakeRequest(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("FORCE-REQUEST({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(uCentralProtocol::MESSAGE)) {

			auto SNum = GetS(RESTAPI::Protocol::SERIALNUMBER, Obj);
			auto MessageType = GetS(uCentralProtocol::MESSAGE, Obj);

			if ((SerialNumber_ != SNum) ||
				(MessageType != uCentralProtocol::STATE &&
				 MessageType != uCentralProtocol::HEALTHCHECK)) {
				CallCanceled("FORCE-REQUEST", CMD_UUID, CMD_RPC,RESTAPI::Errors::MissingOrInvalidParameters);
				return BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
			}

			auto When = GetWhen(Obj);
			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
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

			return RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 60000ms, nullptr, this, Logger_ );
		}
		BadRequest(RESTAPI::Errors::MissingOrInvalidParameters);
	}

	void RESTAPI_device_commandHandler::Rtty(const std::string &CMD_UUID, uint64_t CMD_RPC) {
		Logger_.information(fmt::format("RTTY({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		if (MicroService::instance().ConfigGetBool("rtty.enabled", false)) {
			GWObjects::Device	Device;

			if (StorageService()->GetDevice(SerialNumber_, Device)) {

				GWObjects::RttySessionDetails Rtty{
					.SerialNumber = SerialNumber_,
					.Server = MicroService::instance().ConfigGetString("rtty.server", "localhost"),
					.Port = MicroService::instance().ConfigGetInt("rtty.port", 5912),
					.Token = MicroService::instance().ConfigGetString("rtty.token", "nothing"),
					.TimeOut = MicroService::instance().ConfigGetInt("rtty.timeout", 60),
					.ConnectionId =  Utils::ComputeHash(SerialNumber_,OpenWifi::Now()).substr(0,32),
					.Started = OpenWifi::Now(),
					.CommandUUID = CMD_UUID,
					.ViewPort = MicroService::instance().ConfigGetInt("rtty.viewport", 5913),
					.DevicePassword = ""
				};

				if(RTTYS_server()->UseInternal()) {
					// Rtty.Token = MicroService::instance().CreateHash(UserInfo_.webtoken.refresh_token_ + std::to_string(OpenWifi::Now())).substr(0,32);
					Rtty.Token = Utils::ComputeHash(UserInfo_.webtoken.refresh_token_,OpenWifi::Now()).substr(0,32);
					if(!RTTYS_server()->CreateEndPoint(Rtty.ConnectionId, Rtty.Token, Requester(), SerialNumber_)) {
						return BadRequest(RESTAPI::Errors::MaximumRTTYSessionsReached);
					}
				}

				Poco::JSON::Object ReturnedObject;
				Rtty.to_json(ReturnedObject);

				//	let's create the command for this request
				GWObjects::CommandDetails Cmd;
				Cmd.SerialNumber = SerialNumber_;
				Cmd.SubmittedBy = UserInfo_.webtoken.username_;
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
				Logger_.information(fmt::format("RTTY: user={} serial={} rttyid={} token={} cmd={}.", Requester(), SerialNumber_, Rtty.ConnectionId, Rtty.Token, CMD_UUID));
				return RESTAPI_RPC::WaitForCommand(CMD_RPC,false,Cmd, Params, *Request, *Response, 60000ms, &ReturnedObject, this, Logger_);
			}
			return NotFound();
		}
		Logger_.information(fmt::format("RTTY: user={} serial={}. Internal error.", Requester(), SerialNumber_));
		return ReturnStatus(Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
	}

	void RESTAPI_device_commandHandler::Telemetry(const std::string &CMD_UUID, uint64_t CMD_RPC){
		Logger_.information(fmt::format("TELEMETRY({},{}): user={} serial={}", CMD_UUID, CMD_RPC, Requester(), SerialNumber_));

		const auto &Obj = ParsedBody_;
		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(RESTAPI::Protocol::INTERVAL) &&
			Obj->has(RESTAPI::Protocol::TYPES)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				CallCanceled("TELEMETRY", CMD_UUID, CMD_RPC,RESTAPI::Errors::SerialNumberMismatch);
				return BadRequest(RESTAPI::Errors::SerialNumberMismatch);
			}

			std::stringstream 	oooss;
			Obj->stringify(oooss);
			// std::cout << "Payload:" << oooss.str() << std::endl;

			uint64_t Lifetime = 60 * 60 ; // 1 hour
			uint64_t Interval = 5;
			bool KafkaOnly = false;

			if(Obj->has("kafka")) {
				KafkaOnly = Obj->get("kafka").toString()=="true";
			}

			auto StatusOnly = GetBoolParameter("statusOnly",false);

			AssignIfPresent(Obj, RESTAPI::Protocol::INTERVAL, Interval);
			AssignIfPresent(Obj, RESTAPI::Protocol::LIFETIME, Lifetime);

			Poco::JSON::Object Answer;

			auto IntSerialNumber = Utils::SerialNumberToInt(SerialNumber_);

			if(!StatusOnly) {
				if (KafkaOnly) {
					if (Interval) {
						DeviceRegistry()->SetKafkaTelemetryReporting(IntSerialNumber, Interval, Lifetime);
						Answer.set("action", "Kafka telemetry started.");
						Answer.set("uuid", CMD_UUID);
					} else {
						DeviceRegistry()->StopKafkaTelemetry(IntSerialNumber);
						Answer.set("action", "Kafka telemetry stopped.");
					}
				} else {
					if (Interval) {
						DeviceRegistry()->SetWebSocketTelemetryReporting(IntSerialNumber, Interval,
																				  Lifetime);
						std::string EndPoint;
						if (TelemetryStream()->CreateEndpoint(Utils::SerialNumberToInt(SerialNumber_), EndPoint, CMD_UUID)) {
							Answer.set("action", "WebSocket telemetry started.");
							Answer.set("serialNumber", SerialNumber_);
							Answer.set("uuid", CMD_UUID);
							Answer.set("uri", EndPoint);
						} else {
							return BadRequest(RESTAPI::Errors::InternalError);
						}
					} else {
						Answer.set("action", "WebSocket telemetry stopped.");
						DeviceRegistry()->StopWebSocketTelemetry(IntSerialNumber);
					}
				}
			} else {
				Answer.set("action", "Telemetry status only.");
			}

			bool TelemetryRunning;
			uint64_t TelemetryWebSocketCount, TelemetryKafkaCount, TelemetryInterval,
				TelemetryWebSocketTimer, TelemetryKafkaTimer, TelemetryWebSocketPackets,
				TelemetryKafkaPackets;
			DeviceRegistry()->GetTelemetryParameters(IntSerialNumber,TelemetryRunning,
															  TelemetryInterval,
															  TelemetryWebSocketTimer,
															  TelemetryKafkaTimer,
															  TelemetryWebSocketCount,
															  TelemetryKafkaCount,
															  TelemetryWebSocketPackets,
															  TelemetryKafkaPackets);
			Poco::JSON::Object	TelemetryStatus;
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
}
