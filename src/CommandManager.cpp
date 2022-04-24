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
#include "RESTObjects//RESTAPI_GWobjects.h"
#include "StorageService.h"
#include "framework/MicroService.h"
#include "framework/ow_constants.h"

namespace OpenWifi {

	void CommandManager::run() {
		Running_ = true;
        while(Running_)
        {
            Poco::Thread::trySleep(30000);
			if(!Running_)
				break;

            std::vector<GWObjects::CommandDetails> Commands;
            if(StorageService()->GetReadyToExecuteCommands(0,200,Commands))
            {
                for(auto & Cmd: Commands)
                {
                	if(!Running_)
                		break;
					try {
						Poco::JSON::Parser	P;
						bool Sent;
						Logger().information(fmt::format("Parsing: {}", Cmd.UUID));
						auto Params = P.parse(Cmd.Details).extract<Poco::JSON::Object::Ptr>();
						Logger().information(fmt::format("Parsed: {}", Cmd.UUID));
						auto Result = PostCommandDisk(	Cmd.SerialNumber,
													  	Cmd.Command,
													  	*Params,
													  	Cmd.UUID,
	  												    Sent);
						if(Sent) {
							StorageService()->SetCommandExecuted(Cmd.UUID);
							Logger().information(fmt::format("{}: Sent command '{}-{}'", Cmd.SerialNumber, Cmd.Command, Cmd.UUID));
						} else {
							Logger().information(fmt::format("{}: Could not send command '{}-{}'", Cmd.SerialNumber, Cmd.Command, Cmd.UUID));
						}
					} catch (const Poco::Exception &E) {
						Logger().information(fmt::format("{}: Failed command '{}-{}'", Cmd.SerialNumber, Cmd.Command, Cmd.UUID));
						Logger().log(E);
						StorageService()->SetCommandExecuted(Cmd.UUID);
					} catch (...) {
						Logger().information(fmt::format("{}: Exception - hard fail - Failed command '{}-{}'", Cmd.SerialNumber, Cmd.Command, Cmd.UUID));
						StorageService()->SetCommandExecuted(Cmd.UUID);
					}
                }
            }
        }
    }

    int CommandManager::Start() {
        Logger().notice("Starting...");
        ManagerThread.start(*this);
		JanitorCallback_ = std::make_unique<Poco::TimerCallback<CommandManager>>(*this,&CommandManager::onTimer);
		Timer_.setStartInterval( 10000 );
		Timer_.setPeriodicInterval(5 * 60 * 1000); // 1 hours
		Timer_.start(*JanitorCallback_);
		RPCResponseQueue_->Readable_ += Poco::delegate(this,&CommandManager::onRPCAnswer);
        return 0;
    }

    void CommandManager::Stop() {
        Logger().notice("Stopping...");
		RPCResponseQueue_->Readable_ -= Poco::delegate(this,&CommandManager::onRPCAnswer);
		Running_ = false;
		Timer_.stop();
		ManagerThread.wakeUp();
        ManagerThread.join();
    }

    void CommandManager::WakeUp() {
        Logger().notice("Waking up...");
        ManagerThread.wakeUp();
    }

	void CommandManager::onTimer([[maybe_unused]] Poco::Timer & timer) {
		std::lock_guard G(Mutex_);
		Logger().information("Removing expired commands: start");
		auto now = std::chrono::high_resolution_clock::now();
		for(auto i=OutStandingRequests_.begin();i!=OutStandingRequests_.end();) {
			std::chrono::duration<double, std::milli> delta = now - i->second->submitted;
			if(delta > 120000ms) {
				i = OutStandingRequests_.erase(i);
			} else {
				++i;
			}
		}
		Logger().information("Removing expired commands: done");
	}

	std::shared_ptr<CommandManager::promise_type_t> CommandManager::PostCommand(	const std::string &SerialNumber,
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
				fmt::format("({}): Sending command '{}', ID: {}", SerialNumber, Method, Idx.Id));

