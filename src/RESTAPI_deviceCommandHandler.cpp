//
// Created by stephane bourque on 2021-03-04.
//

#include "Poco/UUIDGenerator.h"
#include "Poco/UUID.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Thread.h"

#include "uStorageService.h"
#include "uDeviceRegistry.h"
#include "uCentral.h"
#include "uCentralConfig.h"
#include "uFileUploader.h"
#include "RESTAPI_deviceCommandHandler.h"
#include "RESTAPI_objects.h"

void RESTAPI_deviceCommandHandler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    try {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        std::string Command = GetBinding("command", "");

        if (Command.empty()) {
            BadRequest(Response);
            return;
        }

        ParseParameters(Request);

        if (Command == "capabilities" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetCapabilities(Request, Response);
        } else if (Command == "capabilities" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteCapabilities(Request, Response);
        } else if (Command == "logs" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetLogs(Request, Response);
        } else if (Command == "logs" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteLogs(Request, Response);
        } else if (Command == "healthchecks" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetChecks(Request, Response);
        } else if (Command == "healthchecks" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteChecks(Request, Response);
        } else if (Command == "statistics" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetStatistics(Request, Response);
        } else if (Command == "statistics" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteStatistics(Request, Response);
        } else if (Command == "status" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetStatus(Request, Response);
        } else if (Command == "perform" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            ExecuteCommand(Request, Response);
        } else if (Command == "configure" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Configure(Request, Response);
        } else if (Command == "upgrade" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Upgrade(Request, Response);
        } else if (Command == "reboot" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Reboot(Request, Response);
        } else if (Command == "factory" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Factory(Request, Response);
        } else if (Command == "leds" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            LEDs(Request, Response);
        } else if (Command == "trace" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Trace(Request, Response);
		} else if (Command == "request" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			MakeRequest(Request, Response);
		} else if (Command == "wifiscan" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			WifiScan(Request, Response);
		} else if (Command == "eventqueue" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
			EventQueue(Request, Response);
		} else {
            BadRequest(Response);
        }
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Response);
}

