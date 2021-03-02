//
// Created by stephane bourque on 2021-02-12.
//
#include "TIPGWServer.h"

TIPGWServer * TIPGWServer::instance_= nullptr;

TIPGWServer::TIPGWServer() noexcept:
    SubSystemServer("TIPGWServer","TIPGWServer","tip.gateway")
{
}

int TIPGWServer::start() {
    SubSystemServer::logger().information("Starting.");
    return 0;
}

void TIPGWServer::stop() {
    SubSystemServer::logger().information("Stopping.");
}