//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralRESTAPIServer.h"

uCentralRESTAPIServer * uCentralRESTAPIServer::instance_ = nullptr;

uCentralRESTAPIServer::uCentralRESTAPIServer() noexcept:
        SubSystemServer("RESTAPIServer","RESTAPIServer","ucentral.restapi")
{

}

int uCentralRESTAPIServer::start() {
    SubSystemServer::logger().information("Starting ");

    return 0;
}

void uCentralRESTAPIServer::stop() {
    SubSystemServer::logger().information("Stopping ");

}