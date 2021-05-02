//
// Created by stephane bourque on 2021-03-04.
//

#include "RESTAPI_objects.h"
#include "RESTAPI_handler.h"

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

#include "uDeviceRegistry.h"

void uCentralDevice::to_json(Poco::JSON::Object & Obj) const {
	Obj.set("serialNumber", SerialNumber);
	Obj.set("deviceType", DeviceType);
	Obj.set("macAddress", MACAddress);
	Obj.set("manufacturer", Manufacturer);
	Obj.set("UUID", UUID);

	std::string C = Configuration.empty() ? "{}" : Configuration;
	Poco::JSON::Parser P;
	Poco::Dynamic::Var result = P.parse(C);
	const auto &CfgObj = result.extract<Poco::JSON::Object::Ptr>();
	Obj.set("configuration", CfgObj);

	Obj.set("notes", Notes);
	Obj.set("createdTimestamp", RESTAPIHandler::to_RFC3339(CreationTimestamp));
	Obj.set("lastConfigurationChange", RESTAPIHandler::to_RFC3339(LastConfigurationChange));
	Obj.set("lastConfigurationDownload", RESTAPIHandler::to_RFC3339(LastConfigurationDownload));
	Obj.set("owner", Owner);
	Obj.set("location", Location);
	Obj.set("firmware", Firmware);
}

void uCentralDevice::to_json_with_status(Poco::JSON::Object & Obj) const
{
	to_json(Obj);

	uCentralConnectionState	ConState;
	if(uCentral::DeviceRegistry::GetState(SerialNumber,ConState))
	{
		ConState.to_json(Obj);
	}
}

bool uCentralDevice::from_json(Poco::JSON::Object::Ptr Obj) {

    try {
        Poco::DynamicStruct ds = *Obj;

        SerialNumber = ds["serialNumber"].toString();
        DeviceType = ds["deviceType"].toString();
        MACAddress = ds["macAddress"].toString();
        UUID = ds["UUID"];

		Configuration = ds["configuration"].toString();

        if(ds.contains("notes"))
            Notes = ds["notes"].toString();
        if(ds.contains("manufacturer"))
            Manufacturer = ds["manufacturer"].toString();
		if(ds.contains("owner"))
			Owner = ds["owner"].toString();
		if(ds.contains("location"))
			Location = ds["location"].toString();
        return true;
    }
    catch (const Poco::Exception &E )
    {
    }
    return false;
}

void uCentralDevice::Print() const {
    std::cout << "Device: " << SerialNumber << " "
                            << DeviceType << " "
                            << MACAddress << " "
                            << Manufacturer << " "
                            << Configuration << std::endl;
}

void uCentralStatistics::to_json(Poco::JSON::Object & Obj) const {
    Obj.set("UUID",UUID);

	std::string D = Data.empty() ? "{}" : Data;
	Poco::JSON::Parser	P;
	Poco::Dynamic::Var result = P.parse(D);
	const auto & CfgObj = result.extract<Poco::JSON::Object::Ptr>();
	Obj.set("data",CfgObj);
	Obj.set("recorded",RESTAPIHandler::to_RFC3339(Recorded));
}

void uCentralCapabilities::to_json(Poco::JSON::Object & Obj) const {
	std::string C = Capabilities.empty() ? "{}" : Capabilities;
	Poco::JSON::Parser	P;
	Poco::Dynamic::Var result = P.parse(C);
	const auto & CfgObj = result.extract<Poco::JSON::Object::Ptr>();
	Obj.set("capabilities",CfgObj);

    Obj.set("firstUpdate",RESTAPIHandler::to_RFC3339(FirstUpdate));
    Obj.set("lastUpdate",RESTAPIHandler::to_RFC3339(LastUpdate));
}

void uCentralDeviceLog::to_json(Poco::JSON::Object & Obj) const
{
    Obj.set("log",Log);
    Obj.set("severity",Severity);

	std::string D = Data.empty() ? "{}" : Data;

	Poco::JSON::Parser	P;
	Poco::Dynamic::Var result = P.parse(D);
	const auto & CfgObj = result.extract<Poco::JSON::Object::Ptr>();
	Obj.set("data",CfgObj);

    Obj.set("recorded",RESTAPIHandler::to_RFC3339(Recorded));
    Obj.set("logType",LogType);
}

