//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <algorithm>

#include "Poco/JSON/Parser.h"

#include "AP_WS_Server.h"
#include "CommandManager.h"
#include "StorageService.h"
#include "framework/MicroServiceFuncs.h"
#include "framework/ow_constants.h"
#include "framework/utils.h"

using namespace std::chrono_literals;

namespace OpenWifi {

	void CommandManager::run() {
		Utils::SetThreadName("cmd:mgr");
		Running_ = true;

		Poco::AutoPtr<Poco::Notification> NextMsg(ResponseQueue_.waitDequeueNotification());
		while (NextMsg && Running_) {
			auto Resp = dynamic_cast<RPCResponseNotification *>(NextMsg.get());

			try {
				if (Resp != nullptr) {
					Poco::JSON::Object::Ptr Payload = Resp->Payload_;
					std::string SerialNumberStr = Utils::IntToSerialNumber(Resp->SerialNumber_);

					if (!Payload->has(uCentralProtocol::ID)) {
						poco_error(Logger(),
								   fmt::format("({}): Invalid RPC response.", SerialNumberStr));
					} else {
						uint64_t ID = Payload->get(uCentralProtocol::ID);
						if (ID > 1) {
							poco_debug(Logger(), fmt::format("({}): Processing {} response.",
															 SerialNumberStr, ID));
							std::lock_guard Lock(LocalMutex_);
							auto RPC = OutStandingRequests_.find(ID);
							if (RPC == OutStandingRequests_.end()) {
								poco_debug(Logger(), fmt::format("({}): RPC {} cannot be found.",
																 SerialNumberStr, ID));
							} else if (RPC->second.SerialNumber != Resp->SerialNumber_) {
								poco_debug(
									Logger(),
									fmt::format("({}): RPC {} serial number mismatch {}!={}.",
												SerialNumberStr, ID, RPC->second.SerialNumber,
												Resp->SerialNumber_));
							} else {
								std::shared_ptr<promise_type_t> TmpRpcEntry;
								std::chrono::duration<double, std::milli> rpc_execution_time =
									std::chrono::high_resolution_clock::now() -
									RPC->second.submitted;
								poco_debug(Logger(),
										   fmt::format("({}): Received RPC answer {}. Command={}",
													   SerialNumberStr, ID,
													   APCommands::to_string(RPC->second.Command)));
								if (RPC->second.Command == APCommands::Commands::script) {
									CompleteScriptCommand(RPC->second, Payload, rpc_execution_time);
								} else if (RPC->second.Command == APCommands::Commands::telemetry) {
									CompleteTelemetryCommand(RPC->second, Payload,
															 rpc_execution_time);
								} else if (RPC->second.Command == APCommands::Commands::configure && RPC->second.rpc_entry==nullptr) {
									CompleteConfigureCommand(RPC->second, Payload,
															 rpc_execution_time);
								} else {
									StorageService()->CommandCompleted(RPC->second.UUID, Payload,
																	   rpc_execution_time, true);
									if (RPC->second.rpc_entry) {
										TmpRpcEntry = RPC->second.rpc_entry;
									}
									RPC->second.State = 0;
									OutStandingRequests_.erase(ID);
									if (TmpRpcEntry != nullptr)
										TmpRpcEntry->set_value(Payload);
								}
							}
						}
					}
				}
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			} catch (...) {
				poco_warning(Logger(), "Exception occurred during run.");
			}
			NextMsg = ResponseQueue_.waitDequeueNotification();
		}
		poco_information(Logger(), "RPC Command processor stopping.");
	}

	bool CommandManager::CompleteTelemetryCommand(
		CommandInfo &Command, [[maybe_unused]] const Poco::JSON::Object::Ptr &Payload,
		std::chrono::duration<double, std::milli> rpc_execution_time) {
		std::shared_ptr<promise_type_t> TmpRpcEntry;

		StorageService()->CommandCompleted(Command.UUID, Payload, rpc_execution_time, true);

		if (Command.rpc_entry) {
			TmpRpcEntry = Command.rpc_entry;
		}
		Command.State = 0;

		OutStandingRequests_.erase(Command.Id);
		if (TmpRpcEntry != nullptr)
			TmpRpcEntry->set_value(Payload);
		return true;
	}

