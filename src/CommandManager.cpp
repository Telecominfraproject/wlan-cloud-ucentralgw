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
#include "framework/uCentral_Protocol.h"

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
						auto Params = P.parse(Cmd.Details).extract<Poco::JSON::Object>();
						auto Result = PostCommandDisk(	Cmd.SerialNumber,
													  	Cmd.Command,
													  	Params,
													  	Cmd.UUID,
													  Sent);
						if(Sent) {
							StorageService()->SetCommandExecuted(Cmd.UUID);
							Logger().information(Poco::format("Sent command '%s' to '%s'",Cmd.Command,Cmd.SerialNumber));
						} else {
							Logger().information(Poco::format("Could not sent command '%s' to '%s'",Cmd.Command,Cmd.SerialNumber));
						}
					} catch (...) {
						Logger().information(Poco::format("Failed to send command '%s' to %s",Cmd.Command,Cmd.SerialNumber));
					}
                }
            }
			Janitor();
        }
    }

    int CommandManager::Start() {
        Logger().notice("Starting...");
        ManagerThread.start(*this);
        return 0;
    }

    void CommandManager::Stop() {
        Logger().notice("Stopping...");
		Running_ = false;
		ManagerThread.wakeUp();
        ManagerThread.join();
    }

    void CommandManager::WakeUp() {
        Logger().notice("Waking up..");
        ManagerThread.wakeUp();
    }

	void CommandManager::Janitor() {
		std::lock_guard G(Mutex_);
		Logger().information("Janitor starting.");
		auto Now = std::chrono::high_resolution_clock::now();
		for(auto i=OutStandingRequests_.begin();i!=OutStandingRequests_.end();) {
			std::chrono::duration<double, std::milli> delta = Now - i->second.submitted;
			if(delta > 120000ms) {
				i = OutStandingRequests_.erase(i);
			} else {
				++i;
			}
		}
		Logger().information("Janitor finished.");
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
		RpcObject			Object;
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
				Poco::format("(%s): Sending command '%s', ID: %lu", SerialNumber, Method, Idx.Id));

			Object.submitted = std::chrono::high_resolution_clock::now();
			Object.uuid = UUID;
			if(disk_only) {
				Object.rpc_entry = nullptr;
			} else {
				Object.rpc_entry = std::make_shared<CommandManager::promise_type_t>();
			}
			OutStandingRequests_[Idx] = Object;
		}

		if(DeviceRegistry()->SendFrame(SerialNumber, ToSend.str())) {
			Sent=true;
			return Object.rpc_entry;
		}
		return nullptr;
	}

	void CommandManager::PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj) {

		if(!Obj->has(uCentralProtocol::ID)){
			Logger().error(Poco::format("(%s): Invalid RPC response.",SerialNumber));
			return;
		}

		uint64_t ID = Obj->get(uCentralProtocol::ID);
		if(ID<2) {
			Logger().error(Poco::format("(%s): Ignoring RPC response.",SerialNumber));
			return;
		}
		std::lock_guard G(Mutex_);
		auto Idx = CommandTagIndex{.Id = ID, .SerialNumber = SerialNumber};
		auto RPC = OutStandingRequests_.find(Idx);
		if (RPC == OutStandingRequests_.end()) {
			Logger().warning(Poco::format("(%s): Outdated RPC %lu", SerialNumber, ID));
			return;
		}
		std::chrono::duration<double, std::milli> rpc_execution_time = std::chrono::high_resolution_clock::now() - RPC->second.submitted;
		StorageService()->CommandCompleted(RPC->second.uuid, Obj, rpc_execution_time, true);
		if(RPC->second.rpc_entry) {
			RPC->second.rpc_entry->set_value(Obj);
		}
		Logger().information(Poco::format("(%s): Received RPC answer %lu", SerialNumber, ID));
	}

}  // namespace