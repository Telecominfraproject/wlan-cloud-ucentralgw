//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "RESTAPI_device_handler.h"
#include "Poco/JSON/Parser.h"
#include "RESTAPI_protocol.h"
#include "StorageService.h"
#include "Utils.h"

namespace uCentral {
void RESTAPI_device_handler::handleRequest(Poco::Net::HTTPServerRequest &Request,
										   Poco::Net::HTTPServerResponse &Response) {
	if (!ContinueProcessing(Request, Response))
		return;

	if (!IsAuthorized(Request, Response))
		return;

	ParseParameters(Request);
	if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
		std::string SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");
		uCentral::Objects::Device Device;

		if (Storage()->GetDevice(SerialNumber, Device)) {
			Poco::JSON::Object Obj;
			Device.to_json(Obj);
			ReturnObject(Request, Obj, Response);
		} else {
			NotFound(Request, Response);
		}
	} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE) {
		std::string SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		if (Storage()->DeleteDevice(SerialNumber)) {
			OK(Request, Response);
		} else {
			NotFound(Request, Response);
		}
	} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
		std::string SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		Poco::JSON::Parser IncomingParser;
		Poco::JSON::Object::Ptr Obj =
			IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();
		uCentral::Objects::Device Device;
		if (!Device.from_json(Obj)) {
			BadRequest(Request, Response);
			return;
		}

		//	make sure the username is filled for the notes.
		for(auto &i:Device.Notes)
			i.createdBy = UserInfo_.userinfo.email;

		if (!uCentral::Utils::ValidSerialNumber(Device.SerialNumber)) {
			Logger_.warning(Poco::format("CREATE-DEVICE(%s): Illegal name.", Device.SerialNumber));
			BadRequest(Request, Response);
			return;
		}

		Device.Print();

		Device.UUID = time(nullptr);
		if (Storage()->CreateDevice(Device)) {
			Poco::JSON::Object DevObj;
			Device.to_json(DevObj);
			ReturnObject(Request, DevObj, Response);
		} else {
			BadRequest(Request, Response);
		}
	} else if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT) {
		std::string SerialNumber = GetBinding(uCentral::RESTAPI::Protocol::SERIALNUMBER, "");

		Poco::JSON::Parser IncomingParser;
		Poco::JSON::Object::Ptr Obj =
			IncomingParser.parse(Request.stream()).extract<Poco::JSON::Object::Ptr>();

		uCentral::Objects::Device Device;
		if (!Device.from_json(Obj)) {
			BadRequest(Request, Response);
			return;
		}

		for(auto &i:Device.Notes)
			i.createdBy = UserInfo_.userinfo.email;

		if (Storage()->UpdateDevice(Device)) {
			Poco::JSON::Object DevObj;
			Device.to_json(DevObj);
			ReturnObject(Request, DevObj, Response);
		} else {
			BadRequest(Request, Response);
		}
	} else {
		BadRequest(Request, Response);
	}
}

}