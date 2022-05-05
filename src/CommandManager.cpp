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
		Running_ = true;
		Poco::AutoPtr<Poco::Notification>	NextMsg(ResponseQueue_.waitDequeueNotification());
		while(NextMsg && Running_) {
			auto Resp = dynamic_cast<RPCResponseNotification*>(NextMsg.get());

			if(Resp!= nullptr) {
				const Poco::JSON::Object & Payload = Resp->Payload_;
				const std::string & SerialNumber = Resp->SerialNumber_;

				std::ostringstream SS;
				Payload.stringify(SS);

				// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
				// std::cout << "Got RPC Answer: " << SerialNumber << "   Payload:" << SS.str() << std::endl;
				Logger().debug(fmt::format("({}): RPC Response received.", SerialNumber));
				if(!Payload.has(uCentralProtocol::ID)){
					// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
					Logger().error(fmt::format("({}): Invalid RPC response.", SerialNumber));
				} else {
					uint64_t ID = Payload.get(uCentralProtocol::ID);
					if (ID < 2) {
						// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
						Logger().debug(fmt::format("({}): Ignoring RPC response.", SerialNumber));
					} else {
						auto Idx = CommandTagIndex{.Id = ID, .SerialNumber = SerialNumber};
						std::lock_guard G(Mutex_);
						auto RPC = OutStandingRequests_.find(Idx);
						if (RPC == OutStandingRequests_.end()) {
							// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
							Logger().warning(
								fmt::format("({}): Outdated RPC {}", SerialNumber, ID));
						} else {
							// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
							std::chrono::duration<double, std::milli> rpc_execution_time =
								std::chrono::high_resolution_clock::now() - RPC->second->submitted;
							StorageService()->CommandCompleted(RPC->second->uuid, Payload,
															   rpc_execution_time, true);
							// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
							if (RPC->second->rpc_entry) {
								// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
								RPC->second->rpc_entry->set_value(Payload);
							}
							// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
							OutstandingUUIDs_.erase(RPC->second->uuid);
							OutStandingRequests_.erase(Idx);
							Logger().information(
								fmt::format("({}): Received RPC answer {}", SerialNumber, ID));
							// std::cout << SerialNumber << ": " << __LINE__ << std::endl;
						}
					}
				}
			}
			NextMsg = ResponseQueue_.waitDequeueNotification();
		}
   	}

    int CommandManager::Start() {
        Logger().notice("Starting...");
		ManagerThread.setStackSize(2000000);
		ManagerThread.setName("CMD-MGR");
        ManagerThread.start(*this);
		JanitorCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(*this,&CommandManager::onJanitorTimer);
		JanitorTimer_.setStartInterval( 10000 );
		JanitorTimer_.setPeriodicInterval(10 * 60 * 1000); // 1 hours
		JanitorTimer_.start(*JanitorCallback_);

		CommandRunnerCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(*this,&CommandManager::onCommandRunnerTimer);
		CommandRunnerTimer_.setStartInterval( 10000 );
		CommandRunnerTimer_.setPeriodicInterval(30 * 1000); // 1 hours
		CommandRunnerTimer_.start(*CommandRunnerCallback_);

		// RPCResponseQueue_->Readable_ += Poco::delegate(this,&CommandManager::onRPCAnswer);
		// RPCResponseQueue_->Writable_ += Poco::delegate(this,&CommandManager::onRPCAnswer);
        return 0;
    }

    void CommandManager::Stop() {
        Logger().notice("Stopping...");
		// RPCResponseQueue_->Readable_ -= Poco::delegate(this,&CommandManager::onRPCAnswer);
		// RPCResponseQueue_->Writable_ -= Poco::delegate(this,&CommandManager::onRPCAnswer);
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
		Logger().information(
			fmt::format("Removing expired commands: start. {} outstanding-requests {} outstanding-uuids commands.",
						OutStandingRequests_.size(), OutstandingUUIDs_.size() ));
		auto now = std::chrono::high_resolution_clock::now();
		for(auto i=OutStandingRequests_.begin();i!=OutStandingRequests_.end();) {
			std::chrono::duration<double, std::milli> delta = now - i->second->submitted;
			if(delta > 6000000ms) {
				Logger().debug(fmt::format("{}: Timed out.", i->second->uuid));
				OutstandingUUIDs_.erase(i->second->uuid);
				i = OutStandingRequests_.erase(i);
			} else {
				++i;
			}
		}
		Logger().information("Removing expired commands: done.");
	}

	void CommandManager::onCommandRunnerTimer([[maybe_unused]] Poco::Timer &timer) {
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
					Logger().information(fmt::format("{}-{}: Processing.", Cmd.SerialNumber, Cmd.UUID));
					auto Params = P.parse(Cmd.Details).extract<Poco::JSON::Object::Ptr>();
					auto Result = PostCommandDisk(	Cmd.SerialNumber,
												  Cmd.Command,
												  *Params,
												  Cmd.UUID,
												  Sent);
					if(Sent) {
						StorageService()->SetCommandExecuted(Cmd.UUID);
						OutstandingUUIDs_.insert(Cmd.UUID);
						Logger().information(fmt::format("{}-{}: Sent command {}.", Cmd.SerialNumber, Cmd.UUID, Cmd.Command));
					} else {
						Logger().information(fmt::format("{}-{}: Could not Send command {}.", Cmd.SerialNumber, Cmd.UUID, Cmd.Command));
					}
				} catch (const Poco::Exception &E) {
					Logger().information(fmt::format("{}-{}: Failed command {}.", Cmd.SerialNumber, Cmd.UUID, Cmd.Command));
					Logger().log(E);
					StorageService()->SetCommandExecuted(Cmd.UUID);
				} catch (...) {
					Logger().information(fmt::format("{}-{}: Hard failed command {}.", Cmd.SerialNumber, Cmd.UUID, Cmd.Command));
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
			Logger().information(
				fmt::format("{}-{}: Sending command {}, ID: {}", SerialNumber, UUID, Method, Idx.Id));

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

		if(DeviceRegistry()->SendFrame(SerialNumber, ToSend.str())) {
			Sent=true;
			return Object->rpc_entry;
		}
		return nullptr;
	}
}  // namespace