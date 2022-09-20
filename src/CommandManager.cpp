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
#include "DeviceRegistry.h"
#include "StorageService.h"
#include "framework/MicroService.h"
#include "framework/ow_constants.h"

namespace OpenWifi {

	void CommandManager::run() {
		Utils::SetThreadName("cmd:mgr");
		Running_ = true;
		Poco::AutoPtr<Poco::Notification>	NextMsg(ResponseQueue_.waitDequeueNotification());

		while(NextMsg && Running_) {
			auto Resp = dynamic_cast<RPCResponseNotification*>(NextMsg.get());

			if(Resp!= nullptr) {
				const Poco::JSON::Object & Payload = Resp->Payload_;
				const std::string & SerialNumber = Resp->SerialNumber_;

				std::ostringstream SS;
				Payload.stringify(SS);

				if(!Payload.has(uCentralProtocol::ID)){
					Logger().error(fmt::format("({}): Invalid RPC response.", SerialNumber));
				} else {
					uint64_t ID = Payload.get(uCentralProtocol::ID);
					if (ID < 2) {
						Logger().debug(fmt::format("({}): Ignoring RPC response.", SerialNumber));
					} else {
						std::lock_guard G(Mutex_);
						auto RPC = OutStandingRequests_.find(ID);
						if (RPC == OutStandingRequests_.end()	||
							RPC->second.SerialNumber!=Utils::SerialNumberToInt(Resp->SerialNumber_)) {
							Logger().warning(fmt::format("({}): Outdated RPC {}", SerialNumber, ID));
						} else {
							std::chrono::duration<double, std::milli> rpc_execution_time = std::chrono::high_resolution_clock::now() - RPC->second.submitted;
							StorageService()->CommandCompleted(RPC->second.UUID, Payload,
															   rpc_execution_time, true);
							if (RPC->second.rpc_entry) {
								RPC->second.rpc_entry->set_value(Payload);
							}
							Logger().information(
								fmt::format("({}): Received RPC answer {}. Command={}", SerialNumber, ID, RPC->second.Command));
							OutStandingRequests_.erase(ID);
						}
					}
				}
			}
			NextMsg = ResponseQueue_.waitDequeueNotification();
		}
   	}

    int CommandManager::Start() {
        Logger().notice("Starting...");

		ManagerThread.start(*this);

		JanitorCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(*this,&CommandManager::onJanitorTimer);
		JanitorTimer_.setStartInterval( 10000 );
		JanitorTimer_.setPeriodicInterval(10 * 60 * 1000); // 1 hours
		JanitorTimer_.start(*JanitorCallback_, MicroService::instance().TimerPool());

		CommandRunnerCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(*this,&CommandManager::onCommandRunnerTimer);
		CommandRunnerTimer_.setStartInterval( 10000 );
		CommandRunnerTimer_.setPeriodicInterval(30 * 1000); // 1 hours
		CommandRunnerTimer_.start(*CommandRunnerCallback_, MicroService::instance().TimerPool());

        return 0;
    }

    void CommandManager::Stop() {
        Logger().notice("Stopping...");
		Running_ = false;
		JanitorTimer_.stop();
		CommandRunnerTimer_.stop();
		ResponseQueue_.wakeUpAll();
		ManagerThread.wakeUp();
        ManagerThread.join();
		Logger().notice("Stopped...");
    }

    void CommandManager::WakeUp() {
        Logger().notice("Waking up...");
        ManagerThread.wakeUp();
    }

	void CommandManager::onJanitorTimer([[maybe_unused]] Poco::Timer & timer) {
		std::lock_guard G(Mutex_);
		Utils::SetThreadName("cmd:janitor");
		Poco::Logger	& MyLogger = Poco::Logger::get("CMD-MGR-JANITOR");
		auto now = std::chrono::high_resolution_clock::now();
		for(auto request=OutStandingRequests_.begin();request!=OutStandingRequests_.end();) {
			std::chrono::duration<double, std::milli> delta = now - request->second.submitted;
			if(delta > 10min) {
				MyLogger.debug(fmt::format("{}: Command={} for {} Timed out.",
										   request->second.UUID,
										   request->second.Command,
										   Utils::IntToSerialNumber(request->second.SerialNumber)));
				request = OutStandingRequests_.erase(request);
			} else {
				++request;
			}
		}
		MyLogger.information(
			fmt::format("Outstanding-requests {}", OutStandingRequests_.size()));
	}