	bool CommandManager::CompleteConfigureCommand(
		CommandInfo &Command, [[maybe_unused]] const Poco::JSON::Object::Ptr &Payload,
		std::chrono::duration<double, std::milli> rpc_execution_time) {
		std::shared_ptr<promise_type_t> TmpRpcEntry;

		if (Command.rpc_entry) {
			TmpRpcEntry = Command.rpc_entry;
		}

		StorageService()->CommandCompleted(Command.UUID, Payload, rpc_execution_time, true);

		if (Payload->has("result")) {
			auto Result = Payload->getObject("result");
			if (Result->has("status") && Result->has("serial")) {
				auto Status = Result->getObject("status");
				auto SerialNumber = Result->get("serial").toString();
				std::uint64_t Error = Status->get("error");
				if (Error == 2) {
					StorageService()->RollbackDeviceConfigurationChange(SerialNumber);
				} else {
					StorageService()->CompleteDeviceConfigurationChange(SerialNumber);
				}
			}
		} else {
		}
		Command.State = 0;

		if (Command.rpc_entry) {
			TmpRpcEntry = Command.rpc_entry;
		}

		OutStandingRequests_.erase(Command.Id);
		if (TmpRpcEntry != nullptr)
			TmpRpcEntry->set_value(Payload);
		return true;
	}

	bool CommandManager::CompleteScriptCommand(
		CommandInfo &Command, const Poco::JSON::Object::Ptr &Payload,
		std::chrono::duration<double, std::milli> rpc_execution_time) {
		bool Reply = true;
		std::shared_ptr<promise_type_t> TmpRpcEntry;

		if (Command.rpc_entry) {
			TmpRpcEntry = Command.rpc_entry;
		}
		if (Command.State == 2) {
			//	 look at the payload to see if we should continue or not...
			if (Payload->has("result")) {
				auto Result = Payload->getObject("result");
				if (Result->has("status")) {
					auto Status = Result->getObject("status");

					std::uint64_t Error = Status->get("error");
					if (Error == 0) {
						StorageService()->CommandCompleted(Command.UUID, Payload,
														   rpc_execution_time, true);
						Command.State = 1;
					} else {
						StorageService()->CommandCompleted(Command.UUID, Payload,
														   rpc_execution_time, true);
						std::string ErrorTxt = Status->get("result");
						StorageService()->CancelWaitFile(Command.UUID, ErrorTxt);
						Command.State = 0;
					}
				} else {
				}
			} else {
				Command.State = 0;
			}
		} else if (Command.State == 1) {
			StorageService()->CommandCompleted(Command.UUID, Payload, rpc_execution_time, true);
			if (Command.Deferred) {
				Reply = false;
			}
			Command.State = 0;
		}

		if (Command.State == 0) {
			OutStandingRequests_.erase(Command.Id);
		}
		if (Reply && TmpRpcEntry != nullptr)
			TmpRpcEntry->set_value(Payload);

		return true;
	}

	int CommandManager::Start() {
		poco_notice(Logger(), "Starting...");

		commandTimeOut_ = MicroServiceConfigGetInt("command.timeout", 4 * 60 * 60);
		commandRetry_ = MicroServiceConfigGetInt("command.retry", 120);
		janitorInterval_ = MicroServiceConfigGetInt("command.janitor", 2 * 60); //	1 hour
		queueInterval_ = MicroServiceConfigGetInt("command.queue", 30);

		ManagerThread.start(*this);

		JanitorCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(
			*this, &CommandManager::onJanitorTimer);
		JanitorTimer_.setStartInterval(10000);
		JanitorTimer_.setPeriodicInterval(janitorInterval_ * 1000); // 1 hours
		JanitorTimer_.start(*JanitorCallback_, MicroServiceTimerPool());

		CommandRunnerCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(
			*this, &CommandManager::onCommandRunnerTimer);
		CommandRunnerTimer_.setStartInterval(10000);
		CommandRunnerTimer_.setPeriodicInterval(queueInterval_ * 1000); // 1 hours
		CommandRunnerTimer_.start(*CommandRunnerCallback_, MicroServiceTimerPool());

		return 0;
	}

	void CommandManager::Stop() {
		poco_notice(Logger(), "Stopping...");
		Running_ = false;
		JanitorTimer_.stop();
		CommandRunnerTimer_.stop();
		ResponseQueue_.wakeUpAll();
		ManagerThread.wakeUp();
		ManagerThread.join();
		poco_notice(Logger(), "Stopped...");
	}

	void CommandManager::WakeUp() {
		poco_notice(Logger(), "Waking up...");
		ManagerThread.wakeUp();
	}

