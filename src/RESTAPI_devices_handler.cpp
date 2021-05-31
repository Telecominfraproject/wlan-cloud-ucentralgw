//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "Poco/Array.h"
#include "Poco/JSON/Stringifier.h"

#include "RESTAPI_devices_handler.h"
#include "uStorageService.h"
#include "uUtils.h"
#include "RESTAPI_protocol.h"

void RESTAPI_devices_handler::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
{
    if(!ContinueProcessing(Request,Response))
        return;

    if(!IsAuthorized(Request,Response))
        return;

    try {
        if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
			ParseParameters(Request);
			InitQueryBlock();
			auto serialOnly = GetBoolParameter(uCentral::RESTAPI::Protocol::SERIALONLY, false);
			auto countOnly = GetBoolParameter(uCentral::RESTAPI::Protocol::COUNTONLY, false);
			auto deviceWithStatus = GetBoolParameter(uCentral::RESTAPI::Protocol::DEVICEWITHSTATUS, false);

			Logger_.debug(Poco::format("DEVICES: from %Lu, limit of %Lu, filter='%s'.",
											 (uint64_t)QB_.Offset, (uint64_t)QB_.Limit, QB_.Filter));

			RESTAPIHandler::PrintBindings();

			Poco::JSON::Object RetObj;

			if (!QB_.Select.empty()) {

				Poco::JSON::Array Objects;
				std::cout << __LINE__ << "Select: " << QB_.Select << std::endl;
				std::vector<std::string>	Numbers = uCentral::Utils::Split(QB_.Select);
				for(auto &i:Numbers) {
					uCentral::Objects::Device	D;
					if(uCentral::Storage::GetDevice(i,D)) {
						std::cout << "Adding device: " << i << std::endl;
						Poco::JSON::Object	Obj;
						if (deviceWithStatus)
							D.to_json_with_status(Obj);
						else
							D.to_json(Obj);
						Objects.add(Obj);
					} else {
						std::cout << "Could not add device: " << i << std::endl;
					}
				}

				if(deviceWithStatus)
					RetObj.set(uCentral::RESTAPI::Protocol::DEVICESWITHSTATUS, Objects);
				else
					RetObj.set(uCentral::RESTAPI::Protocol::DEVICES, Objects);

			} else  if (countOnly == true) {
				uint64_t Count = 0;
				if (uCentral::Storage::GetDeviceCount(Count)) {
					RetObj.set(uCentral::RESTAPI::Protocol::COUNT, Count);
				}
			} else if (serialOnly) {
				std::vector<std::string> SerialNumbers;
				uCentral::Storage::GetDeviceSerialNumbers(QB_.Offset, QB_.Limit, SerialNumbers);
				Poco::JSON::Array Objects;
				for (const auto &i : SerialNumbers) {
					Objects.add(i);
				}
				RetObj.set(uCentral::RESTAPI::Protocol::SERIALNUMBERS, Objects);
			} else {
				std::vector<uCentral::Objects::Device> Devices;
				uCentral::Storage::GetDevices(QB_.Offset, QB_.Limit, Devices);
				Poco::JSON::Array Objects;
				for (const auto &i : Devices) {
					Poco::JSON::Object	Obj;
					if(deviceWithStatus)
						i.to_json_with_status(Obj);
					else
						i.to_json(Obj);
					Objects.add(Obj);
				}
				if(deviceWithStatus)
					RetObj.set(uCentral::RESTAPI::Protocol::DEVICESWITHSTATUS, Objects);
				else
					RetObj.set(uCentral::RESTAPI::Protocol::DEVICES, Objects);
			}
			ReturnObject(Request, RetObj, Response);
			return;
		}
    }
    catch (const Poco::Exception & E)
    {
        Logger_.warning(Poco::format("%s: Failed with: %s",std::string(__func__), E.displayText()));
    }
	BadRequest(Request, Response);
}