	void CommandManager::onCommandRunnerTimer([[maybe_unused]] Poco::Timer &timer) {
		Utils::SetThreadName("cmd:schdlr");
		Poco::Logger	& MyLogger = Poco::Logger::get("CMD-MGR-SCHEDULER");

		std::vector<GWObjects::CommandDetails> Commands;
		if(StorageService()->GetReadyToExecuteCommands(0,200,Commands))
		{
			for(auto & Cmd: Commands)
			{
				if(!Running_)
					break;
				try {
					{
						std::lock_guard M(Mutex_);
						for(const auto &request:OutStandingRequests_) {
							if (request.second.UUID == Cmd.UUID) {
								continue;
							}
						}
					}

					auto now = OpenWifi::Now();
					// 2 hour timeout for commands
					if((now-Cmd.Submitted) > (2 * 60 * 60) ) {
						StorageService()->SetCommandTimedOut(Cmd.UUID);
						continue;
					}

					Poco::JSON::Parser	P;
					bool Sent;
					MyLogger.information(fmt::format("{}: Preparing execution of {} for {}.", Cmd.UUID, Cmd.Command, Cmd.SerialNumber));
					auto Params = P.parse(Cmd.Details).extract<Poco::JSON::Object::Ptr>();
					auto Result = PostCommandDisk(	NextRPCId(),
													Cmd.SerialNumber,
												  	Cmd.Command,
												  	*Params,
												  	Cmd.UUID,
												  	Sent);
					if(Sent) {
						StorageService()->SetCommandExecuted(Cmd.UUID);
						MyLogger.information(fmt::format("{}: Queued command. Serial={} Command={}", Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
					} else {
						MyLogger.information(fmt::format("{}: Queued command. Serial={} Command={}", Cmd.UUID, Cmd.SerialNumber, Cmd.Command));
					}
				} catch (const Poco::Exception &E) {
					MyLogger.information(fmt::format("{}: Failed. Command marked as completed.", Cmd.UUID));
					MyLogger.log(E);
					StorageService()->SetCommandExecuted(Cmd.UUID);
				} catch (...) {
					MyLogger.information(fmt::format("{}: Hard failure.", Cmd.UUID));
					StorageService()->SetCommandExecuted(Cmd.UUID);
				}
			}
		}
	}

	std::shared_ptr<CommandManager::promise_type_t> CommandManager::PostCommand(
		uint64_t RPCID,
		const std::string &SerialNumber,
		const std::string &Command,
		const Poco::JSON::Object &Params,
		const std::string &UUID,
		bool oneway_rpc,
		bool disk_only,
		bool & Sent) {

		auto SerialNumberInt = Utils::SerialNumberToInt(SerialNumber);
		Sent=false;
		if(!DeviceRegistry()->Connected(SerialNumberInt)) {
			return nullptr;
		}

		std::stringstream 	ToSend;

		CommandInfo		Idx;
		Idx.Id = oneway_rpc ? 1 : RPCID;
		Idx.SerialNumber = SerialNumberInt;
		Idx.Command = Command;
		Idx.UUID = UUID;

		Poco::JSON::Object CompleteRPC;
		CompleteRPC.set(uCentralProtocol::JSONRPC, uCentralProtocol::JSONRPC_VERSION);
		CompleteRPC.set(uCentralProtocol::ID, RPCID);
		CompleteRPC.set(uCentralProtocol::METHOD, Command);
		CompleteRPC.set(uCentralProtocol::PARAMS, Params);
		Poco::JSON::Stringifier::stringify(CompleteRPC, ToSend);
		Idx.rpc_entry = disk_only ? nullptr : std::make_shared<CommandManager::promise_type_t>();

		Logger().information(fmt::format("{}: Sending command. ID: {}", UUID, RPCID));
		if(DeviceRegistry()->SendFrame(SerialNumber, ToSend.str())) {
			if(!oneway_rpc) {
				std::lock_guard M(Mutex_);
				OutStandingRequests_[RPCID] = Idx;
			}
			Logger().information(fmt::format("{}: Sent command. ID: {}", UUID, RPCID));
			Sent=true;
			return Idx.rpc_entry;
		}

		Logger().information(fmt::format("{}: Failed to send command. ID: {}", UUID, RPCID));
		return nullptr;
	}
}  // namespace