//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralWebSocketServer.h"
#include "uStorageService.h"

uCentralWebSocketServer * uCentralWebSocketServer::instance_= nullptr;

uCentralWebSocketServer::uCentralWebSocketServer() noexcept:
        SubSystemServer("WebSocketServer","WebSocketServer","ucentral.websocket")
{

}

int uCentralWebSocketServer::start() {
    // SubSystemServer::logger().information

    std::string l{"Starting: " +
    SubSystemServer::host(0).address() + ":" + std::to_string(SubSystemServer::host(0).port()) +
    " key:" + SubSystemServer::host(0).key_file() + " cert:" + SubSystemServer::host(0).cert_file()};

    std::cout << "LOGGING: " << l << std::endl;

    SecureServerSocket  sock( SubSystemServer::host(0).port(),
                              64,
                              new Context(Poco::Net::Context::TLS_SERVER_USE,
                                          SubSystemServer::host(0).key_file(),
                                          SubSystemServer::host(0).cert_file(),
                                          ""));

    server_ = new HTTPServer( new RequestHandlerFactory, sock, new HTTPServerParams);

    server_->start();

    return 0;
}

void uCentralWebSocketServer::stop() {
    SubSystemServer::logger().information("Stopping ");

    server_->stop();
}

std::string default_config() {
    return std::string{"{\"cfg\": {\"uuid\":1613927736,\"steer\":{\"enabled\":1,\"network\":\"wan\",\"debug_level\":0},\"stats\":{\"interval\":60,\"neighbours\":1,\"traffic\":1,\"wifiiface\":1,\"wifistation\":1,\"pids\":1,\"serviceprobe\":1,\"lldp\":1,\"system\":1,\"poe\":1},\"phy\":[{\"band\":\"2\",\"cfg\":{\"disabled\":0,\"country\":\"DE\",\"channel\":6,\"txpower\":30,\"beacon_int\":100,\"htmode\":\"HE40\",\"hwmode\":\"11g\",\"chanbw\":20}},{\"band\":\"5\",\"cfg\":{\"mimo\":\"4x4\",\"disabled\":0,\"country\":\"DE\",\"channel\":0,\"htmode\":\"HE80\"}},{\"band\":\"5u\",\"cfg\":{\"disabled\":0,\"country\":\"DE\",\"channel\":100,\"htmode\":\"VHT80\"}},{\"band\":\"5l\",\"cfg\":{\"disabled\":0,\"country\":\"DE\",\"channel\":36,\"htmode\":\"VHT80\"}}],\"ssid\":[{\"band\":[\"2\"],\"cfg\":{\"ssid\":\"uCentral-Guest\",\"encryption\":\"psk2\",\"key\":\"OpenWifi\",\"mode\":\"ap\",\"isolate\":1,\"network\":\"guest\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f57\"}},{\"band\":[\"5l\",\"5\"],\"cfg\":{\"ssid\":\"uCentral-NAT.200\",\"encryption\":\"psk2\",\"key\":\"OpenWifi\",\"mode\":\"ap\",\"network\":\"nat200\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}},{\"band\":[\"5l\",\"5\"],\"cfg\":{\"ssid\":\"uCentral-EAP\",\"encryption\":\"wpa2\",\"server\":\"148.251.188.218\",\"port\":1812,\"auth_secret\":\"uSyncRad1u5\",\"mode\":\"ap\",\"network\":\"lan\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}},{\"band\":[\"5l\",\"5\"],\"cfg\":{\"ssid\":\"uCentral\",\"encryption\":\"psk2\",\"key\":\"OpenWifi\",\"mode\":\"ap\",\"network\":\"wan\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}}],\"network\":[{\"mode\":\"wan\",\"cfg\":{\"proto\":\"dhcp\"}},{\"mode\":\"gre\",\"cfg\":{\"vid\":\"50\",\"peeraddr\":\"50.210.104.108\"}},{\"mode\":\"nat\",\"vlan\":200,\"cfg\":{\"proto\":\"static\",\"ipaddr\":\"192.168.16.1\",\"netmask\":\"255.255.255.0\",\"mtu\":1500,\"ip6assign\":60,\"dhcp\":{\"start\":10,\"limit\":100,\"leasetime\":\"6h\"},\"leases\":[{\"ip\":\"192.168.100.2\",\"mac\":\"00:11:22:33:44:55\",\"hostname\":\"test\"},{\"ip\":\"192.168.100.3\",\"mac\":\"00:11:22:33:44:56\",\"hostname\":\"test2\"}]}},{\"mode\":\"guest\",\"cfg\":{\"proto\":\"static\",\"ipaddr\":\"192.168.12.11\",\"dhcp\":{\"start\":10,\"limit\":100,\"leasetime\":\"6h\"}}}],\"ntp\":{\"enabled\":1,\"enable_server\":1,\"server\":[\"0.openwrt.pool.ntp.org\",\"1.openwrt.pool.ntp.org\"]},\"ssh\":{\"enable\":1,\"Port\":22},\"system\":{\"timezone\":\"CET-1CEST,M3.5.0,M10.5.0/3\"},\"log\":{\"_log_proto\":\"udp\",\"_log_ip\":\"192.168.11.23\",\"_log_port\":12345,\"_log_hostname\":\"foo\",\"_log_size\":128},\"rtty\":{\"host\":\"websocket.usync.org\",\"token\":\"7049cb6b7949ba06c6b356d76f0f6275\",\"interface\":\"wan\"}}}"};
}

