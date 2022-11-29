//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <algorithm>

#include "Poco/JSON/Parser.h"

#include "CommandManager.h"
#include "AP_WS_Server.h"
#include "StorageService.h"
#include "framework/ow_constants.h"
#include "framework/MicroServiceFuncs.h"
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

					std::ostringstream SS;
					Payload->stringify(SS);

					if (!Payload->has(uCentralProtocol::ID)) {
						poco_error(Logger(), fmt::format("({}): Invalid RPC response.", SerialNumberStr));
					} else {
						uint64_t ID = Payload->get(uCentralProtocol::ID);
						poco_debug(Logger(),fmt::format("({}): Processing {} response.", SerialNumberStr, ID));
						if (ID > 1) {
							std::lock_guard	Lock(LocalMutex_);
							auto RPC = OutStandingRequests_.find(ID);
							if (RPC == OutStandingRequests_.end() ||
								RPC->second.SerialNumber != Resp->SerialNumber_) {
								poco_debug(Logger(),
									fmt::format("({}): RPC {} completed.", SerialNumberStr, ID));
							} else {
								std::chrono::duration<double, std::milli> rpc_execution_time =
									std::chrono::high_resolution_clock::now() -
									RPC->second.submitted;
								StorageService()->CommandCompleted(RPC->second.UUID, Payload,
																   rpc_execution_time, true);
								if (RPC->second.rpc_entry) {
									RPC->second.rpc_entry->set_value(Payload);
								}
								poco_debug(Logger(),
									fmt::format("({}): Received RPC answer {}. Command={}",
													   SerialNumberStr, ID, APCommands::to_string(RPC->second.Command)));
								if(RPC->second.Command==APCommands::Commands::script) {

								}
								RPC->second.State--;
								if(RPC->second.State==0)
									OutStandingRequests_.erase(ID);
							}
						}
					}
				}
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			} catch (...) {
				poco_warning(Logger(),"Exception occurred during run.");
			}
			NextMsg = ResponseQueue_.waitDequeueNotification();
		}
		poco_information(Logger(),"RPC Command processor stopping.");
   	}

    int CommandManager::Start() {
        poco_notice(Logger(),"Starting...");

		ManagerThread.start(*this);

		JanitorCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(*this,&CommandManager::onJanitorTimer);
		JanitorTimer_.setStartInterval( 10000 );
		JanitorTimer_.setPeriodicInterval(10 * 60 * 1000); // 1 hours
		JanitorTimer_.start(*JanitorCallback_, MicroServiceTimerPool());

		CommandRunnerCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(*this,&CommandManager::onCommandRunnerTimer);
		CommandRunnerTimer_.setStartInterval( 10000 );
		CommandRunnerTimer_.setPeriodicInterval(30 * 1000); // 1 hours
		CommandRunnerTimer_.start(*CommandRunnerCallback_, MicroServiceTimerPool());

        return 0;
    }

    void CommandManager::Stop() {
        poco_notice(Logger(),"Stopping...");
		Running_ = false;
		JanitorTimer_.stop();
		CommandRunnerTimer_.stop();
		ResponseQueue_.wakeUpAll();
		ManagerThread.wakeUp();
        ManagerThread.join();
		poco_notice(Logger(),"Stopped...");
    }

    void CommandManager::WakeUp() {
		poco_notice(Logger(),"Waking up...");
        ManagerThread.wakeUp();
    }

	void CommandManager::onJanitorTimer([[maybe_unused]] Poco::Timer & timer) {
		std::lock_guard	Lock(LocalMutex_);
		Utils::SetThreadName("cmd:janitor");
		Poco::Logger	& MyLogger = Poco::Logger::get("CMD-MGR-JANITOR");
		auto now = std::chrono::high_resolution_clock::now();
		for(auto request=OutStandingRequests_.begin();request!=OutStandingRequests_.end();) {
			std::chrono::duration<double, std::milli> delta = now - request->second.submitted;
			if(delta > 10min) {
				MyLogger.debug(fmt::format("{}: Command={} for {} Timed out.",
										   request->second.UUID,
										   APCommands::to_string(request->second.Command),
										   Utils::IntToSerialNumber(request->second.SerialNumber)));
				request = OutStandingRequests_.erase(request);
			} else {
				++request;
			}
		}
		poco_information(MyLogger,
			fmt::format("Outstanding-requests {}", OutStandingRequests_.size()));
	}

	bool CommandManager::IsCommandRunning(const std::string &C) {
		std::lock_guard	Lock(LocalMutex_);
		for (const auto &request : OutStandingRequests_) {
			if (request.second.UUID == C) {
				return true;
			}
		}
		return false;
	}

	void CommandManager::onCommandRunnerTimer([[maybe_unused]] Poco::Timer &timer) {
		Utils::SetThreadName("cmd:schdlr");
		Poco::Logger &MyLogger = Poco::Logger::get("CMD-MGR-SCHEDULER");

		poco_trace(MyLogger,"Scheduler starting.");

		try {

			StorageService()->RemovedExpiredCommands();
			StorageService()->RemoveTimedOutCommands();

			std::vector<GWObjects::CommandDetails> Commands;
			if (StorageService()->GetReadyToExecuteCommands(0, 200, Commands)) {
				poco_trace(MyLogger,fmt::format("Scheduler about to process {} commands.", Commands.size()));
				for (auto &Cmd : Commands) {
					if (!Running_) {
						poco_warning(MyLogger,"Scheduler quitting because service is stopping.");
						break;
					}
					poco_trace(
						MyLogger, fmt::format("{}: Serial={} Command={} Starting processing.",
											  Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
					try {

						//	Skip an already running command
						if(IsCommandRunning(Cmd.UUID))
							continue;

						auto now = Utils::Now();
						// 2 hour timeout for commands
						if ((now - Cmd.Submitted) > (1 * 60 * 60)) {
							poco_information(
								MyLogger, fmt::format("{}: Serial={} Command={} has expired.",
													  Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
							StorageService()->SetCommandTimedOut(Cmd.UUID);
							continue;
						}

						if (!AP_WS_Server()->Connected(
								Utils::SerialNumberToInt(Cmd.SerialNumber))) {
							poco_trace(
								MyLogger,
								fmt::format(
									"{}: Serial={} Command={} Device is not connected.",
									Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
							continue;
						}

						std::string 			ExecutingUUID;
						APCommands::Commands	ExecutingCommand=APCommands::Commands::unknown;
						if (CommandRunningForDevice(Utils::SerialNumberToInt(Cmd.SerialNumber),
													ExecutingUUID, ExecutingCommand)) {
							poco_trace(
								MyLogger,
								fmt::format(
									"{}: Serial={} Command={} Device is already busy with command {} (Command={})."
									, Cmd.UUID, Cmd.SerialNumber, Cmd.Command, ExecutingUUID, APCommands::to_string(ExecutingCommand)));
							continue;
						}

						Poco::JSON::Parser P;
						bool Sent;
						poco_information(MyLogger, fmt::format("{}: Serial={} Command={} Preparing execution.",
														 Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
						auto Params = P.parse(Cmd.Details).extract<Poco::JSON::Object::Ptr>();
						auto Result = PostCommandDisk(Next_RPC_ID(), APCommands::to_apcommand(Cmd.Command.c_str()), Cmd.SerialNumber, Cmd.Command,
													  *Params, Cmd.UUID, Sent);
						if (Sent) {
							StorageService()->SetCommandExecuted(Cmd.UUID);
							poco_debug(MyLogger,
								fmt::format("{}: Serial={} Command={} Sent.",
									 Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
						} else {
							poco_debug(MyLogger,
								fmt::format("{}: Serial={} Command={} Re-queued command.",
									 Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
						}
					} catch (const Poco::Exception &E) {
						poco_debug(MyLogger,
							fmt::format("{}: Serial={} Command={} Failed. Command marked as completed.",
									 Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
						MyLogger.log(E);
						StorageService()->SetCommandExecuted(Cmd.UUID);
					} catch (...) {
						poco_debug(MyLogger,
							 fmt::format("{}: Serial={} Command={} Hard failure. Command marked as completed.",
										 Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
						StorageService()->SetCommandExecuted(Cmd.UUID);
					}
				}
			}
		} catch (Poco::Exception &E) {
			MyLogger.log(E);
		} catch (...) {
			poco_warning(MyLogger,"Exception during command processing.");
		}
		poco_trace(MyLogger,"Scheduler done.");
	}

	std::shared_ptr<CommandManager::promise_type_t> CommandManager::PostCommand(
		uint64_t RPC_ID,
		APCommands::Commands Command,
		const std::string &SerialNumber,
		const std::string &CommandStr,
		const Poco::JSON::Object &Params,
		const std::string &UUID,
		bool oneway_rpc,
		bool disk_only,
		bool & Sent) {

		auto SerialNumberInt = Utils::SerialNumberToInt(SerialNumber);
		Sent=false;

		std::stringstream 	ToSend;

		CommandInfo		Idx;
		Idx.Id = oneway_rpc ? 1 : RPC_ID;
		Idx.SerialNumber = SerialNumberInt;
		Idx.Command = Command;
		if(Command == APCommands::Commands::script)
			Idx.State=2;
		Idx.UUID = UUID;

		Poco::JSON::Object CompleteRPC;
		CompleteRPC.set(uCentralProtocol::JSONRPC, uCentralProtocol::JSONRPC_VERSION);
		CompleteRPC.set(uCentralProtocol::ID, RPC_ID);
		CompleteRPC.set(uCentralProtocol::METHOD, Command);
		CompleteRPC.set(uCentralProtocol::PARAMS, Params);
		Poco::JSON::Stringifier::stringify(CompleteRPC, ToSend);
		Idx.rpc_entry = disk_only ? nullptr : std::make_shared<CommandManager::promise_type_t>();

		poco_debug(Logger(), fmt::format("{}: Sending command {} to {}. ID: {}", UUID, CommandStr, SerialNumber, RPC_ID));
		if(AP_WS_Server()->SendFrame(SerialNumber, ToSend.str())) {
			if(!oneway_rpc) {
				std::lock_guard M(Mutex_);
				OutStandingRequests_[RPC_ID] = Idx;
			}
			poco_debug(Logger(), fmt::format("{}: Sent command. ID: {}", UUID, RPC_ID));
			Sent=true;
			return Idx.rpc_entry;
		}

		poco_warning(Logger(), fmt::format("{}: Failed to send command. ID: {}", UUID, RPC_ID));
		return nullptr;
	}
}  // namespace