//
// Created by stephane bourque on 2021-06-28.
//
#include "RESTAPI_RPC.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <iterator>

#include "AP_WS_Server.h"
#include "CommandManager.h"
#include "ParseWifiScan.h"
#include "StorageService.h"
#include "framework/RESTAPI_Handler.h"
#include "framework/ow_constants.h"
#include "framework/utils.h"
#include <GWKafkaEvents.h>

namespace OpenWifi::RESTAPI_RPC {
	void SetCommandStatus(GWObjects::CommandDetails &Cmd,
						  [[maybe_unused]] Poco::Net::HTTPServerRequest &Request,
						  [[maybe_unused]] Poco::Net::HTTPServerResponse &Response,
						  RESTAPIHandler *Handler, OpenWifi::Storage::CommandExecutionType Status,
						  [[maybe_unused]] Poco::Logger &Logger) {
		if (StorageService()->AddCommand(Cmd.SerialNumber, Cmd, Status)) {
			Poco::JSON::Object RetObj;
			Cmd.to_json(RetObj);
			if (Handler != nullptr)
				if (Cmd.ErrorCode){
					return Handler->ReturnObject(RetObj, Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
				}
				return Handler->ReturnObject(RetObj);
			return;
		}
		if (Handler != nullptr)
			return Handler->ReturnStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
	}

	void WaitForCommand(uint64_t RPCID, APCommands::Commands Command, bool RetryLater,
						GWObjects::CommandDetails &Cmd, Poco::JSON::Object &Params,
						Poco::Net::HTTPServerRequest &Request,
						Poco::Net::HTTPServerResponse &Response,
						std::chrono::milliseconds WaitTimeInMs, Poco::JSON::Object *ObjectToReturn,
						RESTAPIHandler *Handler, Poco::Logger &Logger, bool Deferred) {

		Logger.information(fmt::format("{},{}: New {} command. User={} Serial={}. ", Cmd.UUID,
									   RPCID, Cmd.Command, Cmd.SubmittedBy, Cmd.SerialNumber));
		Cmd.Submitted = Utils::Now();
		Cmd.Executed = 0;

		// 	if the command should be executed in the future, or if the device is not connected,
		// 	then we should just add the command to
		//	the DB and let it figure out when to deliver the command.
		auto SerialNumberInt = Utils::SerialNumberToInt(Cmd.SerialNumber);
		if (Cmd.RunAt || (!AP_WS_Server()->Connected(SerialNumberInt) && RetryLater)) {
			Logger.information(fmt::format(
				"{},{}: Command will be run in the future or when device is connected again.",
				Cmd.UUID, RPCID));
			SetCommandStatus(Cmd, Request, Response, Handler,
							 Storage::CommandExecutionType::COMMAND_PENDING, Logger);
			return;
		} else if ((!AP_WS_Server()->Connected(SerialNumberInt) && !RetryLater)) {
			Logger.information(fmt::format(
				"{},{}: Command canceled. Device is not connected. Command will not be retried.",
				Cmd.UUID, RPCID));
			return SetCommandStatus(Cmd, Request, Response, Handler,
									Storage::CommandExecutionType::COMMAND_FAILED, Logger);
		}

		bool Sent;
		std::chrono::time_point<std::chrono::high_resolution_clock> rpc_submitted =
			std::chrono::high_resolution_clock::now();
		std::shared_ptr<CommandManager::promise_type_t> rpc_endpoint =
			CommandManager()->PostCommand(RPCID, Command, Cmd.SerialNumber, Cmd.Command, Params,
										  Cmd.UUID, Sent, true, Deferred);

		if (RetryLater && (!Sent || rpc_endpoint == nullptr)) {
			Logger.information(fmt::format("{},{}: Pending completion. Device is not connected.",
										   Cmd.UUID, RPCID));
			return SetCommandStatus(Cmd, Request, Response, Handler,
									Storage::CommandExecutionType::COMMAND_PENDING, Logger);
		}

		if (!RetryLater && !Sent) {
			Logger.information(fmt::format(
				"{},{}: Command canceled. Device is not connected. Command will not be retried.",
				Cmd.UUID, RPCID));
			return SetCommandStatus(Cmd, Request, Response, Handler,
									Storage::CommandExecutionType::COMMAND_FAILED, Logger);
		}

		Cmd.Executed = Utils::Now();

		Logger.information(fmt::format("{},{}: Command sent.", Cmd.UUID, RPCID));
		std::future<CommandManager::objtype_t> rpc_future(rpc_endpoint->get_future());
		auto rpc_result = rpc_future.wait_for(WaitTimeInMs);
		if (rpc_result == std::future_status::ready) {
			std::chrono::duration<double, std::milli> rpc_execution_time =
				std::chrono::high_resolution_clock::now() - rpc_submitted;
			auto rpc_answer = rpc_future.get();
			if (!rpc_answer->has(uCentralProtocol::RESULT) ||
				!rpc_answer->isObject(uCentralProtocol::RESULT)) {
				SetCommandStatus(Cmd, Request, Response, Handler,
								 Storage::CommandExecutionType::COMMAND_FAILED, Logger);
				Logger.information(
					fmt::format("{},{}: Invalid response. Missing result.", Cmd.UUID, RPCID));
				return;
			}

			auto ResultFields =
				rpc_answer->get(uCentralProtocol::RESULT).extract<Poco::JSON::Object::Ptr>();
			if (!ResultFields->has(uCentralProtocol::STATUS) ||
				!ResultFields->isObject(uCentralProtocol::STATUS)) {
				Cmd.executionTime = rpc_execution_time.count();
				if (Cmd.Command == "ping") {
					SetCommandStatus(Cmd, Request, Response, Handler,
									 Storage::CommandExecutionType::COMMAND_COMPLETED, Logger);
					Logger.information(fmt::format(
						"{},{}: Invalid response from device (ping: fix override). Missing status.",
						Cmd.UUID, RPCID));
				} else {
					SetCommandStatus(Cmd, Request, Response, Handler,
									 Storage::CommandExecutionType::COMMAND_FAILED, Logger);
					Logger.information(fmt::format(
						"{},{}: Invalid response from device. Missing status.", Cmd.UUID, RPCID));
				}
				return;
			}

			std::ostringstream ResultFieldsLog;
			ResultFields->stringify(ResultFieldsLog);
			Logger.debug(
				fmt::format("{},{}: RPC response: {}.", Cmd.UUID, RPCID, ResultFieldsLog.str()));

			auto StatusInnerObj =
				ResultFields->get(uCentralProtocol::STATUS).extract<Poco::JSON::Object::Ptr>();
			if (StatusInnerObj->has(uCentralProtocol::ERROR))
				Cmd.ErrorCode = StatusInnerObj->get(uCentralProtocol::ERROR);
			if (StatusInnerObj->has(uCentralProtocol::TEXT))
				Cmd.ErrorText = StatusInnerObj->get(uCentralProtocol::TEXT).toString();
			std::stringstream ResultText;
			if (rpc_answer->has(uCentralProtocol::RESULT)) {
				if (Cmd.Command == uCentralProtocol::WIFISCAN) {
					auto ScanObj = rpc_answer->get(uCentralProtocol::RESULT)
									   .extract<Poco::JSON::Object::Ptr>();
					ParseWifiScan(ScanObj, ResultText, Logger);
				} else {
					Poco::JSON::Stringifier::stringify(rpc_answer->get(uCentralProtocol::RESULT),
													   ResultText);
				}
			}
			if (rpc_answer->has(uCentralProtocol::RESULT_64)) {
				uint64_t sz = 0;
				if (rpc_answer->has(uCentralProtocol::RESULT_SZ))
					sz = rpc_answer->get(uCentralProtocol::RESULT_SZ);
				std::string UnCompressedData;
				Utils::ExtractBase64CompressedData(
					rpc_answer->get(uCentralProtocol::RESULT_64).toString(), UnCompressedData, sz);
				Poco::JSON::Stringifier::stringify(UnCompressedData, ResultText);
			}
			Cmd.Results = ResultText.str();
			Cmd.Status = "completed";
			Cmd.Completed = Utils::Now();
			Cmd.executionTime = rpc_execution_time.count();

			if (Cmd.ErrorCode && (Cmd.Command == uCentralProtocol::TRACE ||
								  Cmd.Command == uCentralProtocol::SCRIPT)) {
				Cmd.WaitingForFile = 0;
				Cmd.AttachDate = Cmd.AttachSize = 0;
				Cmd.AttachType = "";
			}

			// If the command fails on the device we should show it as failed and not return 200 OK
			if (Cmd.ErrorCode) {
				Logger.information(fmt::format(
				"Command failed with error on device: {}  Reason: {}.",
				Cmd.ErrorCode, Cmd.ErrorText));
				return SetCommandStatus(Cmd, Request, Response, Handler,
								Storage::CommandExecutionType::COMMAND_FAILED, Logger);
			}

			if (Cmd.ErrorCode == 0 && Cmd.Command == uCentralProtocol::CONFIGURE) {
				//	we need to post a kafka event for this.
				if (Params.has(uCentralProtocol::CONFIG) && Params.isObject(uCentralProtocol::CONFIG)) {
					auto Config = Params.get(uCentralProtocol::CONFIG)
									  .extract<Poco::JSON::Object::Ptr>();
					DeviceConfigurationChangeKafkaEvent KEvent(
						Utils::SerialNumberToInt(Cmd.SerialNumber), Utils::Now(),
						Config);
						
				}
			}

			//	Add the completed command to the database...
			StorageService()->AddCommand(Cmd.SerialNumber, Cmd,
										 Storage::CommandExecutionType::COMMAND_COMPLETED);

			if (ObjectToReturn && Handler) {
				Handler->ReturnObject(*ObjectToReturn);
			} else {
				Poco::JSON::Object O;
				Cmd.to_json(O);
				if (Handler)
					Handler->ReturnObject(O);
			}
			Logger.information(
				fmt::format("{},{}: Completed in {:.3f}ms.", Cmd.UUID, RPCID, Cmd.executionTime));
			return;
		}
		CommandManager()->RemovePendingCommand(RPCID);
		if (RetryLater) {
			Logger.information(fmt::format("{},{}: Pending completion.", Cmd.UUID, RPCID));
			SetCommandStatus(Cmd, Request, Response, Handler,
							 Storage::CommandExecutionType::COMMAND_PENDING, Logger);
		} else {
			Logger.information(fmt::format(
				"{},{}: Command canceled. Device is not connected. Command will not be retried.",
				Cmd.UUID, RPCID));
			return SetCommandStatus(Cmd, Request, Response, Handler,
									Storage::CommandExecutionType::COMMAND_FAILED, Logger);
		}
	}
} // namespace OpenWifi::RESTAPI_RPC
