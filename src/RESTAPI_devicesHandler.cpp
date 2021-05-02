//
// Created by stephane bourque on 2021-03-03.
//

#include "RESTAPI_devicesHandler.h"
#include "uStorageService.h"
#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"

using Poco::Array;

void RESTAPI_devicesHandler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    try {
        if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
			ParseParameters(Request);

			auto Offset = GetParameter("offset", 0);
			auto Limit = GetParameter("limit", 100);
			auto Filter = GetParameter("filter", "");
			auto Select = GetParameter("select", "");
			auto serialOnly = GetBoolParameter("serialOnly", false);
			auto countOnly = GetBoolParameter("countOnly", false);
			auto deviceWithStatus = GetBoolParameter("deviceWithStatus", false);

			std::cout << "Select: " << Select << std::endl;

			Logger_.information(Poco::format("DEVICES: from %Lu, limit of %Lu, filter='%s'.",
											 (uint64_t)Offset, (uint64_t)Limit, Filter));

			RESTAPIHandler::PrintBindings();

			Poco::JSON::Object RetObj;

			if (!Select.empty()) {

				unsigned long P=0;
				std::string S;
				uCentralDevice	D;
				Poco::JSON::Array Objects;

				while(P<Select.size())
				{
					auto P2 = Select.find_first_of(',', P);
					if(P2==std::string::npos) {
						S = Select.substr(P);
						std::cout << "Serial number: " << S << std::endl;
						if(uCentral::Storage::GetDevice(S,D))
						{
							Poco::JSON::Object	Obj;
							D.to_json(Obj);
							Objects.add(Obj);
						}
						break;
					}
					else {
						S = Select.substr(P, P2);
						std::cout << "Serial number: " << S << std::endl;
						if(uCentral::Storage::GetDevice(S,D))
						{
							Poco::JSON::Object	Obj;
							D.to_json(Obj);
							Objects.add(Obj);
						}
					}
					P=P2+1;
				}
				RetObj.set("devices", Objects);
			} else  if (countOnly == true) {
				uint64_t Count = 0;
				if (uCentral::Storage::GetDeviceCount(Count)) {
					RetObj.set("count", Count);
				}
			} else if (deviceWithStatus) {
				std::vector<uCentralDevice> Devices;
				uCentral::Storage::GetDevices(Offset, Limit, Devices);
				Poco::JSON::Array Objects;
				for (const auto &i : Devices) {
					Poco::JSON::Object	Obj;
					i.to_json_with_status(Obj);
					Objects.add(Obj);
				}
				RetObj.set("devicesWithStatus", Objects);
			} else if (serialOnly) {
				std::vector<std::string> SerialNumbers;
				uCentral::Storage::GetDeviceSerialNumbers(Offset, Limit, SerialNumbers);
				Poco::JSON::Array Objects;
				for (const auto &i : SerialNumbers) {
					Objects.add(i);
				}
				RetObj.set("serialNumbers", Objects);
			} else {
				std::vector<uCentralDevice> Devices;
				uCentral::Storage::GetDevices(Offset, Limit, Devices);
				Poco::JSON::Array Objects;
				for (const auto &i : Devices) {
					Poco::JSON::Object	Obj;
					i.to_json(Obj);
					Objects.add(Obj);
				}
				RetObj.set("devices", Objects);
			}
			ReturnObject(RetObj, Response);
			return;
		}
    }
    catch (const Poco::Exception & E)
    {
        Logger_.warning(Poco::format("%s: Failed with: %s",std::string(__func__), E.displayText()));
    }
}
