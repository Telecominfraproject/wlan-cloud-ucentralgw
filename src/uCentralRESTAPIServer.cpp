//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralRESTAPIServer.h"

#include "Poco/URI.h"

#include "RESTAPI_oauth2Handler.h"
#include "RESTAPI_devicesHandler.h"
#include "RESTAPI_deviceHandler.h"
#include "RESTAPI_UnknownRequestHandler.h"

uCentralRESTAPIServer * uCentralRESTAPIServer::instance_ = nullptr;

uCentralRESTAPIServer::uCentralRESTAPIServer() noexcept:
        SubSystemServer("RESTAPIServer","RESTAPIServer","ucentral.restapi"),
        server_(nullptr)
{

}

int uCentralRESTAPIServer::start() {
    SubSystemServer::logger().information("Starting.");

    std::string l{"Starting: " +
                  SubSystemServer::host(0).address() + ":" + std::to_string(SubSystemServer::host(0).port()) +
                  " key:" + SubSystemServer::host(0).key_file() +
                  " cert:" + SubSystemServer::host(0).cert_file()};

    logger().information(l);

    SecureServerSocket  sock( SubSystemServer::host(0).port(),
                              64,
                              new Context(Poco::Net::Context::TLS_SERVER_USE,
                                          SubSystemServer::host(0).key_file(),
                                          SubSystemServer::host(0).cert_file(),
                                          ""));

    auto Params = new HTTPServerParams;

    Params->setMaxThreads(16);
    Params->setMaxQueued(100);

    server_ = new HTTPServer( new RESTAPIRequestHandlerFactory, sock, Params);

    server_->start();

    return 0;
}

HTTPRequestHandler* RESTAPIRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
    uCentralRESTAPIServer::instance()->logger().information("Request from "
                                                              + request.clientAddress().toString()
                                                              + ": "
                                                              + request.getMethod()
                                                              + " "
                                                              + request.getURI()
                                                              + " "
                                                              + request.getVersion());

    for (auto it = request.begin(); it != request.end(); ++it)
    {
        uCentralRESTAPIServer::instance()->logger().information(it->first + ": " + it->second);
    }

    Poco::URI   uri(request.getURI());

    if( strncmp(uri.getPath().c_str(),"/api/v1/oauth2",strlen("/api/v1/oauth2"))==0) {
        return new RESTAPI_oauth2Handler;
    } else if ( strncmp(uri.getPath().c_str(),"/api/v1/devices",strlen("/api/v1/devices"))==0) {
        return new RESTAPI_devicesHandler;
    } else if ( strncmp(uri.getPath().c_str(),"/api/v1/device",strlen("/api/v1/device"))==0) {
        return new RESTAPI_deviceHandler;
    }

    return new RESTAPI_UnknownRequestHandler;
}

void uCentralRESTAPIServer::stop() {
    SubSystemServer::logger().information("Stopping ");

    server_->stop();
}

void RESTAPIPageRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
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