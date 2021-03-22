//
// Created by stephane bourque on 2021-03-19.
//

#include "uCommandManager.h"

namespace uCentral::CommandManager {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("CmdManager", "CMD_MGR", "command.manager")
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
    }

    void Service::WakeUp() {
        Logger_.information("Waking up..");
        ManagerThread.wakeUp();
    }


};  // namespace