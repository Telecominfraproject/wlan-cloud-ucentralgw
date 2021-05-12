//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "uCommandManager.h"

#include "RESTAPI_objects.h"
#include "uStorageService.h"
#include "uDeviceRegistry.h"

namespace uCentral::CommandManager {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept: uSubSystemServer("CmdManager", "CMD_MGR", "command.manager"),
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
            std::vector<uCentral::Objects::CommandDetails> Commands;

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

}  // namespace