//
// Created by stephane bourque on 2021-04-13.
//

#include "RESTAPI_BlackList.h"

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"

#include "uStorageService.h"

void RESTAPI_BlackList::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {

	if(!ContinueProcessing(Request,Response))
		return;

	if(!IsAuthorized(Request,Response))
		return;

	try {
		if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_DELETE)
			DoDelete(Request, Response);
		else if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_GET)
			DoGet(Request, Response);
		else if(Request.getMethod()==Poco::Net::HTTPRequest::HTTP_POST)
			DoPost(Request, Response);
		return;
	}
	catch(const Poco::Exception &E)
	{
		Logger_.error(Poco::format("%s: failed with %s",std::string(__func__), E.displayText()));
	}
	BadRequest(Response);
}

void RESTAPI_BlackList::DoDelete(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {

	try {
		auto SerialNumber = GetParameter("serialNumber", "");

		if(!SerialNumber.empty()) {
			if (uCentral::Storage::DeleteBlackListDevice(SerialNumber))
				OK(Response);
			else
				NotFound(Response);
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Response);
}

void RESTAPI_BlackList::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		auto Offset = GetParameter("offset", 0);
		auto Limit = GetParameter("limit", 100);

		std::vector<uCentralBlackListedDevice>	Devices;

		if(uCentral::Storage::GetBlackListDevices(Offset,Limit,Devices))
		{
			Poco::JSON::Array Objects;
			for (const auto & i:Devices)
				Objects.add(i.to_json());

			Poco::JSON::Object RetObj;
			RetObj.set("devices", Objects);
			ReturnObject(RetObj, Response);

			return;
		}
	} catch(const Poco::Exception & E) {
		Logger_.log(E);
	}
	BadRequest(Response);
}

void RESTAPI_BlackList::DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if (ds.contains("devices") && ds["devices"].isArray()) {
			auto List = ds["devices"];
			std::vector<uCentralBlackListedDevice>	Devices;
			for (const auto &i : List) {
				if(i.isStruct()) {
					auto O = i.toString();
					Poco::JSON::Parser	pp;
					auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
					Poco::DynamicStruct Vars = *InnerObj;
					if (Vars.contains("serialNumber") && Vars.contains("reason")) {
						auto SerialNumber = Vars["serialNumber"].toString();
						auto Reason = Vars["reason"].toString();
						uCentralBlackListedDevice	D{ .SerialNumber = SerialNumber,
							.Reason = Reason,
							.Author = UserName_,
							.Created = (uint64_t ) time(nullptr) };
						Devices.push_back(D);
					}
				}
			}
			if(!Devices.empty()) {
				if(uCentral::Storage::AddBlackListDevices(Devices)) {
					OK(Response);
					return;
				}
			}
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Response);
}
