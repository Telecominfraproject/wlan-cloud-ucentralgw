//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <chrono>
#include <algorithm>

#include "Poco/UUIDGenerator.h"
#include "Poco/JSON/Parser.h"

#include "CentralConfig.h"
#include "Daemon.h"
#include "DeviceRegistry.h"
#include "FileUploader.h"
#include "RESTAPI_GWobjects.h"
#include "RESTAPI_device_commandHandler.h"
#include "StorageService.h"
#include "Utils.h"

#include "uCentralProtocol.h"
#include "RESTAPI_protocol.h"
#include "RESTAPI_RPC.h"

#include "CommandManager.h"
#include "KafkaManager.h"
#include "Kafka_topics.h"
#include "TelemetryStream.h"

namespace OpenWifi {

	void RESTAPI_device_commandHandler::DoGet() {
		if(!ValidateParameters()) {
			BadRequest("Missing command or serial number.");
			return;
		}

		if (Command_ == RESTAPI::Protocol::CAPABILITIES){
			GetCapabilities();
		} else if (Command_ == RESTAPI::Protocol::LOGS) {
			GetLogs();
		} else if (Command_ == RESTAPI::Protocol::HEALTHCHECKS) {
			GetChecks();
		} else if (Command_ == RESTAPI::Protocol::STATISTICS) {
			GetStatistics();
		} else if (Command_ == RESTAPI::Protocol::STATUS) {
			GetStatus();
		} else if (Command_ == RESTAPI::Protocol::RTTY) {
			Rtty();
		} else {
			BadRequest("Bad command.");
		}
	}


	void RESTAPI_device_commandHandler::DoDelete() {
		if(!ValidateParameters()) {
			BadRequest("Missing command or serial number.");
			return;
		}
		if (Command_ == RESTAPI::Protocol::CAPABILITIES) {
			DeleteCapabilities();
		} else if (Command_ == RESTAPI::Protocol::LOGS){
			DeleteLogs();
		} else if (Command_ == RESTAPI::Protocol::HEALTHCHECKS){
			DeleteChecks();
		} else if (Command_ == RESTAPI::Protocol::STATISTICS) {
			DeleteStatistics();
		} else {
			BadRequest("Unknown command.");
		}
	}

