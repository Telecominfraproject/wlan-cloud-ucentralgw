//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UCOMMANDMANAGER_H
#define UCENTRAL_UCOMMANDMANAGER_H

#include "uSubSystemServer.h"

namespace uCentral::CommandManager {

    int Start();
    void Stop();
    void WakeUp();

    class Manager : public Poco::Runnable {
    public:
        explicit Manager(Poco::Logger & Logger):
            Logger_(Logger)
        {}
        void run() override;
        void stop() { Stop_ = true; }
    private:
        bool 			Stop_ = false;
        Poco::Logger    & Logger_;
    };

    class Service : public uSubSystemServer {
    public:
        Service() noexcept;

        friend int Start();
        friend void Stop();
        friend void WakeUp();

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

    private:
		static Service *instance_;
		Manager         Manager_;
		Poco::Thread    ManagerThread;

        int Start() override;
        void Stop() override;
        void WakeUp();
    };

}  // namespace

#endif //UCENTRAL_UCOMMANDMANAGER_H
