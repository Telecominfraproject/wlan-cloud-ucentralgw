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

				Logger().debug(fmt::format("({}): RPC Response received.", SerialNumber));
				if(!Payload.has(uCentralProtocol::ID)){
					Logger().error(fmt::format("({}): Invalid RPC response.", SerialNumber));
				} else {
					uint64_t ID = Payload.get(uCentralProtocol::ID);
					if (ID < 2) {
						Logger().debug(fmt::format("({}): Ignoring RPC response.", SerialNumber));
					} else {
						auto Idx = CommandTagIndex{.Id = ID, .SerialNumber = SerialNumber};
						std::lock_guard G(Mutex_);
						auto RPC = OutStandingRequests_.find(Idx);
						if (RPC == OutStandingRequests_.end()) {
							Logger().warning(
								fmt::format("({}): Outdated RPC {}", SerialNumber, ID));
						} else {
							std::chrono::duration<double, std::milli> rpc_execution_time =
								std::chrono::high_resolution_clock::now() - RPC->second->submitted;
							StorageService()->CommandCompleted(RPC->second->uuid, Payload,
															   rpc_execution_time, true);
							if (RPC->second->rpc_entry) {
								RPC->second->rpc_entry->set_value(Payload);
							}
							OutstandingUUIDs_.erase(RPC->second->uuid);
							OutStandingRequests_.erase(Idx);
							Logger().information(
								fmt::format("({}): Received RPC answer {}", SerialNumber, ID));
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
		for(auto i=OutStandingRequests_.begin();i!=OutStandingRequests_.end();) {
			std::chrono::duration<double, std::milli> delta = now - i->second->submitted;
			if(delta > 6000000ms) {
				MyLogger.debug(fmt::format("{}: Timed out.", i->second->uuid));
				OutstandingUUIDs_.erase(i->second->uuid);
				i = OutStandingRequests_.erase(i);
			} else {
				++i;
			}
		}
		MyLogger.information(
			fmt::format("Removing expired commands: start. {} outstanding-requests {} outstanding-uuids commands.",
						OutStandingRequests_.size(), OutstandingUUIDs_.size() ));
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
						if(OutstandingUUIDs_.find(Cmd.UUID)!=OutstandingUUIDs_.end())
							continue;
					}

					Poco::JSON::Parser	P;
					bool Sent;
					MyLogger.information(fmt::format("{}: Preparing execution of {} for {}.", Cmd.UUID, Cmd.Command, Cmd.SerialNumber));
					auto Params = P.parse(Cmd.Details).extract<Poco::JSON::Object::Ptr>();
					auto Result = PostCommandDisk(	Cmd.SerialNumber,
												  Cmd.Command,
												  *Params,
												  Cmd.UUID,
												  Sent);
					if(Sent) {
						StorageService()->SetCommandExecuted(Cmd.UUID);
						std::lock_guard M(Mutex_);
						OutstandingUUIDs_.insert(Cmd.UUID);
						MyLogger.information(fmt::format("{}: Queued command.", Cmd.UUID));
					} else {
						MyLogger.information(fmt::format("{}: Could queue command.", Cmd.UUID));
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

	std::shared_ptr<CommandManager::promise_type_t> CommandManager::PostCommand(const std::string &SerialNumber,
							  			const std::string &Method,
										const Poco::JSON::Object &Params,
							  			const std::string &UUID,
									 	bool oneway_rpc,
									 	bool disk_only,
										bool & Sent) {

		Sent=false;
		if(!DeviceRegistry()->Connected(SerialNumber)) {
			return nullptr;
		}

		std::stringstream 	ToSend;
		auto Object = std::make_shared<RpcObject>();

		CommandTagIndex 	Idx;
		{
			std::lock_guard M(Mutex_);
			if (oneway_rpc)
				Idx.Id = 1;
			else
				Idx.Id = ++Id_;
			Idx.SerialNumber = SerialNumber;

			Poco::JSON::Object CompleteRPC;
			CompleteRPC.set(uCentralProtocol::JSONRPC, uCentralProtocol::JSONRPC_VERSION);
			CompleteRPC.set(uCentralProtocol::ID, Idx.Id);
			CompleteRPC.set(uCentralProtocol::METHOD, Method);
			CompleteRPC.set(uCentralProtocol::PARAMS, Params);
			Poco::JSON::Stringifier::stringify(CompleteRPC, ToSend);
			Object->submitted = std::chrono::high_resolution_clock::now();
			Object->uuid = UUID;
			if(disk_only) {
				Object->rpc_entry = nullptr;
			} else {
				Object->rpc_entry = std::make_shared<CommandManager::promise_type_t>();
			}
			if(!oneway_rpc) {
				OutStandingRequests_[Idx] = Object;
				OutstandingUUIDs_.insert(UUID);
			}
		}

		Logger().information(fmt::format("{}: Sending command. ID: {}", UUID, Idx.Id));
		if(DeviceRegistry()->SendFrame(SerialNumber, ToSend.str())) {
			Logger().information(fmt::format("{}: Sent command. ID: {}", UUID, Idx.Id));
			Sent=true;
			return Object->rpc_entry;
		}
		Logger().information(fmt::format("{}: Failed to send command. ID: {}", UUID, Idx.Id));
		return nullptr;
	}
}  // namespace