	void RESTAPI_device_commandHandler::DoPost() {
		if(!ValidateParameters()) {
			BadRequest("Missing command or serial number.");
			return;
		}
		if (Command_ == RESTAPI::Protocol::PERFORM) {
			ExecuteCommand();
		} else if (Command_ == RESTAPI::Protocol::CONFIGURE) {
			Configure();
		} else if (Command_ == RESTAPI::Protocol::UPGRADE) {
			Upgrade();
		} else if (Command_ == RESTAPI::Protocol::REBOOT) {
			Reboot();
		} else if (Command_ == RESTAPI::Protocol::FACTORY) {
			Factory();
		} else if (Command_ == RESTAPI::Protocol::LEDS) {
			LEDs();
		} else if (Command_ == RESTAPI::Protocol::TRACE) {
			Trace();
		} else if (Command_ == RESTAPI::Protocol::REQUEST) {
			MakeRequest();
		} else if (Command_ == RESTAPI::Protocol::WIFISCAN) {
			WifiScan();
		} else if (Command_ == RESTAPI::Protocol::EVENTQUEUE) {
			EventQueue();
		} else if (Command_ == RESTAPI::Protocol::TELEMETRY) {
			Telemetry();
		} else {
			BadRequest("Unknown command.");
		}
	}

void RESTAPI_device_commandHandler::GetCapabilities() {
	GWObjects::Capabilities Caps;
	if (Storage()->GetDeviceCapabilities(SerialNumber_, Caps)) {
		Poco::JSON::Object RetObj;
		Caps.to_json(RetObj);
		RetObj.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
		ReturnObject(RetObj);
	} else {
		NotFound();
	}
}

void RESTAPI_device_commandHandler::DeleteCapabilities() {
	if (Storage()->DeleteDeviceCapabilities(SerialNumber_))
		OK();
	else
		NotFound();
}

void RESTAPI_device_commandHandler::GetStatistics() {
	if (QB_.Lifetime) {
		std::string Stats;
		Storage()->GetLifetimeStats(SerialNumber_, Stats);
		Poco::JSON::Parser P;
		if (Stats.empty())
			Stats = uCentralProtocol::EMPTY_JSON_DOC;
		auto Obj = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
		ReturnObject(*Obj);
	} else if (QB_.LastOnly) {
		std::string Stats;
		if (DeviceRegistry()->GetStatistics(SerialNumber_, Stats)) {
			Poco::JSON::Parser P;
			if (Stats.empty())
				Stats = uCentralProtocol::EMPTY_JSON_DOC;
			auto Obj = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
			ReturnObject(*Obj);
		} else {
			NotFound();
		}
	} else {
		std::vector<GWObjects::Statistics> Stats;
		if (QB_.Newest) {
			Storage()->GetNewestStatisticsData(SerialNumber_, QB_.Limit, Stats);
		} else {
			Storage()->GetStatisticsData(SerialNumber_, QB_.StartDate, QB_.EndDate,
												 QB_.Offset, QB_.Limit, Stats);
		}
		Poco::JSON::Array ArrayObj;
		for (auto i : Stats) {
			Poco::JSON::Object Obj;
			i.to_json(Obj);
			ArrayObj.add(Obj);
		}
		Poco::JSON::Object RetObj;
		RetObj.set(RESTAPI::Protocol::DATA, ArrayObj);
		RetObj.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
		ReturnObject(RetObj);
	}
}

void RESTAPI_device_commandHandler::DeleteStatistics() {
	if (QB_.Lifetime) {
		if (Storage()->ResetLifetimeStats(SerialNumber_)) {
			OK();
		} else {
			NotFound();
		}
	} else {
		if (Storage()->DeleteStatisticsData(SerialNumber_, QB_.StartDate,
													QB_.EndDate)) {
			OK();
		} else {
			NotFound();
		}
	}
}

void RESTAPI_device_commandHandler::GetStatus() {
	GWObjects::ConnectionState State;

	if (DeviceRegistry()->GetState(SerialNumber_, State)) {
		Poco::JSON::Object RetObject;
		State.to_json(RetObject);
		ReturnObject(RetObject);
	} else {
		NotFound();
	}
}

void RESTAPI_device_commandHandler::Configure() {
	//  get the configuration from the body of the message
	auto Obj = ParseStream();
	if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
		Obj->has(RESTAPI::Protocol::UUID) &&
		Obj->has(RESTAPI::Protocol::CONFIGURATION)) {

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			BadRequest("Missing serial number in configuration.");
			return;
		}

		auto Configuration = GetS(RESTAPI::Protocol::CONFIGURATION, Obj,uCentralProtocol::EMPTY_JSON_DOC);
		auto When = GetWhen(Obj);
		uint64_t NewUUID;
		if (Storage()->UpdateDeviceConfiguration(SerialNumber_, Configuration, NewUUID)) {
			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = Daemon()->CreateUUID();
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
			RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
			return;
		}
	}
}

void RESTAPI_device_commandHandler::Upgrade() {
	auto Obj = ParseStream();

	if (Obj->has(RESTAPI::Protocol::URI) &&
		Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			BadRequest("Missing serial number.");
			return;
		}

		auto URI = GetS(RESTAPI::Protocol::URI, Obj);
		auto When = GetWhen(Obj);

		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = Daemon()->CreateUUID();
		Cmd.SubmittedBy = UserInfo_.webtoken.username_;
		Cmd.Command = uCentralProtocol::UPGRADE;
		Cmd.RunAt = When;

		Poco::JSON::Object Params;

		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::URI, URI);
		Params.set(uCentralProtocol::WHEN, When);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();

		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
		return;
	} else {
		BadRequest("Missing URI/Serial number.");
	}
}

