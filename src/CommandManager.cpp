//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "CommandManager.h"
#include <algorithm>

#include "CommandManager.h"
#include "DeviceRegistry.h"
#include "RESTAPI_GWobjects.h"
#include "RESTAPI_handler.h"
#include "StorageService.h"
#include "uCentralProtocol.h"

#include "Poco/JSON/Parser.h"

namespace OpenWifi {

    class CommandManager * CommandManager::instance_ = nullptr;

	CommandManager::CommandManager() noexcept:
		SubSystemServer("CommandManager", "CMD_MGR", "command.manager")
    {
    }

	void CommandManager::run() {
		Running_ = true;
        while(Running_)
        {
            Poco::Thread::trySleep(30000);
			if(!Running_)
				break;
            std::vector<GWObjects::CommandDetails> Commands;

            if(Storage()->GetReadyToExecuteCommands(1,200,Commands))
            {
                for(auto & Cmd: Commands)
                {
                	if(!Running_)
                		break;

					uint64_t RPC_Id;
					Poco::JSON::Parser	P;

					auto Params = P.parse(Cmd.Details).extract<Poco::JSON::Object::Ptr>();
					if(SendCommand(	Cmd.SerialNumber,
									Cmd.Command,
									*Params,
									Cmd.UUID,
									RPC_Id)) {
						Storage()->SetCommandExecuted(Cmd.UUID);
						Logger_.information(Poco::format("Sent command '%s' to '%s'",Cmd.Command,Cmd.SerialNumber));
					} else {
                        Logger_.information(Poco::format("Failed to send command '%s' to %s",Cmd.Command,Cmd.SerialNumber));
                    }
                }
            }
			Janitor();
        }
    }

    int CommandManager::Start() {
        Logger_.notice("Starting...");
        ManagerThread.start(*this);
        return 0;
    }

    void CommandManager::Stop() {
        Logger_.notice("Stopping...");
		Running_ = false;
		ManagerThread.wakeUp();
        ManagerThread.join();
    }

    void CommandManager::WakeUp() {
        Logger_.notice("Waking up..");
        ManagerThread.wakeUp();
    }

	void CommandManager::Janitor() {
		std::lock_guard G(Mutex_);
		uint64_t Now = time(nullptr);

		for(auto i=OutStandingRequests_.begin();i!=OutStandingRequests_.end();) {
			if((Now-i->second.Submitted)>120)
				i = OutStandingRequests_.erase(i);
			else
				++i;
		}
	}

	bool CommandManager::GetCommand(uint64_t Id, const std::string &SerialNumber, CommandTag &T) {
		std::lock_guard G(Mutex_);

		std::cout << "Looking for " << Id << " from " << SerialNumber << std::endl;

		CommandTagIndex	TI{.Id=Id,.SerialNumber=SerialNumber};
		auto Hint=OutStandingRequests_.find(TI);
		if(Hint!=OutStandingRequests_.end()) {
			if(Hint->second.Completed) {
				T = Hint->second;
				OutStandingRequests_.erase(Hint);
				std::cout << "Returning Command " << Id << " for " << SerialNumber << std::endl;
				return true;
			}
		}
		return false;
	}

	bool CommandManager::SendCommand(	const std::string &SerialNumber,
							  			const std::string &Method,
										const Poco::JSON::Object &Params,
							  			const std::string &UUID,
									 	uint64_t & Id) {

		std::lock_guard G(Mutex_);

		Id = ++Id_;
		Poco::JSON::Object	CompleteRPC;
		CompleteRPC.set(uCentralProtocol::JSONRPC, uCentralProtocol::JSONRPC_VERSION);
		CompleteRPC.set(uCentralProtocol::ID, Id);
		CompleteRPC.set(uCentralProtocol::METHOD, Method );
		CompleteRPC.set(uCentralProtocol::PARAMS, Params);
		std::stringstream ToSend;
		Poco::JSON::Stringifier::stringify(CompleteRPC, ToSend);
		std::cout << "Sending command (" << Method << ") " << Id << "  for " << SerialNumber << std::endl;
		CommandTagIndex Idx{.Id=Id, .SerialNumber=SerialNumber};
		CommandTag		Tag;
		Tag.UUID = UUID;
		Tag.Submitted=std::time(nullptr);
		Tag.Completed=0;
		Tag.Result = Poco::makeShared<Poco::JSON::Object>();
		OutStandingRequests_[Idx] = Tag;
		return DeviceRegistry()->SendFrame(SerialNumber, ToSend.str());
	}

	void CommandManager::PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj) {

		if(!Obj->has(uCentralProtocol::ID)){
			Logger_.error("Invalid RPC response.");
			std::cout << "Invalid RPC response from " << SerialNumber << std::endl;
			return;
		}

		uint64_t ID = Obj->get(uCentralProtocol::ID);
		std::cout << "Received " << ID << " command for " << SerialNumber << std::endl;
		std::lock_guard G(Mutex_);
		auto Idx = CommandTagIndex{.Id=ID,.SerialNumber=SerialNumber};
		auto RPC = OutStandingRequests_.find(Idx);
		if(RPC != OutStandingRequests_.end()) {
			RPC->second.Completed=std::time(nullptr);
			RPC->second.Result=Obj;
			Storage()->CommandCompleted(RPC->second.UUID, Obj, true);
		} else {
			Logger_.warning(Poco::format("OUTDATED-RPC(%lu): Nothing waiting for this RPC.",ID));
		}
	}

}  // namespace