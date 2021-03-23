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
    void CommandCompletion( const std::string & UUID, const std::string & Status );

    class Manager : public Poco::Runnable {
    public:
        Manager() :
            Stop_(false)
        {}
        void run() override;
        void stop() { Stop_ = true; }
        void CommandCompletion( const std::string & UUID, const std::string & Status );
    private:
        bool Stop_;
    };

    class Service : public SubSystemServer {
    public:
        Service() noexcept;

        friend int Start();
        friend void Stop();
        friend void WakeUp();
        friend void CommandCompletion( const std::string & UUID, const std::string & Status );

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
        void CommandCompletion( const std::string & UUID, const std::string & Status );

        Manager         Manager_;
        Poco::Thread    ManagerThread;

        static Service *instance_;

    };

};  // namespace

#endif //UCENTRAL_UCOMMANDMANAGER_H