void RESTAPI_device_commandHandler::GetLogs() {
	std::vector<GWObjects::DeviceLog> Logs;
	if (QB_.Newest) {
		Storage()->GetNewestLogData(SerialNumber_, QB_.Limit, Logs, QB_.LogType);
	} else {
		Storage()->GetLogData(SerialNumber_, QB_.StartDate, QB_.EndDate, QB_.Offset,
									  QB_.Limit, Logs, QB_.LogType);
	}

	Poco::JSON::Array ArrayObj;
	for (auto i : Logs) {
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
	if (Storage()->DeleteLogData(SerialNumber_, QB_.StartDate, QB_.EndDate,
										 QB_.LogType)) {
		OK();
	} else {
		BadRequest("Could not delete the selected logs.");
	}
}

void RESTAPI_device_commandHandler::GetChecks() {
	std::vector<GWObjects::HealthCheck> Checks;

	if (QB_.LastOnly) {
		GWObjects::HealthCheck	HC;
		if (DeviceRegistry()->GetHealthcheck(SerialNumber_, HC)) {
			Poco::JSON::Object	Answer;
			HC.to_json(Answer);
			ReturnObject(Answer);
		} else {
			NotFound();
		}
	} else {
		if (QB_.Newest) {
			Storage()->GetNewestHealthCheckData(SerialNumber_, QB_.Limit, Checks);
		} else {
			Storage()->GetHealthCheckData(SerialNumber_, QB_.StartDate, QB_.EndDate,
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
	if (Storage()->DeleteHealthCheckData(SerialNumber_, QB_.StartDate, QB_.EndDate)) {
		OK();
	} else {
		BadRequest("Healthchecks could not be deleted.");
	}
}

void RESTAPI_device_commandHandler::ExecuteCommand() {
	//  get the configuration from the body of the message
	auto Obj = ParseStream();
	if (Obj->has(RESTAPI::Protocol::COMMAND) &&
		Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
		Obj->has(RESTAPI::Protocol::PAYLOAD)) {

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			BadRequest("Missing serial number.");
			return;
		}

		auto Command = GetS(RESTAPI::Protocol::COMMAND, Obj);
		auto Payload = GetS(RESTAPI::Protocol::PAYLOAD, Obj);
		auto When = GetWhen(Obj);

		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = Daemon()->CreateUUID();
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

		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
		return;
	} else {
		BadRequest("Missing command parameters");
	}
}

void RESTAPI_device_commandHandler::Reboot() {
	auto Obj = ParseStream();

	if (Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {
		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			BadRequest("Missing serial number.");
			return;
		}

		uint64_t When = GetWhen(Obj);
		GWObjects::CommandDetails Cmd;
		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = Daemon()->CreateUUID();
		Cmd.SubmittedBy = UserInfo_.webtoken.username_;
		Cmd.Command = uCentralProtocol::REBOOT;
		Cmd.RunAt = When;

		Poco::JSON::Object Params;

		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::WHEN, When);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();

		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
		return;
	} else {
		BadRequest("Missing serial number.");
	}
}

void RESTAPI_device_commandHandler::Factory() {
	Poco::JSON::Object::Ptr Obj = ParseStream();
	if (Obj->has(RESTAPI::Protocol::KEEPREDIRECTOR) &&
		Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();

		if (SerialNumber_ != SNum) {
			BadRequest("Missing serial number.");
			return;
		}

		auto KeepRedirector = GetB(RESTAPI::Protocol::KEEPREDIRECTOR, Obj, true);
		uint64_t When = GetWhen(Obj);

		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = Daemon()->CreateUUID();
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

		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
		return;
	} else {
		BadRequest("Missing parameters.");
	}
}

void RESTAPI_device_commandHandler::LEDs() {
	auto Obj = ParseStream();

	if (Obj->has(uCentralProtocol::PATTERN) &&
		Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			BadRequest("Missing serial number.");
			return;
		}

		auto Pattern =
			GetS(uCentralProtocol::PATTERN, Obj, uCentralProtocol::BLINK);
		if (Pattern != uCentralProtocol::ON &&
			Pattern != uCentralProtocol::OFF &&
			Pattern != uCentralProtocol::BLINK) {
			Logger_.warning(Poco::format("LEDs(%s): Bad pattern", SerialNumber_));
			BadRequest("Missing parameters.");
			return;
		}

		auto Duration = Get(uCentralProtocol::DURATION, Obj, 30);
		auto When = GetWhen(Obj);
		Logger_.information(Poco::format("LEDS(%s): Pattern:%s Duration: %Lu", SerialNumber_,
										 Pattern, Duration));

		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = Daemon()->CreateUUID();
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

		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
		return;
	} else {
		BadRequest("Missing parameters.");
	}
}

void RESTAPI_device_commandHandler::Trace() {
	auto Obj = ParseStream();

	if 	(Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
		(Obj->has(RESTAPI::Protocol::NETWORK) ||
		 Obj->has(RESTAPI::Protocol::INTERFACE))) {

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			BadRequest("Missing serial number.");
			return;
		}

		auto Duration = Get(RESTAPI::Protocol::DURATION, Obj, 30);
		auto When = GetWhen(Obj);
		auto NumberOfPackets = Get(RESTAPI::Protocol::NUMBEROFPACKETS, Obj, 100);

		auto Network = GetS(RESTAPI::Protocol::NETWORK, Obj);
		auto Interface = GetS(RESTAPI::Protocol::INTERFACE, Obj);
		auto UUID = Daemon()->CreateUUID();
		auto URI = FileUploader()->FullName() + UUID;

		GWObjects::CommandDetails Cmd;
		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = UUID;
		Cmd.SubmittedBy = UserInfo_.webtoken.username_;
		Cmd.Command = uCentralProtocol::TRACE;
		Cmd.RunAt = When;
		Cmd.WaitingForFile = 1;
		Cmd.AttachType = RESTAPI::Protocol::PCAP_FILE_TYPE;

		Poco::JSON::Object Params;

		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::DURATION, Duration);
		Params.set(uCentralProtocol::WHEN, When);
		Params.set(uCentralProtocol::PACKETS, NumberOfPackets);
		Params.set(uCentralProtocol::NETWORK, Network);
		Params.set(uCentralProtocol::INTERFACE, Interface);
		Params.set(uCentralProtocol::URI, URI);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();

		FileUploader()->AddUUID(UUID);
		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
	} else {
		BadRequest("Missing SerialNumber, Network, or Interface.");
	}
}

