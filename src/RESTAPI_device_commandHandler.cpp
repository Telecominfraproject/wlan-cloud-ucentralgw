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

void RESTAPI_device_commandHandler::handleRequest(Poco::Net::HTTPServerRequest &Request,
												  Poco::Net::HTTPServerResponse &Response) {
	try {
		if (!ContinueProcessing(Request, Response))
			return;

		if (!IsAuthorized(Request, Response))
			return;

		std::string Command = GetBinding(RESTAPI::Protocol::COMMAND, "");
		if (Command.empty()) {
			Logger_.error(Poco::format("Unrecognized command '%s'", Command));
			BadRequest(Request, Response);
			return;
		}

		SerialNumber_ = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");
		if (SerialNumber_.empty()) {
			Logger_.error(Poco::format("Missing serial number for command '%s'", Command));
			BadRequest(Request, Response);
			return;
		}

		ParseParameters(Request);
		if(!InitQueryBlock()) {
			BadRequest(Request, Response, "Illegal parameter value.");
			return;
		}

		if (Command == RESTAPI::Protocol::CAPABILITIES &&
			Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetCapabilities(Request, Response);
		} else if (Command == RESTAPI::Protocol::CAPABILITIES &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
			DeleteCapabilities(Request, Response);
		} else if (Command == RESTAPI::Protocol::LOGS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetLogs(Request, Response);
		} else if (Command == RESTAPI::Protocol::LOGS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
			DeleteLogs(Request, Response);
		} else if (Command == RESTAPI::Protocol::HEALTHCHECKS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetChecks(Request, Response);
		} else if (Command == RESTAPI::Protocol::HEALTHCHECKS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
			DeleteChecks(Request, Response);
		} else if (Command == RESTAPI::Protocol::STATISTICS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetStatistics(Request, Response);
		} else if (Command == RESTAPI::Protocol::STATISTICS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
			DeleteStatistics(Request, Response);
		} else if (Command == RESTAPI::Protocol::STATUS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetStatus(Request, Response);
		} else if (Command == RESTAPI::Protocol::PERFORM &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			ExecuteCommand(Request, Response);
		} else if (Command == RESTAPI::Protocol::CONFIGURE &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Configure(Request, Response);
		} else if (Command == RESTAPI::Protocol::UPGRADE &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Upgrade(Request, Response);
		} else if (Command == RESTAPI::Protocol::REBOOT &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Reboot(Request, Response);
		} else if (Command == RESTAPI::Protocol::FACTORY &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Factory(Request, Response);
		} else if (Command == RESTAPI::Protocol::LEDS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			LEDs(Request, Response);
		} else if (Command == RESTAPI::Protocol::TRACE &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Trace(Request, Response);
		} else if (Command == RESTAPI::Protocol::REQUEST &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			MakeRequest(Request, Response);
		} else if (Command == RESTAPI::Protocol::WIFISCAN &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			WifiScan(Request, Response);
		} else if (Command == RESTAPI::Protocol::EVENTQUEUE &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			EventQueue(Request, Response);
		} else if (Command == RESTAPI::Protocol::RTTY &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			Rtty(Request, Response);
		} else if (Command == RESTAPI::Protocol::TELEMETRY &&
					Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			Telemetry(Request, Response);
		} else {
			BadRequest(Request, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetCapabilities(Poco::Net::HTTPServerRequest &Request,
													Poco::Net::HTTPServerResponse &Response) {
	GWObjects::Capabilities Caps;
	try {
		if (Storage()->GetDeviceCapabilities(SerialNumber_, Caps)) {
			Poco::JSON::Object RetObj;
			Caps.to_json(RetObj);
			RetObj.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
			ReturnObject(Request, RetObj, Response);
		} else {
			NotFound(Request, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteCapabilities(Poco::Net::HTTPServerRequest &Request,
													   Poco::Net::HTTPServerResponse &Response) {
	try {
		if (Storage()->DeleteDeviceCapabilities(SerialNumber_))
			OK(Request, Response);
		else
			NotFound(Request, Response);
		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetStatistics(Poco::Net::HTTPServerRequest &Request,
												  Poco::Net::HTTPServerResponse &Response) {
	try {
		if (QB_.Lifetime) {
			std::string Stats;
			Storage()->GetLifetimeStats(SerialNumber_, Stats);
			Poco::JSON::Parser P;
			if (Stats.empty())
				Stats = uCentralProtocol::EMPTY_JSON_DOC;
			auto Obj = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
			ReturnObject(Request, *Obj, Response);
		} else if (QB_.LastOnly) {
			std::string Stats;
			if (DeviceRegistry()->GetStatistics(SerialNumber_, Stats)) {
				Poco::JSON::Parser P;
				if (Stats.empty())
					Stats = uCentralProtocol::EMPTY_JSON_DOC;
				auto Obj = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
				ReturnObject(Request, *Obj, Response);
			} else {
				NotFound(Request, Response);
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
			ReturnObject(Request, RetObj, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteStatistics(Poco::Net::HTTPServerRequest &Request,
													 Poco::Net::HTTPServerResponse &Response) {
	try {
		if (QB_.Lifetime) {
			if (Storage()->ResetLifetimeStats(SerialNumber_)) {
				OK(Request, Response);
			} else {
				NotFound(Request, Response);
			}
		} else {
			if (Storage()->DeleteStatisticsData(SerialNumber_, QB_.StartDate,
														QB_.EndDate)) {
				OK(Request, Response);
			} else {
				NotFound(Request, Response);
			}
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetStatus(Poco::Net::HTTPServerRequest &Request,
											  Poco::Net::HTTPServerResponse &Response) {
	try {
		GWObjects::ConnectionState State;

		if (DeviceRegistry()->GetState(SerialNumber_, State)) {
			Poco::JSON::Object RetObject;
			State.to_json(RetObject);
			ReturnObject(Request, RetObject, Response);
		} else {
			NotFound(Request, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Configure(Poco::Net::HTTPServerRequest &Request,
											  Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser Parser;
		auto Obj = Parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(RESTAPI::Protocol::UUID) &&
			Obj->has(RESTAPI::Protocol::CONFIGURATION)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto UUID = Obj->get(RESTAPI::Protocol::UUID);
			auto Configuration = GetS(RESTAPI::Protocol::CONFIGURATION, Obj,
									  uCentralProtocol::EMPTY_JSON_DOC);
			auto When = GetWhen(Obj);

			uint64_t NewUUID;

			if (Storage()->UpdateDeviceConfiguration(SerialNumber_, Configuration,
															 NewUUID)) {

				GWObjects::CommandDetails Cmd;

				Cmd.SerialNumber = SerialNumber_;
				Cmd.UUID = Daemon()->CreateUUID();
				Cmd.SubmittedBy = UserInfo_.webtoken.username_;
				Cmd.Command = uCentralProtocol::CONFIGURE;
				Cmd.RunAt = When;

				Config::Config Cfg(Configuration);

				Poco::JSON::Object Params;
				Poco::JSON::Object CfgObj;
				Params.set(uCentralProtocol::SERIAL, SerialNumber_);
				Params.set(uCentralProtocol::UUID, NewUUID);
				Params.set(uCentralProtocol::WHEN, When);
				Params.set(uCentralProtocol::CONFIG, Cfg.to_json());

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				DeviceRegistry()->SetPendingUUID(SerialNumber_, NewUUID);
				RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(5000), nullptr, this);
				return;
			}
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Upgrade(Poco::Net::HTTPServerRequest &Request,
											Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		auto Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(RESTAPI::Protocol::URI) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
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

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetLogs(Poco::Net::HTTPServerRequest &Request,
											Poco::Net::HTTPServerResponse &Response) {
	try {
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
		ReturnObject(Request, RetObj, Response);

		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteLogs(Poco::Net::HTTPServerRequest &Request,
											   Poco::Net::HTTPServerResponse &Response) {
	try {
		if (Storage()->DeleteLogData(SerialNumber_, QB_.StartDate, QB_.EndDate,
											 QB_.LogType)) {
			OK(Request, Response);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetChecks(Poco::Net::HTTPServerRequest &Request,
											  Poco::Net::HTTPServerResponse &Response) {
	try {
		std::vector<GWObjects::HealthCheck> Checks;

		if (QB_.LastOnly) {
			GWObjects::HealthCheck	HC;
			if (DeviceRegistry()->GetHealthcheck(SerialNumber_, HC)) {
				Poco::JSON::Object	Answer;
				HC.to_json(Answer);
				ReturnObject(Request, Answer, Response);
			} else {
				NotFound(Request, Response);
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
			ReturnObject(Request, RetObj, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteChecks(Poco::Net::HTTPServerRequest &Request,
												 Poco::Net::HTTPServerResponse &Response) {
	try {
		if (Storage()->DeleteHealthCheckData(SerialNumber_, QB_.StartDate, QB_.EndDate)) {
			OK(Request, Response);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::ExecuteCommand(Poco::Net::HTTPServerRequest &Request,
												   Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		auto Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(RESTAPI::Protocol::COMMAND) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(RESTAPI::Protocol::PAYLOAD)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
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

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Reboot(Poco::Net::HTTPServerRequest &Request,
										   Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser IncomingParser;
		auto Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {
			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
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

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(2000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Factory(Poco::Net::HTTPServerRequest &Request,
											Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(RESTAPI::Protocol::KEEPREDIRECTOR) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();

			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
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

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::LEDs(Poco::Net::HTTPServerRequest &Request,
										 Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(uCentralProtocol::PATTERN) &&
			Obj->has(RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto Pattern =
				GetS(uCentralProtocol::PATTERN, Obj, uCentralProtocol::BLINK);
			if (Pattern != uCentralProtocol::ON &&
				Pattern != uCentralProtocol::OFF &&
				Pattern != uCentralProtocol::BLINK) {
				Logger_.warning(Poco::format("LEDs(%s): Bad pattern", SerialNumber_));
				BadRequest(Request, Response);
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

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Trace(Poco::Net::HTTPServerRequest &Request,
										  Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if 	(Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			(Obj->has(RESTAPI::Protocol::NETWORK) ||
			 Obj->has(RESTAPI::Protocol::INTERFACE))) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response, "Missing serial number.");
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
			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(5000), nullptr, this);
		} else {
			BadRequest(Request, Response, "Missing SerialNumber, Network, or Interface.");
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::WifiScan(Poco::Net::HTTPServerRequest &Request,
											 Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			BadRequest(Request, Response);
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
				Params.set(uCentralProtocol::BANDS,
						   Obj->get(RESTAPI::Protocol::BANDS));
			} else if (Obj->has(uCentralProtocol::CHANNELS)) {
				Params.set(uCentralProtocol::CHANNELS,
						   Obj->get(RESTAPI::Protocol::CHANNELS));
			}

			if (Obj->has(RESTAPI::Protocol::ACTIVESCAN)) {
				Params.set(
					uCentralProtocol::ACTIVE,
					(int)(Obj->get(RESTAPI::Protocol::ACTIVESCAN).toString() == "true")
						? 1
						: 0);
			} else {
				Params.set(uCentralProtocol::ACTIVE, 0);
			}

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();
			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			if(Cmd.ErrorCode==0) {
				KafkaManager()->PostMessage(KafkaTopics::WIFISCAN, SerialNumber_,
											Cmd.Results);
			}

			return;
		}

	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::EventQueue(Poco::Net::HTTPServerRequest &Request,
											   Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->isArray(RESTAPI::Protocol::TYPES)) {

			auto SNum = Obj->get(RESTAPI::Protocol::SERIALNUMBER).toString();
			auto Types = Obj->getArray(RESTAPI::Protocol::TYPES);

			if (SerialNumber_ == SNum) {
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

				RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
				if(Cmd.ErrorCode==0) {
					KafkaManager()->PostMessage(KafkaTopics::DEVICE_EVENT_QUEUE, SerialNumber_,
												Cmd.Results);
				}
				return;
			}
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::MakeRequest(Poco::Net::HTTPServerRequest &Request,
												Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(uCentralProtocol::MESSAGE)) {

			auto SNum = GetS(RESTAPI::Protocol::SERIALNUMBER, Obj);
			auto MessageType = GetS(uCentralProtocol::MESSAGE, Obj);

			if ((SerialNumber_ != SNum) ||
				(MessageType != uCentralProtocol::STATE &&
				 MessageType != uCentralProtocol::HEALTHCHECK)) {
				BadRequest(Request, Response);
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

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(50000), nullptr, this );
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

	void RESTAPI_device_commandHandler::Rtty(Poco::Net::HTTPServerRequest &Request,
											 Poco::Net::HTTPServerResponse &Response) {
		try {
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
					RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(15000), &ReturnedObject, this);
					return;
				} else {
					NotFound(Request, Response);
					return;
				}
			} else {
				ReturnStatus(Request, Response, Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
				return;
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		BadRequest(Request, Response);
	}

	void RESTAPI_device_commandHandler::Telemetry(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response){
		try {
			Poco::JSON::Parser parser;
			Poco::JSON::Object::Ptr Obj =
				parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

			if (Obj->has(RESTAPI::Protocol::SERIALNUMBER) &&
				Obj->has(RESTAPI::Protocol::INTERVAL) && Obj->has(RESTAPI::Protocol::TYPES)) {

				auto SNum = GetS(RESTAPI::Protocol::SERIALNUMBER, Obj);

				if (SNum != SerialNumber_) {
					BadRequest(Request, Response, "Serial Number mismatch");
					return;
				}

				GWObjects::Device Device;
				if (!Storage()->GetDevice(SerialNumber_, Device)) {
					NotFound(Request, Response);
					return;
				}

				if (!DeviceRegistry()->Connected(SerialNumber_)) {
					BadRequest(Request, Response, "Device is not currently connected.");
					return;
				}

				auto Interval = Obj->get(RESTAPI::Protocol::INTERVAL);
				std::string UUID;
				if (Obj->has(RESTAPI::Protocol::UUID))
					UUID = Obj->get(RESTAPI::Protocol::UUID).toString();

				GWObjects::CommandDetails Cmd;

				Cmd.SerialNumber = SerialNumber_;
				Cmd.SubmittedBy = UserInfo_.webtoken.username_;
				Cmd.UUID = Daemon()->CreateUUID();
				Cmd.Command = uCentralProtocol::REQUEST;

				Poco::JSON::Object Params;

				Params.set(RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
				Params.set(RESTAPI::Protocol::INTERVAL, Interval);
				if (Interval > 0)
					Params.set(RESTAPI::Protocol::TYPES, Obj->getArray(RESTAPI::Protocol::TYPES));

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();
				RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response,
											std::chrono::milliseconds(50000), nullptr, this);

				if (Interval) {
					if (Cmd.ErrorCode == 0) {
						std::string Endpoint, NewUUID;
						if (TelemetryStream()->CreateEndpoint(SerialNumber_, Endpoint, NewUUID)) {
							Poco::JSON::Object Answer;

							Answer.set("serialNumber", SerialNumber_);
							Answer.set("uuid", NewUUID);
							Answer.set("uri", Endpoint);
							ReturnObject(Request, Answer, Response);
							return;
						}
					} else {
						BadRequest(
							Request, Response,
							"Telemetry system could not create WS endpoint. Please try again.");
						return;
					}
				} else {
					TelemetryStream()->DeleteEndPoint(SerialNumber_);
					OK(Request, Response);
				}
				return;
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		BadRequest(Request, Response, "Ill-formed request.");
	}
}
