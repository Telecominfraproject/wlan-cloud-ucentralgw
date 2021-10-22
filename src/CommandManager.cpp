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

    class CommandManager * CommandManager::instance_ = nullptr;

	void CommandManager::run() {
		Running_ = true;
        while(Running_)
        {
            Poco::Thread::trySleep(30000);
			if(!Running_)
				break;

            std::vector<GWObjects::CommandDetails> Commands;
            if(StorageService()->GetReadyToExecuteCommands(1,200,Commands))
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
						StorageService()->SetCommandExecuted(Cmd.UUID);
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
		Logger_.information("Janitor starting.");
		for(auto i=OutStandingRequests_.begin();i!=OutStandingRequests_.end();) {
			if((Now-i->second.Submitted)>120)
				i = OutStandingRequests_.erase(i);
			else
				++i;
		}
		Logger_.information("Janitor finished.");
	}

	bool CommandManager::GetCommand(uint64_t Id, const std::string &SerialNumber, CommandTag &T) {
		std::lock_guard G(Mutex_);
		CommandTagIndex	TI{.Id=Id,.SerialNumber=SerialNumber};
		auto Hint=OutStandingRequests_.find(TI);
		if(Hint==OutStandingRequests_.end() || Hint->second.Completed==0)
			return false;
		T = Hint->second;
		OutStandingRequests_.erase(Hint);
		return true;
	}

	bool CommandManager::SendCommand(	const std::string &SerialNumber,
							  			const std::string &Method,
										const Poco::JSON::Object &Params,
							  			const std::string &UUID,
									 	uint64_t & Id,
									 	bool oneway_rpc) {

		std::stringstream ToSend;
		std::unique_lock G(Mutex_);
		if(oneway_rpc)
			Id = 1;
		else
			Id = ++Id_;
		Poco::JSON::Object CompleteRPC;
		CompleteRPC.set(uCentralProtocol::JSONRPC, uCentralProtocol::JSONRPC_VERSION);
		CompleteRPC.set(uCentralProtocol::ID, Id);
		CompleteRPC.set(uCentralProtocol::METHOD, Method);
		CompleteRPC.set(uCentralProtocol::PARAMS, Params);
		Poco::JSON::Stringifier::stringify(CompleteRPC, ToSend);
		Logger_.information(
			Poco::format("(%s): Sending command '%s', ID: %lu", SerialNumber, Method, Id));
		CommandTagIndex Idx{.Id = Id, .SerialNumber = SerialNumber};
		CommandTag Tag;
		Tag.UUID = UUID;
		Tag.Submitted = std::time(nullptr);
		Tag.Completed = 0;
		Tag.Result = Poco::makeShared<Poco::JSON::Object>();
		OutStandingRequests_[Idx] = Tag;
		G.unlock();
		return DeviceRegistry()->SendFrame(SerialNumber, ToSend.str());
	}

	void CommandManager::PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj) {

		if(!Obj->has(uCentralProtocol::ID)){
			Logger_.error(Poco::format("(%s): Invalid RPC response.",SerialNumber));
			return;
		}

		uint64_t ID = Obj->get(uCentralProtocol::ID);
		if(ID<2) {
			Logger_.error(Poco::format("(%s): Ignoring RPC response.",SerialNumber));
			return;
		}
		std::unique_lock G(Mutex_);
		auto Idx = CommandTagIndex{.Id = ID, .SerialNumber = SerialNumber};
		auto RPC = OutStandingRequests_.find(Idx);
		if (RPC == OutStandingRequests_.end()) {
			Logger_.warning(Poco::format("(%s): Outdated RPC %lu", SerialNumber, ID));
			return;
		}
		RPC->second.Completed = std::time(nullptr);
		RPC->second.Result = Obj;
		Logger_.information(Poco::format("(%s): Received RPC answer %lu", SerialNumber, ID));
		G.unlock();
		StorageService()->CommandCompleted(RPC->second.UUID, Obj, true);
	}

}  // namespace