std::string uCentralWebSocketServer::process_message(const std::string &m, ConnectionState &conn)
{
    Parser  parser;

    Poco::Dynamic::Var result = parser.parse(m);
    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
    Poco::DynamicStruct ds = *object;

    std::string SerialNumber = ds["serial"].toString();

    std::cout << "SERIAL: " << SerialNumber << std::endl;

    conn.SerialNumber = SerialNumber;

    if(ds.contains("state") && ds.contains("serial"))
    {
        logger().information(SerialNumber + ": updating statistics.");
        std::cout << "STATE_MSG 1 " << std::endl ;
        std::string NewStatistics{ds["state"].toString()};
        uStorageService::instance()->AddStatisticsData(conn.SerialNumber,conn.CfgUUID,NewStatistics);
        std::cout << "STATE_MSG 2 " << std::endl ;
    }
    else if(ds.contains("capab") && ds.contains("serial"))
    {
        logger().information(SerialNumber + ": updating capabilities.");
        conn.SerialNumber = ds["serial"].toString();
        std::string     NewCapabilities{ds["capab"].toString()};
        uStorageService::instance()->UpdateDeviceCapabilities(conn.SerialNumber,NewCapabilities);
    }
    else if(ds.contains("uuid") && ds.contains("serial") && ds.contains("active")) {
        conn.CfgUUID = ds["uuid"];
        std::cout << "UUID of config" << ds["uuid"].toString() << std::endl;
        return default_config();
    }
    else if(ds.contains("uuid") && ds.contains("serial")) {
        conn.CfgUUID = ds["uuid"];
        std::cout << "UUID of config" << ds["uuid"].toString() << std::endl;
        return default_config();
    }
    else if(ds.contains("log")) {
        std::string log = ds["log"].toString();
        logger().warning("DEVICE-LOG(" + SerialNumber + "):" + log);
    }
    else
    {
        std::cout << "UNKNOWN_MESSAGE: " << m << std::endl;
    }

    return "";
}

void PageRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
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

void WebSocketRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    Application& app = Application::instance();
    try
    {

        WebSocket   ws(request, response);
        std::string Address{ws.peerAddress().toString()};

        std::cout << "Connection from: " << Address << std::endl;

        ConnectionState conn{ .SerialNumber{""},
                .Address{Address},
                .messages=0,
                .CfgUUID=0,
                .RX=0,
                .TX=0};

        uCentralWebSocketServer::instance()->logger().information("WebSocket connection established from .");
        char buffer[32000];
        int flags;
        int n;
        do
        {

            ws.peerAddress().toString();
            memset(buffer,0,sizeof(buffer));
            n = ws.receiveFrame(buffer, sizeof(buffer), flags);
            conn.messages++;

            uCentralWebSocketServer::instance()->logger().information(Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(flags)));

            std::string m{buffer};
            conn.RX += m.size();

            std::string return_document = uCentralWebSocketServer::instance()->process_message(m,conn);
            conn.TX += return_document.size();

            if( !return_document.empty()) {
                // std::cout << "RETURN:" << return_document << std::endl;
                ws.sendFrame(return_document.c_str(), return_document.size(), flags);
            }
        }
        while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
        std::cout << "ENDING CONNECTION..." << std::endl;
        uCentralWebSocketServer::instance()->logger().information("WebSocket connection closed.");
    }
    catch (WebSocketException& exc)
    {
        uCentralWebSocketServer::instance()->logger().log(exc);
        switch (exc.code())
        {
            case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
                response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
                // fallthrough
            case WebSocket::WS_ERR_NO_HANDSHAKE:
            case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
            case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                break;
        }
    }
}

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
    uCentralWebSocketServer::instance()->logger().information("Request from "
                                                              + request.clientAddress().toString()
                                                              + ": "
                                                              + request.getMethod()
                                                              + " "
                                                              + request.getURI()
                                                              + " "
                                                              + request.getVersion());

    for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it)
    {
        uCentralWebSocketServer::instance()->logger().information(it->first + ": " + it->second);
    }

    if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
        return new WebSocketRequestHandler;
    else
        return new PageRequestHandler;
}
