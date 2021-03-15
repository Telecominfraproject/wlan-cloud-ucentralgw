//
// Created by stephane bourque on 2021-03-04.
//

#include "RESTAPI_Objects.h"
#include "RESTAPI_Handler.h"

Poco::JSON::Object uCentralDevice::to_json() const
{
    Poco::JSON::Object  Obj;

    Obj.set("serialNumber",SerialNumber);
    Obj.set("deviceType",DeviceType);
    Obj.set("macAddress",MACAddress);
    Obj.set("manufacturer",Manufacturer);
    Obj.set("UUID",UUID);
    Obj.set("configuration",Configuration);
    Obj.set("notes",Notes);
    Obj.set("createdTimestamp",RESTAPIHandler::to_RFC3339(CreationTimestamp));
    Obj.set("lastConfigurationChange",RESTAPIHandler::to_RFC3339(LastConfigurationChange));
    Obj.set("lastConfigurationDownload",RESTAPIHandler::to_RFC3339(LastConfigurationDownload));

    return Obj;
}

bool uCentralDevice::from_JSON(Poco::JSON::Object::Ptr Obj) {

    Poco::DynamicStruct ds = *Obj;

    SerialNumber = ds["serialNumber"].toString();
    DeviceType = ds["deviceType"].toString();
    MACAddress = ds["macAddress"].toString();
    Manufacturer = ds["manufacturer"].toString();
    UUID = ds["UUID"];
    Configuration = ds["configuration"].toString();
    Notes = ds["notes"].toString();

    return true;
}

void uCentralDevice::Print() const {
    std::cout << "Device: " << SerialNumber << " " << DeviceType << " " << MACAddress << " " << Manufacturer << " " << Configuration << std::endl;
}

Poco::JSON::Object uCentralStatistics::to_json() const {
    Poco::JSON::Object  Obj;
    Obj.set("UUID",UUID);
    Obj.set("values",Values);
    Obj.set("recorded",RESTAPIHandler::to_RFC3339(Recorded));
    return Obj;
};

Poco::JSON::Object uCentralCapabilities::to_json() const {
    Poco::JSON::Object  Obj;
    Obj.set("deviceType",Capabilities);
    Obj.set("createdTimestamp",RESTAPIHandler::to_RFC3339(FirstUpdate));
    Obj.set("lastConfigurationChange",RESTAPIHandler::to_RFC3339(LastUpdate));
    return Obj;
};

Poco::JSON::Object uCentralDeviceLog::to_json() const
{
    Poco::JSON::Object  Obj;
    Obj.set("log",Log);
    Obj.set("severity",Severity);
    Obj.set("data",Data);
    Obj.set("recorded",RESTAPIHandler::to_RFC3339(Recorded));
    return Obj;
}

Poco::JSON::Object  uCentralHealthcheck::to_json() const {
    Poco::JSON::Object  Obj;
    Obj.set("UUID",UUID);
    Obj.set("values",Data);
    Obj.set("sanity",Sanity);
    Obj.set("recorded",RESTAPIHandler::to_RFC3339(Recorded));
    return Obj;
};

