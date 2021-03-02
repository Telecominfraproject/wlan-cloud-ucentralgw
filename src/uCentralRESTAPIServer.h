//
// Created by stephane bourque on 2021-02-28.
//

#ifndef UCENTRAL_UCENTRALRESTAPISERVER_H
#define UCENTRAL_UCENTRALRESTAPISERVER_H

#include "SubSystemServer.h"

class uCentralRESTAPIServer : public SubSystemServer {

public:
    uCentralRESTAPIServer() noexcept;

    int start();
    void stop();

    Logger & logger() { return SubSystemServer::logger(); };

    static uCentralRESTAPIServer *instance() {
        if(instance_== nullptr) {
            instance_ = new uCentralRESTAPIServer;
        }
        return instance_;
    }

private:
    static uCentralRESTAPIServer * instance_;

};

#endif //UCENTRAL_UCENTRALRESTAPISERVER_H
