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
#include "RESTAPI_device_commandHandler.h"
#include "RESTAPI_objects.h"
#include "StorageService.h"
#include "Utils.h"

#include "uCentralProtocol.h"
#include "RESTAPI_protocol.h"
#include "RESTAPI_RPC.h"

#include "CommandManager.h"
#include "KafkaManager.h"
#include "Kafka_topics.h"

namespace uCentral {

void RESTAPI_device_commandHandler::handleRequest(Poco::Net::HTTPServerRequest &Request,
												  Poco::Net::HTTPServerResponse &Response) {
	try {
		if (!ContinueProcessing(Request, Response))
			return;

		if (!IsAuthorized(Request, Response))
			return;

		std::string Command = GetBinding(uCentral::RESTAPI::Protocol::COMMAND, "");
		if (Command.empty()) {
			Logger_.error(Poco::format("Unrecognized command '%s'", Command));
			BadRequest(Request, Response);
			return;
		}

		SerialNumber_ = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
		if (SerialNumber_.empty()) {
			Logger_.error(Poco::format("Missing serial number for command '%s'", Command));
			BadRequest(Request, Response);
			return;
		}

		ParseParameters(Request);
		InitQueryBlock();

		if (Command == uCentral::RESTAPI::Protocol::CAPABILITIES &&
			Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetCapabilities(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::CAPABILITIES &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
			DeleteCapabilities(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::LOGS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetLogs(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::LOGS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
			DeleteLogs(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::HEALTHCHECKS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetChecks(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::HEALTHCHECKS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
			DeleteChecks(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::STATISTICS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetStatistics(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::STATISTICS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
			DeleteStatistics(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::STATUS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			GetStatus(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::PERFORM &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			ExecuteCommand(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::CONFIGURE &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Configure(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::UPGRADE &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Upgrade(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::REBOOT &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Reboot(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::FACTORY &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Factory(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::LEDS &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			LEDs(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::TRACE &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			Trace(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::REQUEST &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			MakeRequest(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::WIFISCAN &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			WifiScan(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::EVENTQUEUE &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			EventQueue(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::RTTY &&
				   Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			Rtty(Request, Response);
		} else {
			BadRequest(Request, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetCapabilities(Poco::Net::HTTPServerRequest &Request,
													Poco::Net::HTTPServerResponse &Response) {
	uCentral::Objects::Capabilities Caps;
	try {
		if (Storage()->GetDeviceCapabilities(SerialNumber_, Caps)) {
			Poco::JSON::Object RetObj;
			Caps.to_json(RetObj);
			RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
			ReturnObject(Request, RetObj, Response);
		} else {
			NotFound(Request, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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
				Stats = uCentral::uCentralProtocol::EMPTY_JSON_DOC;
			auto Obj = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
			ReturnObject(Request, *Obj, Response);
		} else if (QB_.LastOnly) {
			std::string Stats;
			if (DeviceRegistry()->GetStatistics(SerialNumber_, Stats)) {
				Poco::JSON::Parser P;
				if (Stats.empty())
					Stats = uCentral::uCentralProtocol::EMPTY_JSON_DOC;
				auto Obj = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
				ReturnObject(Request, *Obj, Response);
			} else {
				NotFound(Request, Response);
			}
		} else {
			std::vector<uCentral::Objects::Statistics> Stats;
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
			RetObj.set(uCentral::RESTAPI::Protocol::DATA, ArrayObj);
			RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
			ReturnObject(Request, RetObj, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetStatus(Poco::Net::HTTPServerRequest &Request,
											  Poco::Net::HTTPServerResponse &Response) {
	try {
		uCentral::Objects::ConnectionState State;

		if (DeviceRegistry()->GetState(SerialNumber_, State)) {
			Poco::JSON::Object RetObject;
			State.to_json(RetObject);
			ReturnObject(Request, RetObject, Response);
		} else {
			NotFound(Request, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Configure(Poco::Net::HTTPServerRequest &Request,
											  Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser Parser;
		auto Obj = Parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(uCentral::RESTAPI::Protocol::UUID) &&
			Obj->has(uCentral::RESTAPI::Protocol::CONFIGURATION)) {

			auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto UUID = Obj->get(uCentral::RESTAPI::Protocol::UUID);
			auto Configuration = GetS(uCentral::RESTAPI::Protocol::CONFIGURATION, Obj,
									  uCentral::uCentralProtocol::EMPTY_JSON_DOC);
			auto When = GetWhen(Obj);

			uint64_t NewUUID;

			if (Storage()->UpdateDeviceConfiguration(SerialNumber_, Configuration,
															 NewUUID)) {

				uCentral::Objects::CommandDetails Cmd;

				Cmd.SerialNumber = SerialNumber_;
				Cmd.UUID = Daemon()->CreateUUID();
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.Command = uCentral::uCentralProtocol::CONFIGURE;
				Cmd.RunAt = When;

				uCentral::Config::Config Cfg(Configuration);

				Poco::JSON::Object Params;
				Poco::JSON::Object CfgObj;
				Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
				Params.set(uCentral::uCentralProtocol::UUID, NewUUID);
				Params.set(uCentral::uCentralProtocol::WHEN, When);
				Params.set(uCentral::uCentralProtocol::CONFIG, Cfg.to_json());

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				DeviceRegistry()->SetPendingUUID(SerialNumber_, NewUUID);
				RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(5000), nullptr, this);
				return;
			}
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Upgrade(Poco::Net::HTTPServerRequest &Request,
											Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		auto Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(uCentral::RESTAPI::Protocol::URI) &&
			Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto URI = GetS(uCentral::RESTAPI::Protocol::URI, Obj);
			auto When = GetWhen(Obj);

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = Daemon()->CreateUUID();
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = uCentral::uCentralProtocol::UPGRADE;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentral::uCentralProtocol::URI, URI);
			Params.set(uCentral::uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetLogs(Poco::Net::HTTPServerRequest &Request,
											Poco::Net::HTTPServerResponse &Response) {
	try {
		std::vector<uCentral::Objects::DeviceLog> Logs;

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
		RetObj.set(uCentral::RESTAPI::Protocol::VALUES, ArrayObj);
		RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
		ReturnObject(Request, RetObj, Response);

		return;
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetChecks(Poco::Net::HTTPServerRequest &Request,
											  Poco::Net::HTTPServerResponse &Response) {
	try {
		std::vector<uCentral::Objects::HealthCheck> Checks;

		if (QB_.LastOnly) {
			std::string Healthcheck;
			if (DeviceRegistry()->GetHealthcheck(SerialNumber_, Healthcheck)) {
				Poco::JSON::Parser P;
				if (Healthcheck.empty())
					Healthcheck = uCentral::uCentralProtocol::EMPTY_JSON_DOC;
				auto Obj = P.parse(Healthcheck).extract<Poco::JSON::Object::Ptr>();
				ReturnObject(Request, *Obj, Response);
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
			RetObj.set(uCentral::RESTAPI::Protocol::VALUES, ArrayObj);
			RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBER, SerialNumber_);
			ReturnObject(Request, RetObj, Response);
		}
		return;
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::ExecuteCommand(Poco::Net::HTTPServerRequest &Request,
												   Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		auto Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(uCentral::RESTAPI::Protocol::COMMAND) &&
			Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(uCentral::RESTAPI::Protocol::PAYLOAD)) {

			auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto Command = GetS(uCentral::RESTAPI::Protocol::COMMAND, Obj);
			auto Payload = GetS(uCentral::RESTAPI::Protocol::PAYLOAD, Obj);
			auto When = GetWhen(Obj);

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = Daemon()->CreateUUID();
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = Command;
			Cmd.Custom = 1;
			Cmd.RunAt = When;

			Poco::JSON::Parser parser2;

			Poco::Dynamic::Var result = parser2.parse(Payload);
			const auto &PayloadObject = result.extract<Poco::JSON::Object::Ptr>();

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentral::uCentralProtocol::COMMAND, Command);
			Params.set(uCentral::uCentralProtocol::WHEN, When);
			Params.set(uCentral::uCentralProtocol::PAYLOAD, PayloadObject);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Reboot(Poco::Net::HTTPServerRequest &Request,
										   Poco::Net::HTTPServerResponse &Response) {
	try {
		//  get the configuration from the body of the message
		Poco::JSON::Parser IncomingParser;
		auto Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {
			auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			uint64_t When = GetWhen(Obj);
			uCentral::Objects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = Daemon()->CreateUUID();
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = uCentral::uCentralProtocol::REBOOT;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentral::uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(2000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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

		if (Obj->has(uCentral::RESTAPI::Protocol::KEEPREDIRECTOR) &&
			Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();

			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto KeepRedirector = GetB(uCentral::RESTAPI::Protocol::KEEPREDIRECTOR, Obj, true);
			uint64_t When = GetWhen(Obj);

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = Daemon()->CreateUUID();
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = uCentral::uCentralProtocol::FACTORY;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentral::uCentralProtocol::KEEP_REDIRECTOR, KeepRedirector ? 1 : 0);
			Params.set(uCentral::uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::LEDs(Poco::Net::HTTPServerRequest &Request,
										 Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(uCentral::uCentralProtocol::PATTERN) &&
			Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {

			auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto Pattern =
				GetS(uCentral::uCentralProtocol::PATTERN, Obj, uCentral::uCentralProtocol::BLINK);
			if (Pattern != uCentral::uCentralProtocol::ON &&
				Pattern != uCentral::uCentralProtocol::OFF &&
				Pattern != uCentral::uCentralProtocol::BLINK) {
				Logger_.warning(Poco::format("LEDs(%s): Bad pattern", SerialNumber_));
				BadRequest(Request, Response);
				return;
			}

			auto Duration = Get(uCentral::uCentralProtocol::DURATION, Obj, 30);
			auto When = GetWhen(Obj);
			Logger_.information(Poco::format("LEDS(%s): Pattern:%s Duration: %Lu", SerialNumber_,
											 Pattern, Duration));

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = Daemon()->CreateUUID();
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = uCentral::uCentralProtocol::LEDS;
			Cmd.RunAt = When;
			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentral::uCentralProtocol::DURATION, Duration);
			Params.set(uCentral::uCentralProtocol::WHEN, When);
			Params.set(uCentral::uCentralProtocol::PATTERN, Pattern);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Trace(Poco::Net::HTTPServerRequest &Request,
										  Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		if (Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
			(Obj->has(uCentral::RESTAPI::Protocol::NETWORK) ||
			 Obj->has(uCentral::RESTAPI::Protocol::INTERFACE))) {

			auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();
			if (SerialNumber_ != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto Duration = Get(uCentral::RESTAPI::Protocol::DURATION, Obj);
			auto When = GetWhen(Obj);
			auto NumberOfPackets = Get(uCentral::RESTAPI::Protocol::NUMBEROFPACKETS, Obj);

			auto Network = GetS(uCentral::RESTAPI::Protocol::NETWORK, Obj);
			auto Interface = GetS(uCentral::RESTAPI::Protocol::INTERFACE, Obj);
			auto UUID = Daemon()->CreateUUID();
			auto URI = FileUploader()->FullName() + UUID;

			uCentral::Objects::CommandDetails Cmd;
			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = UUID;
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = uCentral::uCentralProtocol::TRACE;
			Cmd.RunAt = When;
			Cmd.WaitingForFile = 1;
			Cmd.AttachType = uCentral::RESTAPI::Protocol::PCAP_FILE_TYPE;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentral::uCentralProtocol::DURATION, Duration);
			Params.set(uCentral::uCentralProtocol::WHEN, When);
			Params.set(uCentral::uCentralProtocol::PACKETS, NumberOfPackets);
			Params.set(uCentral::uCentralProtocol::NETWORK, Network);
			Params.set(uCentral::uCentralProtocol::INTERFACE, Interface);
			Params.set(uCentral::uCentralProtocol::URI, URI);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			FileUploader()->AddUUID(UUID);
			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(3000), nullptr, this);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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

		auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();
		if (SerialNumber_ != SNum) {
			BadRequest(Request, Response);
			return;
		}

		if ((Obj->has(uCentral::RESTAPI::Protocol::BANDS) &&
				 Obj->isArray(uCentral::RESTAPI::Protocol::BANDS) ||
			 (Obj->has(uCentral::RESTAPI::Protocol::CHANNELS) &&
			  Obj->isArray(uCentral::RESTAPI::Protocol::CHANNELS)) ||
			 (!Obj->has(uCentral::RESTAPI::Protocol::BANDS) &&
			  !Obj->has(uCentral::RESTAPI::Protocol::CHANNELS)))) {
			bool Verbose = GetB(uCentral::RESTAPI::Protocol::VERBOSE, Obj);
			auto UUID = Daemon()->CreateUUID();
			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.UUID = UUID;
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = uCentral::uCentralProtocol::WIFISCAN;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentral::uCentralProtocol::VERBOSE, Verbose);

			if (Obj->has(uCentral::uCentralProtocol::BANDS)) {
				Params.set(uCentral::uCentralProtocol::BANDS,
						   Obj->get(uCentral::RESTAPI::Protocol::BANDS));
			} else if (Obj->has(uCentral::uCentralProtocol::CHANNELS)) {
				Params.set(uCentral::uCentralProtocol::CHANNELS,
						   Obj->get(uCentral::RESTAPI::Protocol::CHANNELS));
			}

			if (Obj->has(uCentral::RESTAPI::Protocol::ACTIVESCAN)) {
				Params.set(
					uCentral::uCentralProtocol::ACTIVE,
					(int)(Obj->get(uCentral::RESTAPI::Protocol::ACTIVESCAN).toString() == "true")
						? 1
						: 0);
			} else {
				Params.set(uCentral::uCentralProtocol::ACTIVE, 0);
			}

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();
			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
			KafkaManager()->PostMessage(uCentral::KafkaTopics::WIFISCAN, SerialNumber_,
										 Cmd.Results);

			return;
		}

	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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

		if (Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(uCentral::RESTAPI::Protocol::TYPES) &&
			Obj->isArray(uCentral::RESTAPI::Protocol::TYPES)) {
			auto SNum = Obj->get(uCentral::RESTAPI::Protocol::SERIALNUMBER).toString();
			auto Types = Obj->getArray(uCentral::RESTAPI::Protocol::TYPES);

			if (SerialNumber_ == SNum) {
				auto UUID = Daemon()->CreateUUID();
				uCentral::Objects::CommandDetails Cmd;

				Cmd.SerialNumber = SerialNumber_;
				Cmd.UUID = UUID;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.Command = uCentral::uCentralProtocol::EVENT;

				Poco::JSON::Object Params;

				Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
				Params.set(uCentral::uCentralProtocol::TYPES, Types);

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(20000), nullptr, this);
				return;
			}
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
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

		if (Obj->has(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
			Obj->has(uCentral::uCentralProtocol::MESSAGE)) {

			auto SNum = GetS(uCentral::RESTAPI::Protocol::SERIALNUMBER, Obj);
			auto MessageType = GetS(uCentral::uCentralProtocol::MESSAGE, Obj);

			if ((SerialNumber_ != SNum) ||
				(MessageType != uCentral::uCentralProtocol::STATE &&
				 MessageType != uCentral::uCentralProtocol::HEALTHCHECK)) {
				BadRequest(Request, Response);
				return;
			}

			auto When = GetWhen(Obj);
			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber_;
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.UUID = Daemon()->CreateUUID();
			Cmd.Command = uCentral::uCentralProtocol::REQUEST;
			Cmd.RunAt = When;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
			Params.set(uCentral::uCentralProtocol::WHEN, When);
			Params.set(uCentral::uCentralProtocol::MESSAGE, MessageType);
			Params.set(uCentral::uCentralProtocol::REQUEST_UUID, Cmd.UUID);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			RESTAPI_RPC::WaitForCommand(Cmd, Params, Request, Response, std::chrono::milliseconds(50000), nullptr, this );
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Rtty(Poco::Net::HTTPServerRequest &Request,
										 Poco::Net::HTTPServerResponse &Response) {
	try {
		if (Daemon()->ConfigGetString("rtty.enabled", "false") == "true") {
			Objects::Device	Device;
			if (Storage()->GetDevice(SerialNumber_, Device)) {
				auto CommandUUID = uCentral::Daemon::instance()->CreateUUID();

				uCentral::Objects::RttySessionDetails Rtty{
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
				uCentral::Objects::CommandDetails Cmd;
				Cmd.SerialNumber = SerialNumber_;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.UUID = CommandUUID;
				Cmd.Command = uCentral::uCentralProtocol::RTTY;

				Poco::JSON::Object Params;

				Params.set(uCentral::uCentralProtocol::METHOD, uCentral::uCentralProtocol::RTTY);
				Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber_);
				Params.set(uCentral::uCentralProtocol::ID, Rtty.ConnectionId);
				Params.set(uCentral::uCentralProtocol::TOKEN, Rtty.Token);
				Params.set(uCentral::uCentralProtocol::SERVER, Rtty.Server);
				Params.set(uCentral::uCentralProtocol::PORT, Rtty.Port);
				Params.set(uCentral::uCentralProtocol::USER, UserInfo_.username_);
				Params.set(uCentral::uCentralProtocol::TIMEOUT, Rtty.TimeOut);
				Params.set(uCentral::uCentralProtocol::PASSWORD, Device.DevicePassword);

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
		Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

}
