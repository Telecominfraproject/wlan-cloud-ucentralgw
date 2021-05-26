//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/UUIDGenerator.h"
#include "Poco/JSON/Parser.h"

#include "RESTAPI_device_commandHandler.h"
#include "RESTAPI_objects.h"
#include "uCentral.h"
#include "uCentralConfig.h"
#include "uDeviceRegistry.h"
#include "uFileUploader.h"
#include "uStorageService.h"
#include "uUtils.h"

#include "uCentralProtocol.h"
#include "RESTAPI_protocol.h"

void RESTAPI_device_commandHandler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    try {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        std::string Command = GetBinding(uCentral::RESTAPI::Protocol::COMMAND, "");
        if (Command.empty()) {
            BadRequest(Request, Response);
            return;
        }

        ParseParameters(Request);

        if (Command == uCentral::RESTAPI::Protocol::CAPABILITIES && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetCapabilities(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::CAPABILITIES && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteCapabilities(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::LOGS && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetLogs(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::LOGS && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteLogs(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::HEALTHCHECKS && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetChecks(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::HEALTHCHECKS && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteChecks(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::STATISTICS && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetStatistics(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::STATISTICS && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteStatistics(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::STATUS && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetStatus(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::PERFORM && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            ExecuteCommand(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::CONFIGURE && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Configure(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::UPGRADE && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Upgrade(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::REBOOT && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Reboot(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::FACTORY && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Factory(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::LEDS && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            LEDs(Request, Response);
        } else if (Command == uCentral::RESTAPI::Protocol::TRACE && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Trace(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::REQUEST && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			MakeRequest(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::WIFISCAN && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			WifiScan(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::EVENTQUEUE && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			EventQueue(Request, Response);
		} else if (Command == uCentral::RESTAPI::Protocol::RTTY && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
			Rtty(Request, Response);
		} else {
            BadRequest(Request, Response);
        }
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetCapabilities(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    uCentral::Objects::Capabilities    Caps;
    try {
        auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        if (uCentral::Storage::GetDeviceCapabilities(SerialNumber, Caps)) {
            Poco::JSON::Object RetObj;
			Caps.to_json(RetObj);
            RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBER, SerialNumber);
            ReturnObject(Request, RetObj, Response );
        } else {
			NotFound(Request, Response);
		}
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteCapabilities(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        if (uCentral::Storage::DeleteDeviceCapabilities(SerialNumber))
            OK(Request, Response);
        else
            NotFound(Request, Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetStatistics(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
        auto StartDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::STARTDATE, ""));
        auto EndDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::ENDDATE, ""));
        auto Offset = GetParameter(uCentral::RESTAPI::Protocol::OFFSET, 0);
        auto Limit = GetParameter(uCentral::RESTAPI::Protocol::LIMIT, 100);
		auto Lifetime = GetBoolParameter(uCentral::RESTAPI::Protocol::LIFETIME,false);

		if(Lifetime) {
			std::string Stats;
			uCentral::Storage::GetLifetimeStats(SerialNumber,Stats);
			Poco::JSON::Parser	P;
			if(Stats.empty())
				Stats = uCentral::uCentralProtocol::EMPTY_JSON_DOC;
			auto Obj = P.parse(Stats).extract<Poco::JSON::Object::Ptr>();
			ReturnObject(Request, *Obj, Response);
		} else {
			std::vector<uCentral::Objects::Statistics> Stats;
			uCentral::Storage::GetStatisticsData(SerialNumber, StartDate, EndDate, Offset, Limit,
												 Stats);
			Poco::JSON::Array ArrayObj;
			for (auto i : Stats) {
				Poco::JSON::Object Obj;
				i.to_json(Obj);
				ArrayObj.add(Obj);
			}
			Poco::JSON::Object RetObj;
			RetObj.set(uCentral::RESTAPI::Protocol::DATA, ArrayObj);
			RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBER, SerialNumber);
			ReturnObject(Request, RetObj, Response);
		}

        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteStatistics(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
		auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
		auto StartDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::STARTDATE, ""));
		auto EndDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::ENDDATE, ""));
		auto Lifetime = GetBoolParameter(uCentral::RESTAPI::Protocol::LIFETIME,false);

		if(Lifetime) {
			if(uCentral::Storage::ResetLifetimeStats(SerialNumber)) {
				OK(Request, Response);
			} else {
				NotFound(Request, Response);
			}
		} else {
			if (uCentral::Storage::DeleteStatisticsData(SerialNumber, StartDate, EndDate)) {
				OK(Request, Response);
			} else {
				NotFound(Request, Response);
			}
		}
		return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetStatus(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
		auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
        uCentral::Objects::ConnectionState State;

        if (uCentral::DeviceRegistry::GetState(SerialNumber, State)) {
            Poco::JSON::Object RetObject;
			State.to_json(RetObject);
            ReturnObject(Request, RetObject, Response);
        } else {
			NotFound(Request, Response);
		}
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Configure(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
		auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
        if(SerialNumber.empty())
        {
            BadRequest(Request, Response);
            return;
        }

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
            ds.contains(uCentral::RESTAPI::Protocol::UUID) &&
            ds.contains(uCentral::RESTAPI::Protocol::CONFIGURATION)) {

            auto SNum = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();

            if(SerialNumber != SNum)
            {
                BadRequest(Request, Response);
                return;
            }

            auto UUID = ds[uCentral::RESTAPI::Protocol::UUID];
            auto Configuration = ds[uCentral::RESTAPI::Protocol::CONFIGURATION].toString();
            uint64_t When = 0 ;

            if(ds.contains(uCentral::RESTAPI::Protocol::WHEN))
                When = uCentral::Utils::from_RFC3339(ds[uCentral::RESTAPI::Protocol::WHEN].toString());

            uint64_t NewUUID;

            if (uCentral::Storage::UpdateDeviceConfiguration(SerialNumber, Configuration, NewUUID)) {
                uCentral::Objects::CommandDetails  Cmd;

                Cmd.SerialNumber = SerialNumber;
                Cmd.UUID = uCentral::instance()->CreateUUID();
                Cmd.SubmittedBy = UserInfo_.username_;
                Cmd.Command = uCentral::uCentralProtocol::CONFIGURE;
                Cmd.Custom = 0;
                Cmd.RunAt = When;
                Cmd.WaitingForFile = 0;

                uCentral::Config::Config    Cfg(Configuration);

                Cfg.SetUUID(NewUUID);

                Poco::JSON::Object  Params;
				Poco::JSON::Object	CfgObj;

                Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber );
                Params.set(uCentral::uCentralProtocol::UUID, NewUUID);
                Params.set(uCentral::uCentralProtocol::WHEN, When);
				Cfg.to_json(CfgObj);
                Params.set(uCentral::uCentralProtocol::CONFIG, CfgObj);

                std::stringstream ParamStream;
                Params.stringify(ParamStream);
                Cmd.Details = ParamStream.str();

                if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd,Request, Response);
					return;
                } else {
					ReturnStatus(Request, Response,
								 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				}
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Upgrade(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
		auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains(uCentral::RESTAPI::Protocol::URI) &&
            ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {

            auto SNum = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

            auto URI = ds[uCentral::RESTAPI::Protocol::URI].toString();

            uint64_t When = 0 ;
            if(ds.contains(uCentral::RESTAPI::Protocol::WHEN))
                When = uCentral::Utils::from_RFC3339(ds[uCentral::RESTAPI::Protocol::WHEN].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Custom = 0;
            Cmd.Command = uCentral::uCentralProtocol::UPGRADE;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set( uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set( uCentral::uCentralProtocol::URI, URI);
            Params.set( uCentral::uCentralProtocol::WHEN, When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd,Request, Response, 20000);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetLogs(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
		auto StartDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::STARTDATE, ""));
		auto EndDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::ENDDATE, ""));
		auto Offset = GetParameter(uCentral::RESTAPI::Protocol::OFFSET, 0);
		auto Limit = GetParameter(uCentral::RESTAPI::Protocol::LIMIT, 100);
        auto LogType = GetParameter(uCentral::RESTAPI::Protocol::LOGTYPE,0);

        std::vector<uCentral::Objects::DeviceLog> Logs;

        uCentral::Storage::GetLogData(SerialNumber, StartDate, EndDate, Offset, Limit,
                                                                  Logs,LogType);
        Poco::JSON::Array ArrayObj;

        for (auto i : Logs) {
            Poco::JSON::Object Obj;
			i.to_json(Obj);
            ArrayObj.add(Obj);
        }
        Poco::JSON::Object RetObj;
        RetObj.set(uCentral::RESTAPI::Protocol::VALUES, ArrayObj);
        RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBER, SerialNumber);

        ReturnObject(Request, RetObj, Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteLogs(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
		auto StartDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::STARTDATE, ""));
		auto EndDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::ENDDATE, ""));
		auto LogType = GetParameter(uCentral::RESTAPI::Protocol::LOGTYPE,0);

        if (uCentral::Storage::DeleteLogData(SerialNumber, StartDate, EndDate, LogType)) {
			OK(Request, Response);
			return;
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::GetChecks(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
		auto StartDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::STARTDATE, ""));
		auto EndDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::ENDDATE, ""));
		auto Offset = GetParameter(uCentral::RESTAPI::Protocol::OFFSET, 0);
		auto Limit = GetParameter(uCentral::RESTAPI::Protocol::LIMIT, 100);

        std::vector<uCentral::Objects::HealthCheck> Checks;
        uCentral::Storage::GetHealthCheckData(SerialNumber, StartDate, EndDate, Offset, Limit, Checks);
        Poco::JSON::Array ArrayObj;

        for (auto i : Checks) {
            Poco::JSON::Object Obj;
			i.to_json(Obj);
            ArrayObj.add(Obj);
        }

        Poco::JSON::Object RetObj;
        RetObj.set(uCentral::RESTAPI::Protocol::VALUES, ArrayObj);
        RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBER, SerialNumber);

        ReturnObject(Request, RetObj, Response);

        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::DeleteChecks(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
		auto StartDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::STARTDATE, ""));
		auto EndDate = uCentral::Utils::from_RFC3339(GetParameter(uCentral::RESTAPI::Protocol::ENDDATE, ""));

        if (uCentral::Storage::DeleteHealthCheckData(SerialNumber, StartDate, EndDate)) {
			OK(Request, Response);
			return;
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::ExecuteCommand(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SNum = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains(uCentral::RESTAPI::Protocol::COMMAND) &&
            ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
            ds.contains(uCentral::RESTAPI::Protocol::PAYLOAD)) {

            auto SerialNumber = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

            auto Command = ds[uCentral::RESTAPI::Protocol::COMMAND].toString();
            auto Payload = ds[uCentral::RESTAPI::Protocol::PAYLOAD].toString();

			uint64_t When = ds.contains(uCentral::uCentralProtocol::WHEN) ? uCentral::Utils::from_RFC3339(ds[uCentral::uCentralProtocol::WHEN].toString()) : 0;
            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = Command;
            Cmd.Custom = 1;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Parser parser2;

            Poco::Dynamic::Var result = parser2.parse(Payload);
            const auto & PayloadObject = result.extract<Poco::JSON::Object::Ptr>();

            Poco::JSON::Object  Params;

            Params.set( uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set( uCentral::uCentralProtocol::COMMAND, Command);
            Params.set( uCentral::uCentralProtocol::WHEN, When);
            Params.set( uCentral::uCentralProtocol::PAYLOAD, PayloadObject);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd, Request, Response, 20000);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Reboot(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SNum = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct     ds = *Obj;

        if (ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {
            auto SerialNumber = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

            uint64_t When = 0 ;
            if(ds.contains(uCentral::uCentralProtocol::WHEN))
                When = uCentral::Utils::from_RFC3339(ds[uCentral::uCentralProtocol::WHEN].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = uCentral::uCentralProtocol::REBOOT;
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set( uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set( uCentral::uCentralProtocol::WHEN, When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd, Request, Response, 20000);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Factory(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains(uCentral::RESTAPI::Protocol::KEEPREDIRECTOR) &&
            ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {

			auto SerialNumber = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();

			if (SerialNumber != SNum) {
				BadRequest(Request, Response);
				return;
			}

			auto KeepRedirector = ds[uCentral::RESTAPI::Protocol::KEEPREDIRECTOR].toString();
			uint64_t KeepIt;
			if (KeepRedirector == "true")
				KeepIt = 1;
			else if (KeepRedirector == "false")
				KeepIt = 0;
			else {
				BadRequest(Request, Response);
				return;
			}

			uint64_t When = 0;
			if (ds.contains(uCentral::uCentralProtocol::WHEN))
				When = uCentral::Utils::from_RFC3339(ds[uCentral::uCentralProtocol::WHEN].toString());

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber;
			Cmd.UUID = uCentral::instance()->CreateUUID();
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = uCentral::uCentralProtocol::FACTORY;
			Cmd.Custom = 0;
			Cmd.RunAt = When;
			Cmd.WaitingForFile = 0;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber);
			Params.set(uCentral::uCentralProtocol::KEEP_REDIRECTOR, KeepIt);
			Params.set(uCentral::uCentralProtocol::WHEN, When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			if (uCentral::Storage::AddCommand(SerialNumber, Cmd)) {
				WaitForRPC(Cmd, Request, Response, 20000);
				return;
			} else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::LEDs(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains(uCentral::uCentralProtocol::PATTERN) &&
            ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {

            auto SerialNumber = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

			auto Pattern = ds[uCentral::uCentralProtocol::PATTERN].toString();

			if(Pattern!=uCentral::uCentralProtocol::ON && Pattern!=uCentral::uCentralProtocol::OFF && Pattern!=uCentral::uCentralProtocol::BLINK)
			{
				Logger_.warning(Poco::format("LEDs(%s): Bad pattern",SerialNumber));
				BadRequest(Request, Response);
				return;
			}

			auto Duration = ds.contains(uCentral::uCentralProtocol::DURATION) ? (uint64_t ) ds[uCentral::uCentralProtocol::DURATION] : 20 ;

            uint64_t When = 0 ;
            if(ds.contains(uCentral::uCentralProtocol::WHEN))
                When = uCentral::Utils::from_RFC3339(ds[uCentral::uCentralProtocol::WHEN].toString());

			Logger_.information(Poco::format("LEDS(%s): Pattern:%s Duration: %d", SerialNumber, Pattern, (int)Duration));

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = uCentral::uCentralProtocol::LEDS;
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set(uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set(uCentral::uCentralProtocol::DURATION, Duration);
            Params.set(uCentral::uCentralProtocol::WHEN, When);
			Params.set(uCentral::uCentralProtocol::PATTERN,Pattern);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd, Request, Response, 20000);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Trace(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER) && (
            	ds.contains(uCentral::RESTAPI::Protocol::NETWORK) ||
            	ds.contains(uCentral::RESTAPI::Protocol::INTERFACE))) {

            auto SerialNumber = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();

            if(SerialNumber != SNum) {
                BadRequest(Request, Response);
                return;
            }

            uint64_t Duration = ds.contains(uCentral::RESTAPI::Protocol::DURATION) ? (uint64_t)ds[uCentral::RESTAPI::Protocol::DURATION] : 0;
            uint64_t When = ds.contains(uCentral::RESTAPI::Protocol::WHEN) ? uCentral::Utils::from_RFC3339(ds[uCentral::RESTAPI::Protocol::WHEN].toString()) : 0;
            uint64_t NumberOfPackets = ds.contains(uCentral::RESTAPI::Protocol::NUMBEROFPACKETS) ? (uint64_t)ds[uCentral::RESTAPI::Protocol::NUMBEROFPACKETS] : 0;

            auto Network = ds.contains(uCentral::RESTAPI::Protocol::NETWORK) ? ds[uCentral::RESTAPI::Protocol::NETWORK].toString() : "";
            auto Interface = ds.contains(uCentral::RESTAPI::Protocol::INTERFACE) ? ds[uCentral::RESTAPI::Protocol::INTERFACE].toString() : "";
            auto UUID = uCentral::instance()->CreateUUID();
            auto URI = uCentral::uFileUploader::FullName() + UUID ;

            uCentral::Objects::CommandDetails  Cmd;
            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = UUID;
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = uCentral::uCentralProtocol::TRACE;
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 1;
			Cmd.AttachType = uCentral::RESTAPI::Protocol::PCAP_FILE_TYPE;

            Poco::JSON::Object  Params;

            Params.set(uCentral::uCentralProtocol::SERIAL , SerialNumber );
            Params.set(uCentral::uCentralProtocol::DURATION, Duration);
            Params.set(uCentral::uCentralProtocol::WHEN, When);
            Params.set(uCentral::uCentralProtocol::PACKETS, NumberOfPackets);
            Params.set(uCentral::uCentralProtocol::NETWORK, Network);
            Params.set(uCentral::uCentralProtocol::INTERFACE, Interface);
            Params.set(uCentral::uCentralProtocol::URI,URI);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
                uCentral::uFileUploader::AddUUID(UUID);
				Poco::JSON::Object RetObj;
				Cmd.to_json(RetObj);
				ReturnObject(Request, RetObj, Response);
				return;
            } else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::WifiScan(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try{
		auto SNum = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if(ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER)) {
			auto SerialNumber = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();
			if(	(ds.contains(uCentral::RESTAPI::Protocol::BANDS) && ds[uCentral::RESTAPI::Protocol::BANDS].isArray()) ||
				(ds.contains(uCentral::RESTAPI::Protocol::CHANNELS) && ds[uCentral::RESTAPI::Protocol::CHANNELS].isArray()) ||
				(!ds.contains(uCentral::RESTAPI::Protocol::BANDS) && !ds.contains(uCentral::RESTAPI::Protocol::CHANNELS))
				)
			{
				bool Verbose = false ;

				if(ds.contains(uCentral::RESTAPI::Protocol::VERBOSE))
				{
					Verbose = ds[uCentral::RESTAPI::Protocol::VERBOSE].toString() == "true";
				}

				auto UUID = uCentral::instance()->CreateUUID();
				uCentral::Objects::CommandDetails  Cmd;

				Cmd.SerialNumber = SerialNumber;
				Cmd.UUID = UUID;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.Command = uCentral::uCentralProtocol::WIFISCAN;
				Cmd.Custom = 0;
				Cmd.RunAt = 0;
				Cmd.WaitingForFile = 0;

				Poco::JSON::Object  Params;

				Params.set(uCentral::uCentralProtocol::SERIAL , SerialNumber );
				Params.set(uCentral::uCentralProtocol::VERBOSE, Verbose);

				if( ds.contains(uCentral::uCentralProtocol::BANDS)) {
					Params.set(uCentral::uCentralProtocol::BANDS,ds[uCentral::RESTAPI::Protocol::BANDS]);
				} else if ( ds.contains(uCentral::uCentralProtocol::CHANNELS)) {
					Params.set(uCentral::uCentralProtocol::CHANNELS,ds[uCentral::RESTAPI::Protocol::CHANNELS]);
				}

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd, Request, Response, 20000);
					return;
				} else {
					ReturnStatus(Request, Response,
								 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				}
			}
		}
	} catch (const Poco::Exception & E) {
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::EventQueue(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		auto SNum = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if(ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
			ds.contains(uCentral::RESTAPI::Protocol::TYPES) && ds[uCentral::RESTAPI::Protocol::TYPES].isArray())
		{
			auto SerialNumber = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();
			auto Types = ds[uCentral::RESTAPI::Protocol::TYPES];

			if( SerialNumber == SNum ) {
				auto UUID = uCentral::instance()->CreateUUID();
				uCentral::Objects::CommandDetails  Cmd;

				Cmd.SerialNumber = SerialNumber;
				Cmd.UUID = UUID;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.Command = uCentral::uCentralProtocol::EVENT;
				Cmd.Custom = 0;
				Cmd.RunAt = 0;
				Cmd.WaitingForFile = 0;

				Poco::JSON::Object  Params;

				Params.set(uCentral::uCentralProtocol::SERIAL , SerialNumber );
				Params.set(uCentral::uCentralProtocol::TYPES, Types);

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd, Request, Response, 20000);
					return;
				} else {
					ReturnStatus(Request, Response,
								 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				}
			}
		}
	} catch ( const Poco::Exception & E ) {
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::MakeRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		auto SNum = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if (ds.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
			ds.contains(uCentral::uCentralProtocol::MESSAGE)) {

			auto SerialNumber = ds[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();
			auto MessageType = ds[uCentral::uCentralProtocol::MESSAGE].toString();

			if ((SerialNumber != SNum) ||
				(MessageType != uCentral::uCentralProtocol::STATE && MessageType != uCentral::uCentralProtocol::HEALTHCHECK)) {
				BadRequest(Request, Response);
				return;
			}

			uint64_t When =
				ds.contains(uCentral::uCentralProtocol::WHEN) ? uCentral::Utils::from_RFC3339(ds[uCentral::uCentralProtocol::WHEN].toString()) : 0;

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber;
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.UUID = uCentral::instance()->CreateUUID();
			Cmd.Command = uCentral::uCentralProtocol::REQUEST;
			Cmd.Custom = 0;
			Cmd.RunAt = When;
			Cmd.WaitingForFile = 0;

			Poco::JSON::Object Params;

			Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber);
			Params.set(uCentral::uCentralProtocol::WHEN, When);
			Params.set(uCentral::uCentralProtocol::MESSAGE, MessageType);
			Params.set(uCentral::uCentralProtocol::REQUEST_UUID, Cmd.UUID);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			if (uCentral::Storage::AddCommand(SerialNumber, Cmd)) {
				WaitForRPC(Cmd, Request, Response, 4000);
				return;
			} else {
				ReturnStatus(Request, Response,
							 Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				return;
			}
		}
	}
	catch(const Poco::Exception &E)
	{
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}

void RESTAPI_device_commandHandler::Rtty(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {

		auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		if(uCentral::ServiceConfig::GetString("rtty.enabled","false") == "true") {

			if (uCentral::Storage::DeviceExists(SerialNumber)) {
				auto CommandUUID = uCentral::Daemon::instance()->CreateUUID();
				uCentral::Objects::RttySessionDetails Rtty{
					.SerialNumber = SerialNumber,
					.Server = uCentral::ServiceConfig::GetString("rtty.server", "localhost") ,
					.Port = uCentral::ServiceConfig::GetInt("rtty.port",5912),
					.Token = uCentral::ServiceConfig::GetString("rtty.token","nothing"),
					.TimeOut = uCentral::ServiceConfig::GetInt("rtty.timeout",60),
					.ConnectionId = CommandUUID,
					.Started = (uint64_t) time(nullptr),
					.CommandUUID = CommandUUID,
					.ViewPort = uCentral::ServiceConfig::GetInt("rtty.viewport",5913),
					};

				Poco::JSON::Object	ReturnedObject;
				Rtty.to_json(ReturnedObject);

				//	let's create the command for this request
				uCentral::Objects::CommandDetails	Cmd;
				Cmd.SerialNumber = SerialNumber;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.UUID = CommandUUID;
				Cmd.Command = uCentral::uCentralProtocol::RTTY;
				Cmd.Custom = 0;
				Cmd.RunAt = 0;
				Cmd.WaitingForFile = 0;

				Poco::JSON::Object  Params;

				Params.set(uCentral::uCentralProtocol::METHOD,uCentral::uCentralProtocol::RTTY);
				Params.set(uCentral::uCentralProtocol::SERIAL, SerialNumber);
				Params.set(uCentral::uCentralProtocol::ID, Rtty.ConnectionId);
				Params.set(uCentral::uCentralProtocol::TOKEN,Rtty.Token);
				Params.set(uCentral::uCentralProtocol::SERVER, Rtty.Server);
				Params.set(uCentral::uCentralProtocol::PORT, Rtty.Port);
				Params.set(uCentral::uCentralProtocol::USER, UserInfo_.username_);
				Params.set(uCentral::uCentralProtocol::TIMEOUT, Rtty.TimeOut);

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					if(WaitForRPC(Cmd, Request, Response, 10000, false))
						ReturnObject(Request, ReturnedObject, Response);
					else
						ReturnStatus(Request, Response,Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				} else {
					ReturnStatus(Request, Response, Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
					return;
				}
			} else {
				NotFound(Request, Response);
				return;
			}
		} else {
			ReturnStatus(Request, Response, Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Request, Response);
}