void  RESTAPI_deviceCommandHandler::GetCapabilities(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    uCentral::Objects::Capabilities    Caps;
    try {
        auto SerialNumber = GetBinding("serialNumber", "");

        if (uCentral::Storage::GetDeviceCapabilities(SerialNumber, Caps)) {
            Poco::JSON::Object RetObj;
			Caps.to_json(RetObj);
            RetObj.set("serialNumber", SerialNumber);
            ReturnObject( RetObj, Response );
        } else
            NotFound(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void  RESTAPI_deviceCommandHandler::DeleteCapabilities(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");

        if (uCentral::Storage::DeleteDeviceCapabilities(SerialNumber))
            OK(Response);
        else
            NotFound(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetStatistics(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);

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

        RetObj.set("data", ArrayObj);
        RetObj.set("serialNumber", SerialNumber);

        ReturnObject(RetObj, Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::DeleteStatistics(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));

        if (uCentral::Storage::DeleteStatisticsData(SerialNumber, StartDate, EndDate)) {
			OK(Response);
			return;
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetStatus(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");

        uCentral::Objects::ConnectionState State;

        if (uCentral::DeviceRegistry::GetState(SerialNumber, State)) {

            Poco::JSON::Object RetObject;
			State.to_json(RetObject);

            ReturnObject(RetObject, Response);

        } else
            NotFound(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Configure(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        if(SNum.empty())
        {
            BadRequest(Response);
            return;
        }

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("serialNumber") &&
            ds.contains("UUID") &&
            ds.contains("configuration")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum)
            {
                BadRequest(Response);
                return;
            }

            auto UUID = ds["UUID"];
            auto Configuration = ds["configuration"].toString();
            uint64_t When = 0 ;

            if(ds.contains("when"))
                When = RESTAPIHandler::from_RFC3339(ds["when"].toString());

            uint64_t NewUUID;

            if (uCentral::Storage::UpdateDeviceConfiguration(SerialNumber, Configuration, NewUUID)) {
                uCentral::Objects::CommandDetails  Cmd;

                Cmd.SerialNumber = SerialNumber;
                Cmd.UUID = uCentral::instance()->CreateUUID();
                Cmd.SubmittedBy = UserInfo_.username_;
                Cmd.Command = "configure";
                Cmd.Custom = 0;
                Cmd.RunAt = When;
                Cmd.WaitingForFile = 0;

                uCentral::Config::Config    Cfg(Configuration);

                Cfg.SetUUID(NewUUID);

                Poco::JSON::Object  Params;
				Poco::JSON::Object	CfgObj;

                Params.set("serial", SerialNumber );
                Params.set("uuid", NewUUID);
                Params.set("when", When);
				Cfg.to_json(CfgObj);
                Params.set("config", CfgObj);

                std::stringstream ParamStream;
                Params.stringify(ParamStream);
                Cmd.Details = ParamStream.str();

                if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd,Response);
					return;
                }
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Upgrade(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("uri") &&
            ds.contains("serialNumber")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Response);
                return;
            }

            auto URI = ds["uri"].toString();

            uint64_t When = 0 ;
            if(ds.contains("when"))
                When = RESTAPIHandler::from_RFC3339(ds["when"].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Custom = 0;
            Cmd.Command = "upgrade";
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set( "serial" , SerialNumber );
            Params.set( "uri", URI);
            Params.set( "when", When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd,Response, 20000);
				return;
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetLogs(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);
        auto LogType = GetParameter("logType",0);

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
        RetObj.set("values", ArrayObj);
        RetObj.set("serialNumber", SerialNumber);

        ReturnObject(RetObj, Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::DeleteLogs(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));
        auto LogType = GetParameter("logType",0);

        if (uCentral::Storage::DeleteLogData(SerialNumber, StartDate, EndDate, LogType)) {
			OK(Response);
			return;
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetChecks(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);

        std::vector<uCentral::Objects::HealthCheck> Checks;

        uCentral::Storage::GetHealthCheckData(SerialNumber, StartDate, EndDate, Offset, Limit,
                                      Checks);
        Poco::JSON::Array ArrayObj;

        for (auto i : Checks) {
            Poco::JSON::Object Obj;
			i.to_json(Obj);
            ArrayObj.add(Obj);
        }
        Poco::JSON::Object RetObj;
        RetObj.set("values", ArrayObj);
        RetObj.set("serialNumber", SerialNumber);

        ReturnObject(RetObj, Response);

        return;
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::DeleteChecks(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));

        if (uCentral::Storage::DeleteHealthCheckData(SerialNumber, StartDate, EndDate)) {
			OK(Response);
			return;
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::ExecuteCommand(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("command") &&
            ds.contains("serialNumber") &&
            ds.contains("payload")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Response);
                return;
            }

            auto Command = ds["command"].toString();
            auto Payload = ds["payload"].toString();

            uint64_t RunAt = 0 ;
            if(ds.contains("runAt"))
                RunAt = RESTAPIHandler::from_RFC3339(ds["runAt"].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = Command;
            Cmd.Custom = 1;
            Cmd.RunAt = RunAt;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Parser parser2;

            Poco::Dynamic::Var result = parser2.parse(Payload);
            const auto & PayloadObject = result.extract<Poco::JSON::Object::Ptr>();

            Poco::JSON::Object  Params;

            Params.set( "serial" , SerialNumber );
            Params.set( "command", Command);
            Params.set( "when", RunAt);
            Params.set( "payload", PayloadObject);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd,Response, 20000);
				return;
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Reboot(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser      IncomingParser;
        Poco::JSON::Object::Ptr Obj = IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct     ds = *Obj;

        if (ds.contains("serialNumber")) {
            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Response);
                return;
            }

            uint64_t When = 0 ;
            if(ds.contains("when"))
                When = RESTAPIHandler::from_RFC3339(ds["when"].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = "reboot";
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set( "serial" , SerialNumber );
            Params.set( "when", When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd,Response, 20000);
				return;
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Factory(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("keepRedirector") &&
            ds.contains("serialNumber")) {

			auto SerialNumber = ds["serialNumber"].toString();

			if (SerialNumber != SNum) {
				BadRequest(Response);
				return;
			}

			auto KeepRedirector = ds["keepRedirector"].toString();
			uint64_t KeepIt;
			if (KeepRedirector == "true")
				KeepIt = 1;
			else if (KeepRedirector == "false")
				KeepIt = 0;
			else {
				BadRequest(Response);
				return;
			}

			uint64_t When = 0;
			if (ds.contains("when"))
				When = RESTAPIHandler::from_RFC3339(ds["when"].toString());

			uCentral::Objects::CommandDetails Cmd;

			Cmd.SerialNumber = SerialNumber;
			Cmd.UUID = uCentral::instance()->CreateUUID();
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.Command = "factory";
			Cmd.Custom = 0;
			Cmd.RunAt = When;
			Cmd.WaitingForFile = 0;

			Poco::JSON::Object Params;

			Params.set("serial", SerialNumber);
			Params.set("keep_redirector", KeepIt);
			Params.set("when", When);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			if (uCentral::Storage::AddCommand(SerialNumber, Cmd)) {
				WaitForRPC(Cmd,Response, 20000);
				return;
			}
		}
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::LEDs(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("pattern") &&
            ds.contains("serialNumber")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Response);
                return;
            }

			auto Pattern = ds["pattern"].toString();

			if(Pattern!="on" && Pattern!="off" && Pattern!="blink")
			{
				Logger_.warning(Poco::format("LEDs(%s): Bad pattern",SerialNumber));
				BadRequest(Response);
				return;
			}

			auto Duration = ds.contains("duration") ? (uint64_t ) ds["duration"] : 20 ;

            uint64_t When = 0 ;
            if(ds.contains("when"))
                When = RESTAPIHandler::from_RFC3339(ds["when"].toString());

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = "leds";
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 0;

            Poco::JSON::Object  Params;

            Params.set("serial" , SerialNumber );
            Params.set("duration", Duration);
            Params.set("when", When);
			Params.set("pattern",Pattern);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd,Response, 20000);
				return;
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Trace(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("serialNumber") &&
            ds.contains("network") &&
            ds.contains("interface")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Response);
                return;
            }

            uint64_t Duration = ds.contains("duration") ? (uint64_t)ds["duration"] : 0;
            uint64_t When = ds.contains("when") ? RESTAPIHandler::from_RFC3339(ds["when"].toString()) : 0;
            uint64_t NumberOfPackets = ds.contains("numberOfPackets") ? (uint64_t)ds["numberOfPackets"] : 0;
            auto Network = ds["network"].toString();
            auto Interface = ds["interface"].toString();
            auto UUID = uCentral::instance()->CreateUUID();
            auto URI = uCentral::uFileUploader::FullName() + UUID ;

            uCentral::Objects::CommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = UUID;
            Cmd.SubmittedBy = UserInfo_.username_;
            Cmd.Command = "trace";
            Cmd.Custom = 0;
            Cmd.RunAt = When;
            Cmd.WaitingForFile = 1;
			Cmd.AttachType = "trace";

            Poco::JSON::Object  Params;

            Params.set("serial" , SerialNumber );
            Params.set("duration", Duration);
            Params.set("when", When);
            Params.set("packets", NumberOfPackets);
            Params.set("network", Network);
            Params.set("interface", Interface);
            Params.set("uri",URI);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
                uCentral::uFileUploader::AddUUID(UUID);
				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					Poco::JSON::Object RetObj;
					Cmd.to_json(RetObj);
					ReturnObject(RetObj, Response);
					return;
				}
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::WifiScan(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try{
		auto SNum = GetBinding("serialNumber", "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if(ds.contains("serialNumber")) {
			auto SerialNumber = ds["serialNumber"].toString();
			if(	(ds.contains("bands") && ds["bands"].isArray()) ||
				(ds.contains("channels") && ds["channels"].isArray()) ||
				(!ds.contains("bands") && !ds.contains("channels"))
				)
			{
				bool Verbose = false ;

				if(ds.contains("verbose"))
				{
					Verbose = ds["verbose"].toString() == "true";
				}

				auto UUID = uCentral::instance()->CreateUUID();
				uCentral::Objects::CommandDetails  Cmd;

				Cmd.SerialNumber = SerialNumber;
				Cmd.UUID = UUID;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.Command = "wifiscan";
				Cmd.Custom = 0;
				Cmd.RunAt = 0;
				Cmd.WaitingForFile = 0;

				Poco::JSON::Object  Params;

				Params.set("serial" , SerialNumber );
				Params.set("verbose", Verbose);

				if( ds.contains("bands")) {
					Params.set("bands",ds["bands"]);
				} else if ( ds.contains("channels")) {
					Params.set("channels",ds["channels"]);
				}

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd,Response, 20000);
					return;
				}
			}
		}
	} catch (const Poco::Exception & E) {
		Logger_.log(E);
	}
	BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::EventQueue(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		auto SNum = GetBinding("serialNumber", "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if(ds.contains("serialNumber") &&
			ds.contains("types") && ds["types"].isArray())
		{
			auto SerialNumber = ds["serialNumber"].toString();
			auto Types = ds["types"];

			if( SerialNumber == SNum ) {
				auto UUID = uCentral::instance()->CreateUUID();
				uCentral::Objects::CommandDetails  Cmd;

				Cmd.SerialNumber = SerialNumber;
				Cmd.UUID = UUID;
				Cmd.SubmittedBy = UserInfo_.username_;
				Cmd.Command = "event";
				Cmd.Custom = 0;
				Cmd.RunAt = 0;
				Cmd.WaitingForFile = 0;

				Poco::JSON::Object  Params;

				Params.set("serial" , SerialNumber );
				Params.set("types", Types);

				std::stringstream ParamStream;
				Params.stringify(ParamStream);
				Cmd.Details = ParamStream.str();

				if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
					WaitForRPC(Cmd,Response, 20000);
					return;
				}
			}
		}
	} catch ( const Poco::Exception & E ) {
		Logger_.log(E);
	}
	BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::MakeRequest(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		auto SNum = GetBinding("serialNumber", "");

		//  get the configuration from the body of the message
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if (ds.contains("serialNumber") &&
			ds.contains("message")) {

			auto SerialNumber = ds["serialNumber"].toString();
			auto MessageType = ds["message"].toString();

			if((SerialNumber != SNum) || (MessageType!="state" && MessageType!="healthcheck")) {
				BadRequest(Response);
				return;
			}

			uint64_t When = ds.contains("when") ? RESTAPIHandler::from_RFC3339(ds["when"].toString()) : 0;

			uCentral::Objects::CommandDetails  Cmd;

			Cmd.SerialNumber = SerialNumber;
			Cmd.SubmittedBy = UserInfo_.username_;
			Cmd.UUID = uCentral::instance()->CreateUUID();
			Cmd.Command = "request";
			Cmd.Custom = 0;
			Cmd.RunAt = When;
			Cmd.WaitingForFile = 0;

			Poco::JSON::Object  Params;

			Params.set("serial" , SerialNumber );
			Params.set("when", When);
			Params.set("message",MessageType);
			Params.set("request_uuid",Cmd.UUID);

			std::stringstream ParamStream;
			Params.stringify(ParamStream);
			Cmd.Details = ParamStream.str();

			if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {
				WaitForRPC(Cmd,Response, 4000);
				return;
			}
		}
	}
	catch(const Poco::Exception &E)
	{
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Response);
}
