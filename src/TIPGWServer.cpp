//
// Created by stephane bourque on 2021-02-12.
//
#include "TIPGWServer.h"

namespace uCentral::TIPGW {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("TIPGWServer", "TIPGWServer", "tip.gateway") {
    }

    int Service::start() {
        SubSystemServer::logger().information("Starting.");
        return 0;
    }

    void Service::stop() {
        SubSystemServer::logger().information("Stopping.");
    }

};  // namespace