void uCentralHealthCheck::to_json(Poco::JSON::Object & Obj) const {
    Obj.set("UUID",UUID);

	std::string D = Data.empty() ? "{}" : Data;
	Poco::JSON::Parser	P;
	Poco::Dynamic::Var result = P.parse(D);
	const auto & CfgObj = result.extract<Poco::JSON::Object::Ptr>();
	Obj.set("values",CfgObj);

    Obj.set("sanity",Sanity);
    Obj.set("recorded",RESTAPIHandler::to_RFC3339(Recorded));
}

/*
    std::string     Configuration;
    std::string     Models;
    uint64_t        Created;
    uint64_t        LastModified;
    [[nodiscard]] Poco::JSON::Object to_json() const;
    bool from_JSON(Poco::JSON::Object::Ptr Obj);
 */
void uCentralDefaultConfiguration::to_json(Poco::JSON::Object & Obj) const {
    Obj.set("name",Name);
    Obj.set("modelIds",Models);
    Obj.set("description",Description);

	std::string C = Configuration.empty() ? "{}" : Configuration ;
	Poco::JSON::Parser	P;
	Poco::Dynamic::Var result = P.parse(C);
	const auto & CfgObj = result.extract<Poco::JSON::Object::Ptr>();
	Obj.set("configuration",CfgObj);

    Obj.set("created",RESTAPIHandler::to_RFC3339(Created));
    Obj.set("lastModified",RESTAPIHandler::to_RFC3339(LastModified));
}

void uCentralCommandDetails::to_json(Poco::JSON::Object & Obj) const {
    Obj.set("UUID",UUID);
    Obj.set("serialNumber",SerialNumber);
    Obj.set("command",Command);

	{
		std::string D = Details.empty() ? "{}" : Details;
		Poco::JSON::Parser P;
		Poco::Dynamic::Var result = P.parse(D);
		const auto &DetailsObj = result.extract<Poco::JSON::Object::Ptr>();
		Obj.set("details", DetailsObj);
	}

	{
		std::string D = Results.empty() ? "{}" : Results;
		Poco::JSON::Parser P;
		Poco::Dynamic::Var result = P.parse(D);
		const auto &ResultsObj = result.extract<Poco::JSON::Object::Ptr>();
		Obj.set("results", ResultsObj);
	}

    Obj.set("errorText", ErrorText);
    Obj.set("submittedBy",SubmittedBy);
    Obj.set("status",Status);
    Obj.set("submitted",RESTAPIHandler::to_RFC3339(Submitted));
    Obj.set("executed",RESTAPIHandler::to_RFC3339(Executed));
    Obj.set("completed",RESTAPIHandler::to_RFC3339(Completed));
    Obj.set("when",RESTAPIHandler::to_RFC3339(RunAt));
    Obj.set("errorCode",ErrorCode);
    Obj.set("custom",Custom);
    Obj.set("waitingForFile", WaitingForFile);
    Obj.set("attachFile",RESTAPIHandler::to_RFC3339(AttachDate));
}

bool uCentralDefaultConfiguration::from_json(Poco::JSON::Object::Ptr Obj) {
    Poco::DynamicStruct ds = *Obj;

    try {
        Name = ds["name"].toString();
        Configuration = ds["configuration"].toString();
        Models = ds["modelIds"].toString();
        if (ds.contains("description"))
            Description = ds["description"].toString();
        return true;
    }
    catch (const Poco::Exception & E )
    {

    }

    return false;
}

void  uCentralBlackListedDevice::to_json(Poco::JSON::Object & Obj) const {
	Obj.set("serialNumber",SerialNumber);
	Obj.set("author",Author);
	Obj.set("reason",Reason);
	Obj.set("created",RESTAPIHandler::to_RFC3339(Created));
}

void  uCentralConnectionState::to_json(Poco::JSON::Object & Obj) const
{
	Obj.set("serialNumber", SerialNumber);
	Obj.set("ipAddress",Address);
	Obj.set("txBytes",TX);
	Obj.set("rxBytes",RX);
	Obj.set("messageCount",MessageCount);
	Obj.set("UUID",UUID);
	Obj.set("connected",Connected);
	Obj.set("firmware",Firmware);
	Obj.set("lastContact",RESTAPIHandler::to_RFC3339(LastContact));
	Obj.set("verifiedCertificate", VerifiedCertificate);
}
