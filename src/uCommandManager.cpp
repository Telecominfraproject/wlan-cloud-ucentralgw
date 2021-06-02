//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <algorithm>
#include "uCommandManager.h"

#include "RESTAPI_objects.h"
#include "RESTAPI_handler.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"
#include "uCentralProtocol.h"
#include "uCommandManager.h"

#include "Poco/JSON/Parser.h"

#define DBG		std::cout << __LINE__ << "   " __FILE__ << std::endl;

namespace uCentral::CommandManager {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept: uSubSystemServer("CmdManager", "CMD_MGR", "command.manager"),
            Manager_(Logger_)
    {
    }

    int Start()
    {
        return Service::instance()->Start();
    }

    void Stop()
    {
        Service::instance()->Stop();
    }

	void Janitor() {
		Service::instance()->Janitor();
	}

    void WakeUp()
    {
        uCentral::CommandManager::Service::instance()->WakeUp();
    }

	void PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Result) {
		Service::instance()->PostCommandResult(SerialNumber, std::move(Result));
	}

	bool SendCommand( 	const std::string & SerialNumber, const std::string & Method,
					 	const Poco::JSON::Object &Params, std::shared_ptr<std::promise<Poco::JSON::Object::Ptr>> Promise,
					 	const std::string &UUID) {
		return Service::instance()->SendCommand(SerialNumber, Method, Params, std::move(Promise), UUID);
	}

	bool SendCommand(uCentral::Objects::CommandDetails & Command) {
		return Service::instance()->SendCommand(Command);
	}

	void Manager::run() {
        while(!Stop_)
        {
            Poco::Thread::trySleep(10000);
            std::vector<uCentral::Objects::CommandDetails> Commands;

            if(uCentral::Storage::GetReadyToExecuteCommands(0,1000,Commands))
            {
                for(auto & Cmd: Commands)
                {
                    if(!SendCommand(Cmd)) {
                        Logger_.information(Poco::format("Failed to send command '%s' to %s",Cmd.Command,Cmd.SerialNumber));
                    }
                }
            }
			Janitor();
        }
    }

    int Service::Start() {
        Logger_.notice("Starting...");
        ManagerThread.start(Manager_);
        return 0;
    }

    void Service::Stop() {
        Logger_.notice("Stopping...");
        Manager_.stop();
        ManagerThread.join();
    }

    void Service::WakeUp() {
        Logger_.notice("Waking up..");
        ManagerThread.wakeUp();
    }

	void Service::Janitor() {
		SubMutexGuard G(SubMutex);
		uint64_t Now = time(nullptr);
		for(auto i = Age_.begin(); i!= Age_.end();)
			if((Now-i->first)>300)
				Age_.erase(i++);
			else
				++i;
	}

	bool Service::SendCommand(const std::string &SerialNumber,
							  const std::string &Method,
							  const Poco::JSON::Object &Params,
							  std::shared_ptr<std::promise<Poco::JSON::Object::Ptr>> Promise,
							  const std::string &UUID) {

		SubMutexGuard G(SubMutex);

		Poco::JSON::Object	CompleteRPC;
		CompleteRPC.set(uCentralProtocol::JSONRPC, uCentralProtocol::JSONRPC_VERSION);
		CompleteRPC.set(uCentralProtocol::ID, Id_);
		CompleteRPC.set(uCentralProtocol::METHOD, Method );
		CompleteRPC.set(uCentralProtocol::PARAMS, Params);
		std::stringstream ToSend;
		Poco::JSON::Stringifier::stringify(CompleteRPC, ToSend);

		OutStandingRequests_[Id_] = std::make_pair(std::move(Promise),UUID);
		Age_[Id_] = time(nullptr);
		Id_++;
		return uCentral::DeviceRegistry::SendFrame(SerialNumber, ToSend.str());
	}

	bool Service::SendCommand(uCentral::Objects::CommandDetails & Command) {
		SubMutexGuard G(SubMutex);

		Logger_.debug(Poco::format("Sending command to %s",Command.SerialNumber));
		try {
			Poco::JSON::Object Obj;

			Obj.set(uCentralProtocol::JSONRPC,uCentralProtocol::JSONRPC_VERSION);
			Obj.set(uCentralProtocol::ID,Id_);
			Obj.set(uCentralProtocol::METHOD, Command.Custom ? uCentralProtocol::PERFORM : Command.Command );

			bool FullCommand = true;
			if(Command.Command==uCentralProtocol::REQUEST)
				FullCommand = false;

			// the params section was composed earlier... just include it here
			Poco::JSON::Parser  parser;
			auto ParsedMessage = parser.parse(Command.Details);
			const auto & ParamsObj = ParsedMessage.extract<Poco::JSON::Object::Ptr>();
			Obj.set(uCentralProtocol::PARAMS,ParamsObj);
			std::stringstream ToSend;
			Poco::JSON::Stringifier::stringify(Obj,ToSend);

			if(uCentral::DeviceRegistry::SendFrame(Command.SerialNumber, ToSend.str())) {
				uCentral::Storage::SetCommandExecuted(Command.UUID);
				OutStandingRequests_[Id_] = std::make_pair(nullptr,Command.UUID);
				Age_[Id_] = time(nullptr);
				return true;
			} else {

			}
			Id_++;
		}
		catch( const Poco::Exception & E )
		{
			Logger_.warning(Poco::format("COMMAND(%s): Exception while sending a command.",Command.SerialNumber));
		}
		return false;
	}


	void Service::PostCommandResult(const std::string &SerialNumber, Poco::JSON::Object::Ptr Obj) {
		if(!Obj->has(uCentralProtocol::ID)){
			Logger_.error("Invalid RPC response.");
			return;
		}

		SubMutexGuard G(SubMutex);

		uint64_t ID = Obj->get(uCentralProtocol::ID);
		auto RPC = OutStandingRequests_.find(ID);
		Age_.erase(ID);
		if(RPC != OutStandingRequests_.end()) {
			if(RPC->second.first != nullptr) {
				try {
					RPC->second.first->set_value(std::move(Obj));
				} catch (...) {
					Logger_.error(Poco::format("COMPLETING-RPC(%Lu): future was lost", ID));
					uCentral::Storage::CommandCompleted(RPC->second.second, Obj, true);
				}
			}
			else {
				uCentral::Storage::CommandCompleted(RPC->second.second, Obj, true);
			}
			OutStandingRequests_.erase(RPC);
		} else {
			Logger_.warning(Poco::format("OUTDATED-RPC(%lu): Nothing waiting for this RPC.",ID));
		}
	}

}  // namespace