	void CommandManager::onJanitorTimer([[maybe_unused]] Poco::Timer &timer) {
		std::lock_guard Lock(LocalMutex_);
		Utils::SetThreadName("cmd:janitor");
		Poco::Logger &MyLogger = Poco::Logger::get("CMD-MGR-JANITOR");
		std::string TimeOutError("No response.");

		auto now = std::chrono::high_resolution_clock::now();
		for (auto request = OutStandingRequests_.begin(); request != OutStandingRequests_.end();) {
			std::chrono::duration<double, std::milli> delta = now - request->second.submitted;
			if (delta > 10min) {
				MyLogger.debug(fmt::format("{}: Command={} for {} Timed out.", request->second.UUID,
										   APCommands::to_string(request->second.Command),
										   Utils::IntToSerialNumber(request->second.SerialNumber)));
				if ((request->second.Command == APCommands::Commands::script &&
					 request->second.Deferred) ||
					(request->second.Command == APCommands::Commands::trace)) {
					StorageService()->CancelWaitFile(request->second.UUID, TimeOutError);
				}
				StorageService()->SetCommandTimedOut(request->second.UUID);
				request = OutStandingRequests_.erase(request);
			} else {
				++request;
			}
		}
		poco_information(MyLogger,
						 fmt::format("Outstanding-requests {}", OutStandingRequests_.size()));
	}

	bool CommandManager::IsCommandRunning(const std::string &C) {
		std::lock_guard Lock(LocalMutex_);
		return std::any_of(
			OutStandingRequests_.begin(), OutStandingRequests_.end(),
			[C](const std::pair<std::uint64_t, CommandInfo> &r) { return r.second.UUID == C; });
	}

