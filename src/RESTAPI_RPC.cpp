//
// Created by stephane bourque on 2021-06-28.
//
#include <cctype>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <future>
#include <numeric>
#include <chrono>
#include "RESTAPI_RPC.h"

#include "StorageService.h"
#include "DeviceRegistry.h"
#include "CommandManager.h"
#include "uCentralProtocol.h"

namespace OpenWifi::RESTAPI_RPC {
	void SetCommandAsPending(GWObjects::CommandDetails &Cmd,
											 Poco::Net::HTTPServerRequest &Request,
											 Poco::Net::HTTPServerResponse &Response, RESTAPIHandler *Handler,
											 Poco::Logger &Logger) {
		if (Storage()->AddCommand(Cmd.SerialNumber, Cmd, Storage::COMMAND_PENDING)) {
			Poco::JSON::Object RetObj;
			Cmd.to_json(RetObj);
			Handler->ReturnObject(Request, RetObj, Response);
			return;
		} else {
			Handler->ReturnStatus(Request, Response,
								  Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
			return;
		}
	}

	void SetCommandAsTimedOut(GWObjects::CommandDetails &Cmd,
							 Poco::Net::HTTPServerRequest &Request,
							 Poco::Net::HTTPServerResponse &Response, RESTAPIHandler *Handler,
							 Poco::Logger &Logger) {
		if (Storage()->AddCommand(Cmd.SerialNumber, Cmd, Storage::COMMAND_TIMEDOUT)) {
			Poco::JSON::Object RetObj;
			Cmd.to_json(RetObj);
			Handler->ReturnObject(Request, RetObj, Response);
			return;
		} else {
			Handler->ReturnStatus(Request, Response,
								  Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
			return;
		}
	}

	void WaitForCommand(GWObjects::CommandDetails &Cmd,
						Poco::JSON::Object  & Params,
						Poco::Net::HTTPServerRequest &Request,
						Poco::Net::HTTPServerResponse &Response,
						int64_t WaitTimeInMs,
						Poco::JSON::Object * ObjectToReturn,
						RESTAPIHandler * Handler,
						Poco::Logger &Logger) {

		// 	if the command should be executed in the future, or if the device is not connected, then we should just add the command to
		//	the DB and let it figure out when to deliver the command.
		if(Cmd.RunAt || !DeviceRegistry()->Connected(Cmd.SerialNumber)) {
			SetCommandAsPending(Cmd, Request, Response, Handler, Logger);
			return;
		}

		Cmd.Executed = std::time(nullptr);

		uint64_t RPC_Id=0;
		if (CommandManager()->SendCommand(Cmd.SerialNumber, Cmd.Command, Params, Cmd.UUID, RPC_Id)) {
			CommandTag T;
			while (CommandManager()->Running() && WaitTimeInMs > 0) {
				if (CommandManager()->GetCommand(RPC_Id, Cmd.SerialNumber, T)) {
					auto Answer = T.Result;
					if (Answer->has("result") && Answer->isObject("result")) {
						auto ResultFields =
							Answer->get("result").extract<Poco::JSON::Object::Ptr>();
						if (ResultFields->has("status") && ResultFields->isObject("status")) {
							auto StatusInnerObj =
								ResultFields->get("status").extract<Poco::JSON::Object::Ptr>();
							if (StatusInnerObj->has("error"))
								Cmd.ErrorCode = StatusInnerObj->get("error");
							if (StatusInnerObj->has("text"))
								Cmd.ErrorText = StatusInnerObj->get("text").toString();
							std::stringstream ResultText;
							Poco::JSON::Stringifier::stringify(Answer->get("result"), ResultText);
							Cmd.Results = ResultText.str();
							Cmd.Status = "completed";
							Cmd.Completed = time(nullptr);

							if (Cmd.ErrorCode && Cmd.Command == uCentralProtocol::TRACE) {
								Cmd.WaitingForFile = 0;
								Cmd.AttachDate = Cmd.AttachSize = 0;
								Cmd.AttachType = "";
							}

							//	Add the completed command to the database...
							Storage()->AddCommand(Cmd.SerialNumber, Cmd,
												  Storage::COMMAND_COMPLETED);

							if (ObjectToReturn) {
								Handler->ReturnObject(Request, *ObjectToReturn, Response);
							} else {
								Poco::JSON::Object O;
								Cmd.to_json(O);
								Handler->ReturnObject(Request, O, Response);
							}
							return;
						} else {
							Logger.information(Poco::format("Invalid response for command '%s'. Missing status.", Cmd.UUID));
						}
					} else {
						Logger.information(Poco::format("Invalid response for command '%s'. Missing result.", Cmd.UUID));
					}
				} else {
					Poco::Thread::trySleep(100);
					WaitTimeInMs -= 100;
				}
			}
			if(WaitTimeInMs<0)
				SetCommandAsTimedOut(Cmd, Request, Response, Handler, Logger);
			else
				SetCommandAsPending(Cmd, Request, Response, Handler, Logger);
		} else {
			SetCommandAsPending(Cmd, Request, Response, Handler, Logger);
		}
	}

}