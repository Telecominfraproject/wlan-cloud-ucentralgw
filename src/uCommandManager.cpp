//
// Created by stephane bourque on 2021-03-19.
//

#include "uCommandManager.h"

#include "RESTAPI_objects.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"

namespace uCentral::CommandManager {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("CmdManager", "CMD_MGR", "command.manager"),
            Manager_(Logger_)
    {
    }

    int Start()
    {
        return uCentral::CommandManager::Service::instance()->Start();
    }

    void Stop()
    {
        uCentral::CommandManager::Service::instance()->Stop();
    }

    void WakeUp()
    {
        uCentral::CommandManager::Service::instance()->WakeUp();
    }

    void Manager::run() {
        while(!Stop_)
        {
            Poco::Thread::trySleep(2000);
            std::vector<uCentralCommandDetails> Commands;

            if(uCentral::Storage::GetReadyToExecuteCommands(0,1000,Commands))
            {
                for(auto & Cmd: Commands)
                {
                    if(!uCentral::DeviceRegistry::SendCommand(Cmd)) {
                        Logger_.information(Poco::format("Failed to send command '%s' to %s",Cmd.Command,Cmd.SerialNumber));
                    }
                }
            }
        }
    }

    int Service::Start() {
        Logger_.information("Starting...");
        ManagerThread.start(Manager_);
        return 0;
    }

    void Service::Stop() {
        Logger_.information("Stopping...");
        Manager_.stop();
        ManagerThread.join();
    }

    void Service::WakeUp() {
        Logger_.information("Waking up..");
        ManagerThread.wakeUp();
    }

};  // namespace