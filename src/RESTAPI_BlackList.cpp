//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <ctime>

#include "RESTAPI_BlackList.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "uStorageService.h"
#include "RESTAPI_protocol.h"

void RESTAPI_BlackList::handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) {

	if(!ContinueProcessing(Request,Response))
		return;

	if(!IsAuthorized(Request,Response))
		return;

	ParseParameters(Request);

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
	BadRequest(Request, Response);
}

void RESTAPI_BlackList::DoDelete(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {

	try {
		auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		if(!SerialNumber.empty()) {
			if (uCentral::Storage::DeleteBlackListDevice(SerialNumber)) {
				OK(Request, Response);
			} else {
				NotFound(Request, Response);
			}
			return;
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_BlackList::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		auto Offset = GetParameter(uCentral::RESTAPI::Protocol::OFFSET, 0);
		auto Limit = GetParameter(uCentral::RESTAPI::Protocol::LIMIT, 100);

		std::vector<uCentral::Objects::BlackListedDevice>	Devices;

		if(uCentral::Storage::GetBlackListDevices(Offset,Limit,Devices))
		{
			Poco::JSON::Array Objects;
			for (const auto & i:Devices) {
				Poco::JSON::Object	Obj;
				i.to_json(Obj);
				Objects.add(Obj);
			}

			Poco::JSON::Object RetObj;
			RetObj.set(uCentral::RESTAPI::Protocol::DEVICES, Objects);
			ReturnObject(Request, RetObj, Response);

			return;
		}
	} catch(const Poco::Exception & E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}

void RESTAPI_BlackList::DoPost(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
	try {
		Poco::JSON::Parser parser;
		Poco::JSON::Object::Ptr Obj =
			parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *Obj;

		if (ds.contains(uCentral::RESTAPI::Protocol::DEVICES) && ds[uCentral::RESTAPI::Protocol::DEVICES].isArray()) {
			auto List = ds[uCentral::RESTAPI::Protocol::DEVICES];
			std::vector<uCentral::Objects::BlackListedDevice>	Devices;
			for (const auto &i : List) {
				if(i.isStruct()) {
					auto O = i.toString();
					Poco::JSON::Parser	pp;
					auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
					Poco::DynamicStruct Vars = *InnerObj;
					if (Vars.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER) && Vars.contains(uCentral::RESTAPI::Protocol::REASON)) {
						auto SerialNumber = Vars[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();
						auto Reason = Vars[uCentral::RESTAPI::Protocol::REASON].toString();
						uCentral::Objects::BlackListedDevice	D{ .SerialNumber = SerialNumber,
							.Reason = Reason,
							.Author = UserInfo_.username_,
							.Created = (uint64_t ) time(nullptr) };
						Devices.push_back(D);
					}
				}
			}
			if(!Devices.empty()) {
				if(uCentral::Storage::AddBlackListDevices(Devices)) {
					OK(Request, Response);
					return;
				}
			}
		}
	} catch (const Poco::Exception &E) {
		Logger_.log(E);
	}
	BadRequest(Request, Response);
}