	void CommandManager::onCommandRunnerTimer([[maybe_unused]] Poco::Timer &timer) {
		Utils::SetThreadName("cmd:schdlr");
		Poco::Logger &MyLogger = Poco::Logger::get("CMD-MGR-SCHEDULER");

		poco_trace(MyLogger, "Scheduler starting.");

		try {

			StorageService()->RemovedExpiredCommands();
			StorageService()->RemoveTimedOutCommands();

			std::uint64_t offset = 0;
			bool Done = false;
			while (!Done) {
				std::vector<GWObjects::CommandDetails> Commands;
				if (StorageService()->GetReadyToExecuteCommands(offset, 200, Commands)) {
					if(Commands.empty()) {
						Done=true;
						continue;
					}
					poco_trace(MyLogger, fmt::format("Scheduler about to process {} commands.",
													 Commands.size()));
					for (auto &Cmd : Commands) {
						if (!Running_) {
							poco_warning(MyLogger,
										 "Scheduler quitting because service is stopping.");
							break;
						}
						poco_trace(MyLogger,
								   fmt::format("{}: Serial={} Command={} Starting processing.",
											   Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
						try {

							//	Skip an already running command
							if (IsCommandRunning(Cmd.UUID)) {
								continue;
							}

							auto now = Utils::Now();
							// 2 hour timeout for commands
							if ((now - Cmd.Submitted) > commandTimeOut_) {
								poco_information(
									MyLogger, fmt::format("{}: Serial={} Command={} has expired.",
														  Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
								StorageService()->SetCommandTimedOut(Cmd.UUID);
								continue;
							}

							auto SerialNumberInt = Utils::SerialNumberToInt(Cmd.SerialNumber);
							if (!AP_WS_Server()->Connected(SerialNumberInt)) {
								poco_trace(
									MyLogger,
									fmt::format("{}: Serial={} Command={} Device is not connected.",
												Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
								StorageService()->SetCommandLastTry(Cmd.UUID);
								continue;
							}

							std::string ExecutingUUID;
							APCommands::Commands ExecutingCommand = APCommands::Commands::unknown;
							if (CommandRunningForDevice(SerialNumberInt, ExecutingUUID,
														ExecutingCommand)) {
								poco_trace(
									MyLogger,
									fmt::format("{}: Serial={} Command={} Device is already busy "
												"with command {} (Command={}).",
												Cmd.UUID, Cmd.SerialNumber, Cmd.Command,
												ExecutingUUID,
												APCommands::to_string(ExecutingCommand)));
								continue;
							}

							Poco::JSON::Parser P;
							bool Sent;
							poco_information(
								MyLogger,
								fmt::format("{}: Serial={} Command={} Preparing execution.",
											Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
							auto Params = P.parse(Cmd.Details).extract<Poco::JSON::Object::Ptr>();
							auto Result = PostCommandDisk(
								Next_RPC_ID(), APCommands::to_apcommand(Cmd.Command.c_str()),
								Cmd.SerialNumber, Cmd.Command, *Params, Cmd.UUID, Sent);
							if (Sent) {
								StorageService()->SetCommandExecuted(Cmd.UUID);
								poco_debug(MyLogger,
										   fmt::format("{}: Serial={} Command={} Sent.", Cmd.UUID,
													   Cmd.SerialNumber, Cmd.Command));
							} else {
								poco_debug(
									MyLogger,
									fmt::format("{}: Serial={} Command={} Re-queued command.",
												Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
								StorageService()->SetCommandLastTry(Cmd.UUID);
							}
						} catch (const Poco::Exception &E) {
							poco_debug(
								MyLogger,
								fmt::format(
									"{}: Serial={} Command={} Failed. Command marked as completed.",
									Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
							MyLogger.log(E);
							StorageService()->SetCommandExecuted(Cmd.UUID);
						} catch (...) {
							poco_debug(MyLogger,
									   fmt::format("{}: Serial={} Command={} Hard failure. "
												   "Command marked as completed.",
												   Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
							StorageService()->SetCommandExecuted(Cmd.UUID);
						}
					}
					offset += Commands.size();
				} else {
					Done=true;
					continue;
				}
			}
		}
		catch (Poco::Exception &E) {
			MyLogger.log(E);
		}
		catch (...) {
			poco_warning(MyLogger, "Exception during command processing.");
		}
		poco_trace(MyLogger, "Scheduler done.");
	}

	std::shared_ptr<CommandManager::promise_type_t> CommandManager::PostCommand(
		uint64_t RPC_ID, APCommands::Commands Command, const std::string &SerialNumber,
		const std::string &CommandStr, const Poco::JSON::Object &Params, const std::string &UUID,
		bool oneway_rpc, [[maybe_unused]] bool disk_only, bool &Sent, bool rpc, bool Deferred) {

		auto SerialNumberInt = Utils::SerialNumberToInt(SerialNumber);
		Sent = false;

		std::stringstream ToSend;

		CommandInfo CInfo;
		CInfo.Id = oneway_rpc ? 1 : RPC_ID;
		CInfo.SerialNumber = SerialNumberInt;
		CInfo.Command = Command;
		CInfo.Deferred = Deferred;
		CInfo.UUID = UUID;
		if (Command == APCommands::Commands::script && Deferred) {
			CInfo.State = 2;
		} else {
			CInfo.State = 1;
		}

		Poco::JSON::Object CompleteRPC;
		CompleteRPC.set(uCentralProtocol::JSONRPC, uCentralProtocol::JSONRPC_VERSION);
		CompleteRPC.set(uCentralProtocol::ID, RPC_ID);
		CompleteRPC.set(uCentralProtocol::METHOD, CommandStr);
		CompleteRPC.set(uCentralProtocol::PARAMS, Params);
		Poco::JSON::Stringifier::stringify(CompleteRPC, ToSend);
		CInfo.rpc_entry = rpc ? std::make_shared<CommandManager::promise_type_t>() : nullptr;

		poco_debug(Logger(), fmt::format("{}: Sending command {} to {}. ID: {}", UUID, CommandStr,
										 SerialNumber, RPC_ID));
		//	Do not change the order. It is possible that an RPC completes before it is entered in
		// the map. So we insert it 	first, even if we may need to remove it later upon failure.
		if (!oneway_rpc) {
			std::lock_guard M(Mutex_);
			OutStandingRequests_[RPC_ID] = CInfo;
		}
		if (AP_WS_Server()->SendFrame(SerialNumber, ToSend.str())) {
			poco_debug(Logger(), fmt::format("{}: Sent command. ID: {}", UUID, RPC_ID));
			Sent = true;
			return CInfo.rpc_entry;
		} else if (!oneway_rpc) {
			std::lock_guard M(Mutex_);
			OutStandingRequests_.erase(RPC_ID);
		}

		poco_warning(Logger(), fmt::format("{}: Failed to send command. ID: {}", UUID, RPC_ID));
		return nullptr;
	}

	bool CommandManager::FireAndForget(const std::string &SerialNumber, const std::string &Method, const Poco::JSON::Object &Params) {
		Poco::JSON::Object CompleteRPC;
		CompleteRPC.set(uCentralProtocol::JSONRPC, uCentralProtocol::JSONRPC_VERSION);
		CompleteRPC.set(uCentralProtocol::ID, 0);
		CompleteRPC.set(uCentralProtocol::METHOD, Method);
		CompleteRPC.set(uCentralProtocol::PARAMS, Params);
		std::stringstream ToSend;
		CompleteRPC.stringify(ToSend);
		poco_debug(Logger(), fmt::format("{}: Fire and forget command {}.", SerialNumber, Method));
		return AP_WS_Server()->SendFrame(SerialNumber, ToSend.str())>0;
	}
} // namespace OpenWifi