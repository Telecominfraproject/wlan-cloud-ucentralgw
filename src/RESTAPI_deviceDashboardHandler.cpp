//
// Created by stephane bourque on 2021-07-21.
//

#include "RESTAPI_deviceDashboardHandler.h"
#include "Daemon.h"
#include "Dashboard.h"

namespace OpenWifi {
	void RESTAPI_deviceDashboardHandler::handleRequest(Poco::Net::HTTPServerRequest &Request,
													   Poco::Net::HTTPServerResponse &Response) {

		if (!ContinueProcessing(Request, Response))
			return;

		if (!IsAuthorized(Request, Response))
			return;

		if (Request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
			DoGet(Request, Response);
		} else {
			BadRequest(Request, Response, "Unsupported method.");
		}
	}

	void RESTAPI_deviceDashboardHandler::DoGet(Poco::Net::HTTPServerRequest &Request, Poco::Net::HTTPServerResponse &Response) {
		Daemon()->GetDashboard().Create();
		Poco::JSON::Object	Answer;
		Daemon()->GetDashboard().Report().to_json(Answer);
		ReturnObject(Request, Answer, Response);
	}
}