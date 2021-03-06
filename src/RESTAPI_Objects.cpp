//
// Created by stephane bourque on 2021-03-04.
//

#include "RESTAPI_Objects.h"
#include "RESTAPI_Handler.h"

Poco::JSON::Object  uCentralDevice::to_json()
{
    Poco::JSON::Object  Obj;

    Obj.set("serialNumber",SerialNumber);
    Obj.set("deviceType",DeviceType);
    Obj.set("macAddress",MACAddress);
    Obj.set("manufacturer",Manufacturer);
    Obj.set("UUID",UUID);
    Obj.set("configuration",Configuration);
    Obj.set("notes",Notes);
    Obj.set("createdTimestamp",RESTAPIHandler::RFC3339(CreationTimestamp));
    Obj.set("lastConfigurationChange",RESTAPIHandler::RFC3339(LastConfigurationChange));
    Obj.set("lastConfigurationDownload",RESTAPIHandler::RFC3339(LastConfigurationDownload));

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

void uCentralDevice::Print() {
    std::cout << "Device: " << SerialNumber << " " << DeviceType << " " << MACAddress << " " << Manufacturer << " " << Configuration << std::endl;
}

Poco::JSON::Object uCentralStatistics::to_json() {
    Poco::JSON::Object  Obj;

    return Obj;
};

Poco::JSON::Object  uCentralCapabilities::to_json() {
    Poco::JSON::Object  Obj;

    Obj.set("serialNumber",SerialNumber);
    Obj.set("deviceType",Capabilities);
    Obj.set("createdTimestamp",RESTAPIHandler::RFC3339(FirstUpdate));
    Obj.set("lastConfigurationChange",RESTAPIHandler::RFC3339(LastUpdate));

    return Obj;
};