void RESTAPI_device_commandHandler::WifiScan() {
	auto Obj = ParseStream();

	auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
	if (SerialNumber_ != SNum) {
		BadRequest("Missing serial number.");
		return;
	}

	if ((Obj->has(RESTAPI::Protocol::BANDS) &&
			 Obj->isArray(RESTAPI::Protocol::BANDS) ||
		 (Obj->has(RESTAPI::Protocol::CHANNELS) &&
		  Obj->isArray(RESTAPI::Protocol::CHANNELS)) ||
		 (!Obj->has(RESTAPI::Protocol::BANDS) &&
		  !Obj->has(RESTAPI::Protocol::CHANNELS)))) {
		bool Verbose = GetB(RESTAPI::Protocol::VERBOSE, Obj);
		auto UUID = Daemon()->CreateUUID();
		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = UUID;
		Cmd.SubmittedBy = UserInfo_.webtoken.username_;
		Cmd.Command = uCentralProtocol::WIFISCAN;

		Poco::JSON::Object Params;

		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::VERBOSE, Verbose);

		if (Obj->has(uCentralProtocol::BANDS)) {
			Params.set(uCentralProtocol::BANDS, Obj->get(RESTAPI::Protocol::BANDS));
		} else if (Obj->has(uCentralProtocol::CHANNELS)) {
			Params.set(uCentralProtocol::CHANNELS, Obj->get(RESTAPI::Protocol::CHANNELS));
		}

		if (Obj->has(RESTAPI::Protocol::ACTIVESCAN)) {
			Params.set(uCentralProtocol::ACTIVE,
					   (int)(Obj->get(RESTAPI::Protocol::ACTIVESCAN).toString() == "true") ? 1 : 0);
		} else {
			Params.set(uCentralProtocol::ACTIVE, 0);
		}

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();
		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
		if (Cmd.ErrorCode == 0) {
			KafkaManager()->PostMessage(KafkaTopics::WIFISCAN, SerialNumber_, Cmd.Results);
		}
	} else {
		BadRequest("Missing parameters.");
	}
}

void RESTAPI_device_commandHandler::EventQueue() {
	auto Obj = ParseStream();
	if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
		Obj->isArray(RESTAPI::Protocol::TYPES)) {

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		auto Types = Obj->getArray(RESTAPI::Protocol::TYPES);

		auto UUID = Daemon()->CreateUUID();
		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.UUID = UUID;
		Cmd.SubmittedBy = UserInfo_.webtoken.username_;
		Cmd.Command = uCentralProtocol::EVENT;

		Poco::JSON::Object Params;
		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::TYPES, Types);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();

		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_);
		if(Cmd.ErrorCode==0) {
			KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, SerialNumber_,
										Cmd.Results);
		}
	} else {
		BadRequest("Missing parameters.");
	}
}

