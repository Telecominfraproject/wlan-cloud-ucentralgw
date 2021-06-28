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

namespace uCentral {
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
			auto SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

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
			InitQueryBlock();
			std::vector<uCentral::Objects::BlackListedDevice> Devices;

			Poco::JSON::Array Objects;
			if (Storage()->GetBlackListDevices(QB_.Offset, QB_.Limit, Devices)) {
				for (const auto &i : Devices) {
					Poco::JSON::Object Obj;
					i.to_json(Obj);
					Objects.add(Obj);
				}
			}
			Poco::JSON::Object RetObj;
			RetObj.set(uCentral::RESTAPI::Protocol::DEVICES, Objects);
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

			if (Obj->has(uCentral::RESTAPI::Protocol::DEVICES) &&
				Obj->isArray(uCentral::RESTAPI::Protocol::DEVICES)) {
				std::vector<uCentral::Objects::BlackListedDevice> Devices;
				auto DeviceArray = Obj->getArray(uCentral::RESTAPI::Protocol::DEVICES);
				for (const auto &i : *DeviceArray) {
					Poco::JSON::Parser pp;
					auto InnerObj = pp.parse(i).extract<Poco::JSON::Object::Ptr>();
					Poco::DynamicStruct Vars = *InnerObj;
					if (Vars.contains(uCentral::RESTAPI::Protocol::SERIALNUMBER) &&
						Vars.contains(uCentral::RESTAPI::Protocol::REASON)) {
						auto SerialNumber = Vars[uCentral::RESTAPI::Protocol::SERIALNUMBER].toString();
						auto Reason = Vars[uCentral::RESTAPI::Protocol::REASON].toString();
						uCentral::Objects::BlackListedDevice D{.SerialNumber = SerialNumber,
															   .Reason = Reason,
															   .Author = UserInfo_.username_,
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