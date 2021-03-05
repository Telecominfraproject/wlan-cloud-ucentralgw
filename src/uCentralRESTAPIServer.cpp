//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralRESTAPIServer.h"

#include "Poco/URI.h"

#include "RESTAPI_oauth2Handler.h"
#include "RESTAPI_devicesHandler.h"
#include "RESTAPI_deviceHandler.h"
#include "RESTAPI_deviceCommandHandler.h"
#include "RESTAPI_UnknownRequestHandler.h"

namespace uCentral::RESTAPI {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("RESTAPIServer", "RESTAPIServer", "ucentral.restapi"),
            server_(nullptr) {

    }

    int Service::start() {
        logger_.information("Starting.");

        std::string l{"Starting: " +
                      SubSystemServer::host(0).address() + ":" + std::to_string(SubSystemServer::host(0).port()) +
                      " key:" + SubSystemServer::host(0).key_file() +
                      " cert:" + SubSystemServer::host(0).cert_file()};

        logger_.information(l);

        SecureServerSocket sock(SubSystemServer::host(0).port(),
                                64,
                                new Context(Poco::Net::Context::TLS_SERVER_USE,
                                            SubSystemServer::host(0).key_file(),
                                            SubSystemServer::host(0).cert_file(),
                                            ""));

        auto Params = new HTTPServerParams;

        Params->setMaxThreads(16);
        Params->setMaxQueued(100);

        server_ = new HTTPServer(this, sock, Params);

        server_->start();

        return 0;
    }

    HTTPRequestHandler *Service::createRequestHandler(const HTTPServerRequest &request) {
        logger_.information("Request from "
                            + request.clientAddress().toString()
                            + ": "
                            + request.getMethod()
                            + " "
                            + request.getURI()
                            + " "
                            + request.getVersion());

//    for (auto it : request)
//        logger_.information(it.first + ": " + it.second);

        Poco::URI uri(request.getURI());
        const char *path = uri.getPath().c_str();

        RESTAPIHandler::BindingMap bindings;

        if (RESTAPIHandler::path_match(path, "/api/v1/oauth2", bindings)) {
            return new RESTAPI_oauth2Handler(bindings, logger_);
        } else if (RESTAPIHandler::path_match(path, "/api/v1/oauth2/{token}", bindings)) {
            return new RESTAPI_oauth2Handler(bindings, logger_);
        } else if (RESTAPIHandler::path_match(path, "/api/v1/devices", bindings)) {
            return new RESTAPI_devicesHandler(bindings, logger_);
        } else if (RESTAPIHandler::path_match(path, "/api/v1/device/{serialNumber}", bindings)) {
            return new RESTAPI_deviceHandler(bindings, logger_);
        } else if (RESTAPIHandler::path_match(path, "/api/v1/device/{serialNumber}/{command}", bindings)) {
            return new RESTAPI_deviceCommandHandler(bindings, logger_);
        }

        return new RESTAPI_UnknownRequestHandler;
    }

    void Service::stop() {
        SubSystemServer::logger().information("Stopping ");

        server_->stop();
    }

/*
void uCentralRESTAPIServer::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    if(request.getURI() == "/api/v1/oauth2") {

    } else if(request.getURI() == "/api/v1/devices") {

    } else if(request.getURI() == "/api/v1/device") {

    }

    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");
    std::ostream& ostr = response.send();
    ostr << "<html>";
    ostr << "<head>";
    ostr << "<title>WebSocketServer</title>";
    ostr << "<script type=\"text/javascript\">";
    ostr << "function WebSocketTest()";
    ostr << "{";
    ostr << "  if (\"WebSocket\" in window)";
    ostr << "  {";
    ostr << "    var ws = new WebSocket(\"ws://" << request.serverAddress().toString() << "/ws\");";
    ostr << "    ws.onopen = function()";
    ostr << "      {";
    ostr << "        ws.send(\"Hello, world!\");";
    ostr << "      };";
    ostr << "    ws.onmessage = function(evt)";
    ostr << "      { ";
    ostr << "        var msg = evt.data;";
    ostr << "        alert(\"Message received: \" + msg);";
    ostr << "        ws.close();";
    ostr << "      };";
    ostr << "    ws.onclose = function()";
    ostr << "      { ";
    ostr << "        alert(\"WebSocket closed.\");";
    ostr << "      };";
    ostr << "  }";
    ostr << "  else";
    ostr << "  {";
    ostr << "     alert(\"This browser does not support WebSockets.\");";
    ostr << "  }";
    ostr << "}";
    ostr << "</script>";
    ostr << "</head>";
    ostr << "<body>";
    ostr << "  <h1>WebSocket Server</h1>";
    ostr << "  <p><a href=\"javascript:WebSocketTest()\">Run WebSocket Script</a></p>";
    ostr << "</body>";
    ostr << "</html>";
}
 */

};  // namespace