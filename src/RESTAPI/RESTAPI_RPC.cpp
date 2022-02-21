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

#include "CommandManager.h"
#include "DeviceRegistry.h"
#include "StorageService.h"
#include "framework/ow_constants.h"

namespace OpenWifi::RESTAPI_RPC {
	void SetCommandStatus(GWObjects::CommandDetails &Cmd,
							 Poco::Net::HTTPServerRequest &Request,
							 Poco::Net::HTTPServerResponse &Response,
					  		 RESTAPIHandler *Handler,
					  		 OpenWifi::Storage::CommandExecutionType Status,
							 Poco::Logger &Logger) {
		if (StorageService()->AddCommand(Cmd.SerialNumber, Cmd, Status)) {
			Poco::JSON::Object RetObj;
			Cmd.to_json(RetObj);
			return Handler->ReturnObject(RetObj);
		}
		return Handler->ReturnStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
	}

	void WaitForCommand(GWObjects::CommandDetails &Cmd,
						Poco::JSON::Object  & Params,
						Poco::Net::HTTPServerRequest &Request,
						Poco::Net::HTTPServerResponse &Response,
						std::chrono::milliseconds WaitTimeInMs,
						Poco::JSON::Object * ObjectToReturn,
						RESTAPIHandler * Handler,
						Poco::Logger &Logger) {

		// 	if the command should be executed in the future, or if the device is not connected,
		// 	then we should just add the command to
		//	the DB and let it figure out when to deliver the command.
		if (Cmd.RunAt || !DeviceRegistry()->Connected(Cmd.SerialNumber)) {
			SetCommandStatus(Cmd, Request, Response, Handler, Storage::COMMAND_PENDING, Logger);
			return;
		}

		Cmd.Executed = std::time(nullptr);

		bool Sent;
		std::chrono::time_point<std::chrono::high_resolution_clock> rpc_submitted = std::chrono::high_resolution_clock::now();
		std::shared_ptr<CommandManager::promise_type_t> rpc_endpoint =
			CommandManager()->PostCommand(Cmd.SerialNumber, Cmd.Command, Params, Cmd.UUID, Sent);

		if (Sent && rpc_endpoint!= nullptr) {
			std::future<CommandManager::objtype_t> rpc_future(rpc_endpoint->get_future());
			auto rpc_result = rpc_future.wait_for(WaitTimeInMs);
			if (rpc_result == std::future_status::ready && rpc_future.valid()) {
				std::chrono::duration<double, std::milli> rpc_execution_time = std::chrono::high_resolution_clock::now() - rpc_submitted;
				auto rpc_answer = rpc_future.get();
				if (rpc_answer) {
					if (rpc_answer->has("result") && rpc_answer->isObject("result")) {
						auto ResultFields =
							rpc_answer->get("result").extract<Poco::JSON::Object::Ptr>();
						if (ResultFields->has("status") && ResultFields->isObject("status")) {
							auto StatusInnerObj =
								ResultFields->get("status").extract<Poco::JSON::Object::Ptr>();
							if (StatusInnerObj->has("error"))
								Cmd.ErrorCode = StatusInnerObj->get("error");
							if (StatusInnerObj->has("text"))
								Cmd.ErrorText = StatusInnerObj->get("text").toString();
							std::stringstream ResultText;
							Poco::JSON::Stringifier::stringify(rpc_answer->get("result"),
															   ResultText);
							Cmd.Results = ResultText.str();
							Cmd.Status = "completed";
							Cmd.Completed = time(nullptr);
							Cmd.executionTime = rpc_execution_time.count();

							if (Cmd.ErrorCode && Cmd.Command == uCentralProtocol::TRACE) {
								Cmd.WaitingForFile = 0;
								Cmd.AttachDate = Cmd.AttachSize = 0;
								Cmd.AttachType = "";
							}

							//	Add the completed command to the database...
							StorageService()->AddCommand(Cmd.SerialNumber, Cmd, Storage::COMMAND_COMPLETED);

							if (ObjectToReturn) {
								Handler->ReturnObject(*ObjectToReturn);
							} else {
								Poco::JSON::Object O;
								Cmd.to_json(O);
								Handler->ReturnObject(O);
							}
							Logger.information(Poco::format("Command(%s): completed in %8.3fms.", Cmd.UUID, Cmd.executionTime));
							return;
						} else {
							SetCommandStatus(Cmd, Request, Response, Handler,
											 Storage::COMMAND_FAILED, Logger);
							Logger.information(Poco::format(
								"Invalid response for command '%s'. Missing status.", Cmd.UUID));
							return;
						}
					} else {
						SetCommandStatus(Cmd, Request, Response, Handler, Storage::COMMAND_FAILED,
										 Logger);
						Logger.information(Poco::format(
							"Invalid response for command '%s'. Missing status.", Cmd.UUID));
						return;
					}
				}
			} else if (rpc_result == std::future_status::timeout) {
				SetCommandStatus(Cmd, Request, Response, Handler, Storage::COMMAND_TIMEDOUT,
								 Logger);
			} else {
				SetCommandStatus(Cmd, Request, Response, Handler, Storage::COMMAND_PENDING, Logger);
			}
		} else {
			SetCommandStatus(Cmd, Request, Response, Handler, Storage::COMMAND_PENDING, Logger);
		}
	}
}