void RESTAPI_device_commandHandler::MakeRequest() {
	auto Obj = ParseStream();
	if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
		Obj->has(uCentralProtocol::MESSAGE)) {

		auto SNum = GetS(RESTAPI::Protocol::SERIALNUMBER, Obj);
		auto MessageType = GetS(uCentralProtocol::MESSAGE, Obj);

		if ((SerialNumber_ != SNum) ||
			(MessageType != uCentralProtocol::STATE &&
			 MessageType != uCentralProtocol::HEALTHCHECK)) {
			BadRequest("Missing parameters");
			return;
		}

		auto When = GetWhen(Obj);
		GWObjects::CommandDetails Cmd;

		Cmd.SerialNumber = SerialNumber_;
		Cmd.SubmittedBy = UserInfo_.webtoken.username_;
		Cmd.UUID = Daemon()->CreateUUID();
		Cmd.Command = uCentralProtocol::REQUEST;
		Cmd.RunAt = When;

		Poco::JSON::Object Params;

		Params.set(uCentralProtocol::SERIAL, SerialNumber_);
		Params.set(uCentralProtocol::WHEN, When);
		Params.set(uCentralProtocol::MESSAGE, MessageType);
		Params.set(uCentralProtocol::REQUEST_UUID, Cmd.UUID);

		std::stringstream ParamStream;
		Params.stringify(ParamStream);
		Cmd.Details = ParamStream.str();

		RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, nullptr, this, Logger_ );
		return;
	} else {
		BadRequest("Missing parameters.");
	}
}

	void RESTAPI_device_commandHandler::Rtty() {
		if (Daemon()->ConfigGetString("rtty.enabled", "false") == "true") {
			GWObjects::Device	Device;
			if (Storage()->GetDevice(SerialNumber_, Device)) {
				auto CommandUUID = Daemon::instance()->CreateUUID();

				GWObjects::RttySessionDetails Rtty{
					.SerialNumber = SerialNumber_,
					.Server = Daemon()->ConfigGetString("rtty.server", "localhost"),
					.Port = Daemon()->ConfigGetInt("rtty.port", 5912),
					.Token = Daemon()->ConfigGetString("rtty.token", "nothing"),
					.TimeOut = Daemon()->ConfigGetInt("rtty.timeout", 60),
					.ConnectionId = CommandUUID,
					.Started = (uint64_t)time(nullptr),
					.CommandUUID = CommandUUID,
					.ViewPort = Daemon()->ConfigGetInt("rtty.viewport", 5913),

				};

				Poco::JSON::Object ReturnedObject;
				Rtty.to_json(ReturnedObject);

				//	let's create the command for this request
				GWObjects::CommandDetails Cmd;
				Cmd.SerialNumber = SerialNumber_;
				Cmd.SubmittedBy = UserInfo_.webtoken.username_;
				Cmd.UUID = CommandUUID;
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
				RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response, 60000, &ReturnedObject, this, Logger_);
			} else {
				NotFound();
			}
		} else {
			ReturnStatus(Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
		}
	}

	void RESTAPI_device_commandHandler::Telemetry(){
		auto Obj = ParseStream();

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(RESTAPI::Protocol::INTERVAL) && Obj->has(RESTAPI::Protocol::TYPES)) {

			auto SNum = GetS(RESTAPI::Protocol::SERIALNUMBER, Obj);

			if (SNum != SerialNumber_) {
				BadRequest("Serial Number mismatch");
				return;
			}

			GWObjects::Device Device;
			if (!Storage()->GetDevice(SerialNumber_, Device)) {
				NotFound();
				return;
			}

			if (!DeviceRegistry()->Connected(SerialNumber_)) {
				BadRequest("Device is not currently connected.");
				return;
			}

			auto Interval = Obj->get(RESTAPI::Protocol::INTERVAL);
			std::string UUID;
			if (Obj->has(RESTAPI::Protocol::UUID))
				UUID = Obj->get(RESTAPI::Protocol::UUID).toString();

			GWObjects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.SubmittedBy = UserInfo_.webtoken.username_;
			Cmd.Command = uCentralProtocol::TELEMETRY;
			Poco::JSON::Object Params;

			Params.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
			Params.set(RESTAPI::Protocol::INTERVAL, Interval);
			if (Interval > 0)
				Params.set(RESTAPI::Protocol::TYPES, Obj->getArray(RESTAPI::Protocol::TYPES));

			std::string Endpoint, NewUUID;
			Poco::JSON::Object Answer;
			if (Interval) {
				if (TelemetryStream()->CreateEndpoint(SerialNumber_, Endpoint, NewUUID)) {
					Answer.set("serialNumber", SerialNumber_);
					Answer.set("uuid", NewUUID);
					Answer.set("uri", Endpoint);
				}
			} else {
				BadRequest("Telemetry system could not create WS endpoint. Please try again.");
				return;
			}

			Cmd.UUID = NewUUID;
			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(Cmd, Params, *Request, *Response,
										60000, &Answer, this, Logger_);
		} else {
			BadRequest("Missing parameters.");
		}
	}
}
