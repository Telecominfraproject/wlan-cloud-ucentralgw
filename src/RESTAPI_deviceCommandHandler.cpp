//
// Created by stephane bourque on 2021-03-04.
//

#include "RESTAPI_deviceCommandHandler.h"
#include "uAuthService.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"

void RESTAPI_deviceCommandHandler::handleRequest(HTTPServerRequest& Request, HTTPServerResponse& Response)
{
    try {
        if (!ContinueProcessing(Request, Response))
            return;

        if (!IsAuthorized(Request, Response))
            return;

        std::string Command = GetBinding("command", "-");
        if (Command == "-") {
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
        } else if (Command == "statistics" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetStatistics(Request, Response);
        } else if (Command == "statistics" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_DELETE) {
            DeleteStatistics(Request, Response);
        } else if (Command == "status" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_GET) {
            GetStatus(Request, Response);
        } else if (Command == "command" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            ExecuteCommand(Request, Response);
        } else if (Command == "configure" && Request.getMethod() == Poco::Net::HTTPServerRequest::HTTP_POST) {
            Configure(Request, Response);
        } else {
            BadRequest(Response);
        }
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",__FUNCTION__ ,E.displayText()));
    }
    BadRequest(Response);
};

void  RESTAPI_deviceCommandHandler::GetCapabilities(HTTPServerRequest &Request, HTTPServerResponse &Response) {
    uCentralCapabilities    Caps;
    try {
        auto SerialNumber = GetParameter("serialNumber", "");

        if (uCentral::Storage::Service::instance()->GetDeviceCapabilities(SerialNumber, Caps)) {
            PrepareResponse(Response);
            Poco::JSON::Object ReturnObject = Caps.to_json();
        } else
            NotFound(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",__FUNCTION__ ,E.displayText()));
    }
    BadRequest(Response);
}

void  RESTAPI_deviceCommandHandler::DeleteCapabilities(HTTPServerRequest &Request, HTTPServerResponse &Response) {
    try {
        auto SerialNumber = GetParameter("serialNumber", "");

        if (uCentral::Storage::Service::instance()->DeleteDeviceCapabilities(SerialNumber))
            OK(Response);
        else
            NotFound(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",__FUNCTION__ ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetStatistics(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetParameter("serialNumber", "");
        auto StartDate = GetParameter("startDate", "");
        auto EndDate = GetParameter("endDate", "");
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);

        std::vector<uCentralStatistics> Stats;

        uCentral::Storage::Service::instance()->GetStatisticsData(SerialNumber, StartDate, EndDate, Offset, Limit,
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
        logger_.error(Poco::format("%s: failed with %s",__FUNCTION__ ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::DeleteStatistics(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetParameter("serialNumber", "");
        auto StartDate = GetParameter("startDate", "");
        auto EndDate = GetParameter("endDate", "");
        auto Offset = GetParameter("offset", 0);
        auto Limit = GetParameter("limit", 100);

        std::vector<uCentralStatistics> Stats;

        if (uCentral::Storage::Service::instance()->DeleteStatisticsData(SerialNumber, StartDate, EndDate, Offset, Limit))
            OK(Response);
        else
            BadRequest(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",__FUNCTION__ ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::GetStatus(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetParameter("serialNumber", "");

        uCentral::DeviceRegistry::ConnectionState State;

        if (uCentral::DeviceRegistry::Service::instance()->GetState(SerialNumber, State)) {
            Poco::JSON::Object RetObject = State.to_JSON();
            ReturnObject(RetObject, Response);
        } else
            NotFound(Response);
        return;
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",__FUNCTION__ ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::Configure(HTTPServerRequest& Request, HTTPServerResponse& Response) {
    try {
        auto SerialNumber = GetParameter("serialNumber", "");

        //  get the configuration from the body of the message
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr Obj = parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *Obj;

        if (ds.find("serialNumber") != ds.end() &&
            ds.find("UUID") != ds.end() &&
            ds.find("configuration") != ds.end()) {

            auto SNum = ds["serialNumber"].toString();
            auto UUID = ds["UUID"];
            auto Configuration = ds["configuration"].toString();

            if (uCentral::Storage::Service::instance()->UpdateDeviceConfiguration(SNum, Configuration)) {
                OK(Response);
                return;
            }
        }
    }
    catch(const Poco::Exception &E)
    {
        logger_.error(Poco::format("%s: failed with %s",__FUNCTION__ ,E.displayText()));
    }
    BadRequest(Response);
}

void RESTAPI_deviceCommandHandler::ExecuteCommand(HTTPServerRequest& Request, HTTPServerResponse& Response) {

}

void RESTAPI_deviceCommandHandler::GetLogs(HTTPServerRequest& Request, HTTPServerResponse& Response) {

}

