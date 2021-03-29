//
// Created by stephane bourque on 2021-03-04.
//

#include "RESTAPI_deviceCommandHandler.h"
#include "uAuthService.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"
#include "Poco/UUIDGenerator.h"
#include "Poco/UUID.h"

#include "uCentral.h"
#include "uCentralConfig.h"

void RESTAPI_deviceCommandHandler::handleRequest(HTTPServerRequest& Request, HTTPServerResponse& Response)
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
        } else if (Command == "blink" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Blink(Request, Response);
        } else {
            BadRequest(Response);
        }
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Response);
};

void  RESTAPI_deviceCommandHandler::GetCapabilities(HTTPServerRequest &Request, HTTPServerResponse &Response) {
    uCentralCapabilities    Caps;
    try {
        auto SerialNumber = GetBinding("serialNumber", "");

        if (uCentral::Storage::GetDeviceCapabilities(SerialNumber, Caps)) {
            Poco::JSON::Object RetObj = Caps.to_json();
            RetObj.set("serialNumber", SerialNumber);
            ReturnObject( RetObj, Response );
        } else
            NotFound(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void  RESTAPI_deviceCommandHandler::DeleteCapabilities(HTTPServerRequest &Request, HTTPServerResponse &Response) {
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
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetStatistics(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);

        std::vector<uCentralStatistics> Stats;

        uCentral::Storage::GetStatisticsData(SerialNumber, StartDate, EndDate, Offset, Limit,
                                                                  Stats);

        Poco::JSON::Array ArrayObj;

        for (auto i : Stats) {
            Poco::JSON::Object Obj = i.to_json();
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
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::DeleteStatistics(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));

        if (uCentral::Storage::DeleteStatisticsData(SerialNumber, StartDate, EndDate))
            OK(Response);
        else
            BadRequest(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetStatus(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");

        uCentral::DeviceRegistry::ConnectionState State;

        if (uCentral::DeviceRegistry::GetState(SerialNumber, State)) {

            Poco::JSON::Object RetObject = State.to_JSON();

            ReturnObject(RetObject, Response);

        } else
            NotFound(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Configure(HTTPServerRequest& Request, HTTPServerResponse& Response) {
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
                uCentralCommandDetails  Cmd;

                Cmd.SerialNumber = SerialNumber;
                Cmd.UUID = uCentral::instance()->CreateUUID();
                Cmd.SubmittedBy = UserName_;
                Cmd.Command = "configure";
                Cmd.Custom = 0;
                Cmd.RunAt = When;

                uCentral::Config::Config    Cfg(Configuration);

                Cfg.SetUUID(NewUUID);

                Poco::JSON::Object  Params;

                Params.set( "serial" , SerialNumber );
                Params.set("uuid",NewUUID);
                Params.set("when",When);
                Params.set("config", Cfg.to_json());

                std::stringstream ParamStream;
                Params.stringify(ParamStream);
                Cmd.Details = ParamStream.str();

                if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {

                    Poco::JSON::Object RetObj;

                    RetObj.set("serialNumber", SerialNumber);
                    RetObj.set("command", Cmd.Command);
                    RetObj.set("UUID", Cmd.UUID);

                    ReturnObject(RetObj, Response);

                    return;
                }
                else
                {
                    BadRequest(Response);
                    return;
                }
            }
            else {
                BadRequest(Response);
            }
        }
        else {
            BadRequest(Response);
        }
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Upgrade(HTTPServerRequest &Request, HTTPServerResponse &Response) {
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

            uCentralCommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserName_;
            Cmd.Custom = 0;
            Cmd.Command = "upgrade";
            Cmd.RunAt = When;

            Poco::JSON::Object  Params;

            Params.set( "serial" , SerialNumber );
            Params.set( "uri", URI);
            Params.set( "when", When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {

                Poco::JSON::Object RetObj;

                RetObj.set("serialNumber", SerialNumber);
                RetObj.set("command", Cmd.Command);
                RetObj.set("UUID", Cmd.UUID);

                ReturnObject(RetObj, Response);

                return;
            }
            else
            {
                BadRequest(Response);
                return;
            }
        }

        else
            BadRequest(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetLogs(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);

        std::vector<uCentralDeviceLog> Logs;

        uCentral::Storage::GetLogData(SerialNumber, StartDate, EndDate, Offset, Limit,
                                                                  Logs);
        Poco::JSON::Array ArrayObj;

        for (auto i : Logs) {
            Poco::JSON::Object Obj = i.to_json();
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
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::DeleteLogs(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));

        if (uCentral::Storage::DeleteLogData(SerialNumber, StartDate, EndDate))
            OK(Response);
        else
            BadRequest(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetChecks(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);

        std::vector<uCentralHealthcheck> Checks;

        uCentral::Storage::GetHealthCheckData(SerialNumber, StartDate, EndDate, Offset, Limit,
                                      Checks);
        Poco::JSON::Array ArrayObj;

        for (auto i : Checks) {
            Poco::JSON::Object Obj = i.to_json();
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
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::DeleteChecks(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetBinding("serialNumber", "");
        auto StartDate = RESTAPIHandler::from_RFC3339(GetParameter("startDate", ""));
        auto EndDate = RESTAPIHandler::from_RFC3339(GetParameter("endDate", ""));

        if (uCentral::Storage::DeleteHealthCheckData(SerialNumber, StartDate, EndDate))
            OK(Response);
        else
            BadRequest(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__) ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::ExecuteCommand(HTTPServerRequest& Request, HTTPServerResponse& Response) {
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

            uCentralCommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserName_;
            Cmd.Command = Command;
            Cmd.Custom = 1;
            Cmd.RunAt = RunAt;

            Parser parser2;

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

                Poco::JSON::Object RetObj;

                RetObj.set("serialNumber", SerialNumber);
                RetObj.set("command", Cmd.Command);
                RetObj.set("UUID", Cmd.UUID);

                ReturnObject(RetObj, Response);

                return;
            }
            else
            {
                BadRequest(Response);
                return;
            }
        }
        else
            BadRequest(Response);

        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Reboot(HTTPServerRequest& Request, HTTPServerResponse& Response) {
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

            uCentralCommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserName_;
            Cmd.Command = "reboot";
            Cmd.Custom = 0;
            Cmd.RunAt = When;

            Poco::JSON::Object  Params;

            Params.set( "serial" , SerialNumber );
            Params.set( "when", When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {

                Poco::JSON::Object RetObj;

                RetObj.set("serialNumber", SerialNumber);
                RetObj.set("command", Cmd.Command);
                RetObj.set("UUID", Cmd.UUID);

                ReturnObject(RetObj, Response);

                return;
            }
            else
            {
                BadRequest(Response);
                return;
            }
        }
        else {
            BadRequest(Response);
        }
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

/*
   FactoryRequest:
      type: object
      properties:
        serialNumber:
          type: string
        when:
          type: string
          format: 'date-time'
        keepRedirector:
          type: boolean
 */

void RESTAPI_deviceCommandHandler::Factory(HTTPServerRequest &Request, HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("keepRedirector") &&
            ds.contains("serialNumber")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Response);
                return;
            }

            auto KeepRedirector = ds["keepRedirector"];

            uint64_t When = 0 ;
            if(ds.contains("when"))
                When = RESTAPIHandler::from_RFC3339(ds["when"].toString());

            uCentralCommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserName_;
            Cmd.Command = "factory";
            Cmd.Custom = 0;
            Cmd.RunAt = When;

            Poco::JSON::Object  Params;

            Params.set( "serial" , SerialNumber );
            Params.set( "keep_redirector", KeepRedirector);
            Params.set( "when", When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {

                Poco::JSON::Object RetObj;

                RetObj.set("serialNumber", SerialNumber);
                RetObj.set("command", Cmd.Command);
                RetObj.set("UUID", Cmd.UUID);

                ReturnObject(RetObj, Response);

                return;
            }
            else
            {
                BadRequest(Response);
                return;
            }
        }
        else
            BadRequest(Response);

        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}

/*
BlinkRequest:
type: object
        properties:
serialNumber:
type: string
        when:
type: string
        format: 'date-time'
duration:
type: integer
        format: int64
        */

void RESTAPI_deviceCommandHandler::Blink(HTTPServerRequest &Request, HTTPServerResponse &Response) {
    try {
        auto SNum = GetBinding("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.contains("duration") &&
            ds.contains("serialNumber")) {

            auto SerialNumber = ds["serialNumber"].toString();

            if(SerialNumber != SNum) {
                BadRequest(Response);
                return;
            }

            auto Duration = ds["duration"];

            uint64_t When = 0 ;
            if(ds.contains("when"))
                When = RESTAPIHandler::from_RFC3339(ds["when"].toString());

            uCentralCommandDetails  Cmd;

            Cmd.SerialNumber = SerialNumber;
            Cmd.UUID = uCentral::instance()->CreateUUID();
            Cmd.SubmittedBy = UserName_;
            Cmd.Command = "blink";
            Cmd.Custom = 0;
            Cmd.RunAt = When;

            Poco::JSON::Object  Params;

            Params.set( "serial" , SerialNumber );
            Params.set( "duration", Duration);
            Params.set( "when", When);

            std::stringstream ParamStream;
            Params.stringify(ParamStream);
            Cmd.Details = ParamStream.str();

            if(uCentral::Storage::AddCommand(SerialNumber,Cmd)) {

                Poco::JSON::Object RetObj;

                RetObj.set("serialNumber", SerialNumber);
                RetObj.set("command", Cmd.Command);
                RetObj.set("UUID", Cmd.UUID);

                ReturnObject(RetObj, Response);

                return;
            }
            else
            {
                BadRequest(Response);
                return;
            }
        }
        else
            BadRequest(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
    }
    BadRequest(Response);
}