			Object->submitted = std::chrono::high_resolution_clock::now();
			Object->uuid = UUID;
			if(disk_only) {
				Object->rpc_entry = nullptr;
			} else {
				Object->rpc_entry = std::make_shared<CommandManager::promise_type_t>();
			}
			OutStandingRequests_[Idx] = Object;
		}

		if(DeviceRegistry()->SendFrame(SerialNumber, ToSend.str())) {
			Sent=true;
			return Object->rpc_entry;
		}
		return nullptr;
	}

/*
	void CommandManager::PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj) {

		if(!Obj->has(uCentralProtocol::ID)){
			Logger().error(fmt::format("({}): Invalid RPC response.",SerialNumber));
			return;
		}

		uint64_t ID = Obj->get(uCentralProtocol::ID);
		if(ID<2) {
			Logger().error(fmt::format("({}): Ignoring RPC response.",SerialNumber));
			return;
		}
		std::lock_guard G(Mutex_);
		auto Idx = CommandTagIndex{.Id = ID, .SerialNumber = SerialNumber};
		auto RPC = OutStandingRequests_.find(Idx);
		if (RPC == OutStandingRequests_.end()) {
			Logger().warning(fmt::format("({}): Outdated RPC {}", SerialNumber, ID));
			return;
		}
		std::chrono::duration<double, std::milli> rpc_execution_time = std::chrono::high_resolution_clock::now() - RPC->second->submitted;
		StorageService()->CommandCompleted(RPC->second->uuid, Obj, rpc_execution_time, true);
		if(RPC->second->rpc_entry) {
			RPC->second->rpc_entry->set_value(*Obj);
		}
		Logger().information(fmt::format("({}): Received RPC answer {}", SerialNumber, ID));
	}
*/

	void CommandManager::onRPCAnswer(bool &b) {
		std::cout << __LINE__ << std::endl;
		if(b) {
			std::cout << __LINE__ << std::endl;
			RPCResponse Resp;
			std::lock_guard	M(Mutex_);
			auto S = RPCResponseQueue_->Read(Resp);
			const std::string & SerialNumber = Resp.serialNumber;
			std::cout << __LINE__ << std::endl;
			if(S) {
				std::cout << __LINE__ << std::endl;
				if(!Resp.payload.has(uCentralProtocol::ID)){
					std::cout << __LINE__ << std::endl;
					Logger().error(fmt::format("({}): Invalid RPC response.", SerialNumber));
					return;
				}
				std::cout << __LINE__ << std::endl;

				uint64_t ID = Resp.payload.get(uCentralProtocol::ID);
				if(ID<2) {
					std::cout << __LINE__ << std::endl;
					Logger().error(fmt::format("({}): Ignoring RPC response.", SerialNumber));
					return;
				}
				std::cout << __LINE__ << std::endl;
				auto Idx = CommandTagIndex{.Id = ID, .SerialNumber = SerialNumber};
				auto RPC = OutStandingRequests_.find(Idx);
				std::cout << __LINE__ << std::endl;
				if (RPC == OutStandingRequests_.end()) {
					std::cout << __LINE__ << std::endl;
					Logger().warning(fmt::format("({}): Outdated RPC {}", SerialNumber, ID));
					return;
				}
				std::cout << __LINE__ << std::endl;
				std::chrono::duration<double, std::milli> rpc_execution_time = std::chrono::high_resolution_clock::now() - RPC->second->submitted;
				StorageService()->CommandCompleted(RPC->second->uuid, Resp.payload, rpc_execution_time, true);
				if(RPC->second->rpc_entry) {
					std::cout << __LINE__ << std::endl;
					RPC->second->rpc_entry->set_value(Resp.payload);
				}
				Logger().information(fmt::format("({}): Received RPC answer {}", SerialNumber, ID));
				std::cout << __LINE__ << std::endl;
			}
		}
	}

}  // namespace