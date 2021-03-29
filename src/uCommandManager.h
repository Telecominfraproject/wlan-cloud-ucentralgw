//
// Created by stephane bourque on 2021-03-19.
//

#ifndef UCENTRAL_UCOMMANDMANAGER_H
#define UCENTRAL_UCOMMANDMANAGER_H

#include "SubSystemServer.h"

namespace uCentral::CommandManager {

    int Start();
    void Stop();
    void WakeUp();

    class Manager : public Poco::Runnable {
    public:
        explicit Manager(Poco::Logger & Logger):
            Stop_(false),
            Logger_(Logger)
        {}
        void run() override;
        void stop() { Stop_ = true; }
    private:
        bool Stop_;
        Poco::Logger    & Logger_;
    };

    class Service : public SubSystemServer {
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
        int Start() override;
        void Stop() override;
        void WakeUp();

        Manager         Manager_;
        Poco::Thread    ManagerThread;

        static Service *instance_;

    };

};  // namespace

#endif //UCENTRAL_UCOMMANDMANAGER_H
