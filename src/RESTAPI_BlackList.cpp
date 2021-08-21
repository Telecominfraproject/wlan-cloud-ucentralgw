//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <ctime>

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "RESTAPI_BlackList.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"

namespace OpenWifi {
	void RESTAPI_BlackList::handleRequest(Poco::Net::HTTPServerRequest &Request,
										  Poco::Net::HTTPServerResponse &Response) {

		if (!ContinueProcessing(Request, Response))
			return;

		if (!IsAuthorized(Request, Response))
			return;

		ParseParameters(Request);

		try {
			if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
				DoDelete(Request, Response);
			else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
				DoGet(Request, Response);
			else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
				DoPost(Request, Response);
			return;
		} catch (const Poco::Exception &E) {
			Logger_.error(Poco::format("%s: failed with %s", std::string(__func__), E.displayText()));
		}
		BadRequest(Request, Response);
	}

	void RESTAPI_BlackList::DoDelete(Poco::Net::HTTPServerRequest &Request,
									 Poco::Net::HTTPServerResponse &Response) {

		try {
			auto SerialNumber = GetBinding(RESTAPI::Protocol::SERIALNUMBER, "");

			if (!SerialNumber.empty()) {
				if (Storage()->DeleteBlackListDevice(SerialNumber)) {
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

	void RESTAPI_BlackList::DoGet(Poco::Net::HTTPServerRequest &Request,
								  Poco::Net::HTTPServerResponse &Response) {
		try {
			if(!InitQueryBlock()) {
				BadRequest(Request, Response, "Illegal parameter value.");
				return;
			}
			std::vector<GWObjects::BlackListedDevice> Devices;

			Poco::JSON::Array Objects;
			if (Storage()->GetBlackListDevices(QB_.Offset, QB_.Limit, Devices)) {
				for (const auto &i : Devices) {
					Poco::JSON::Object Obj;
					i.to_json(Obj);
					Objects.add(Obj);
				}
			}
			Poco::JSON::Object RetObj;
			RetObj.set(RESTAPI::Protocol::DEVICES, Objects);
			ReturnObject(Request, RetObj, Response);
			return;
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
		}
		BadRequest(Request, Response);
	}

	void RESTAPI_BlackList::DoPost(Poco::Net::HTTPServerRequest &Request,
								   Poco::Net::HTTPServerResponse &Response) {
		try {
			Poco::JSON::Parser parser;
			Poco::JSON::Object::Ptr Obj =
				parser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

			if (Obj->has(RESTAPI::Protocol::DEVICES) &&
				Obj->isArray(RESTAPI::Protocol::DEVICES)) {
				std::vector<GWObjects::BlackListedDevice> Devices;
				auto DeviceArray = Obj->getArray(RESTAPI::Protocol::DEVICES);
				for (const auto &i : *DeviceArray) {
					Poco::JSON::Parser pp;
					auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
					Poco::DynamicStruct Vars = *InnerObj;
					if (Vars.contains(RESTAPI::Protocol::SERIALNUMBER) &&
						Vars.contains(RESTAPI::Protocol::REASON)) {
						auto SerialNumber = Vars[RESTAPI::Protocol::SERIALNUMBER].toString();
						auto Reason = Vars[RESTAPI::Protocol::REASON].toString();
						GWObjects::BlackListedDevice D{.SerialNumber = SerialNumber,
															   .Reason = Reason,
															   .Author = UserInfo_.webtoken.username_,
															   .Created = (uint64_t)time(nullptr)};
						Devices.push_back(D);
					}
				}
				if (!Devices.empty()) {
					if (Storage()->AddBlackListDevices(Devices)) {
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
}