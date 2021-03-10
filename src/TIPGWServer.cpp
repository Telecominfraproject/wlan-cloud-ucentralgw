//
// Created by stephane bourque on 2021-02-12.
//
#include "TIPGWServer.h"

namespace uCentral::TIPGW {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("TIPGWServer", "TIPGWServer", "tip.gateway") {
    }

    int Start() {
        return uCentral::TIPGW::Service::instance()->Start();
    }

    void Stop() {
        uCentral::TIPGW::Service::instance()->Stop();
    }

    int Service::Start() {
        SubSystemServer::logger().information("Starting.");
        return 0;
    }

    void Service::Stop() {
        SubSystemServer::logger().information("Stopping.");
    }

};  // namespace