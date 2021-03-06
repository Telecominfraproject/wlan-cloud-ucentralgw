//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralWebSocketServer.h"
#include "uStorageService.h"

namespace uCentral::WebSocket {

    Service *Service::instance_ = nullptr;

    Service::Service() noexcept:
            SubSystemServer("WebSocketServer", "WS-SVR", "ucentral.websocket")
    {

    }

    int Service::Start() {

        //  create the reactor
        SocketReactorThread_.start(SocketReactor_);

        for(const auto & svr : ConfigurationServers()) {
            std::string l{
                    "Starting: " + svr.address() + ":" + std::to_string(svr.port()) +
                    " key:" + svr.key_file() +
                    " cert:" + svr.cert_file()};

            logger().information(l);

            SecureServerSocket sock(svr.port(),
                                    64,
                                    new Context(Poco::Net::Context::TLS_SERVER_USE,
                                                svr.key_file(),
                                                svr.cert_file(),
                                                ""));

            auto NewServer = std::make_shared<Poco::Net::HTTPServer>(new WSRequestHandlerFactory(SocketReactor_), sock, new HTTPServerParams);

            NewServer->start();

            HTTPServers_.push_back(NewServer);
        }

        return 0;
    }

    void Service::Stop() {
        SubSystemServer::logger().information("Stopping ");

        SocketReactor_.stop();
        for(auto const & svr : HTTPServers_)
            svr->stop();
    }

    std::string default_config_1() {
        return std::string{
                "{\"uuid\":1613927736,\"steer\":{\"enabled\":1,\"network\":\"wan\",\"debug_level\":0},\"stats\":"
                "{\"interval\":60,\"neighbours\":1,\"traffic\":1,\"wifiiface\":1,\"wifistation\":1,\"pids\":1,"
                "\"serviceprobe\":1,\"lldp\":1,\"system\":1,\"poe\":1},\"phy\":[{\"band\":\"2\",\"cfg\":{\"disabled\":0"
                ",\"country\":\"DE\",\"channel\":6,\"txpower\":30,\"beacon_int\":100,\"htmode\":\"HE40\",\"hwmode"
                "\":\"11g\",\"chanbw\":20}},{\"band\":\"5\",\"cfg\":{\"mimo\":\"4x4\",\"disabled\":0,\"country\":\"DE\","
                "\"channel\":0,\"htmode\":\"HE80\"}},{\"band\":\"5u\",\"cfg\":{\"disabled\":0,\"country\":\"DE\","
                "\"channel\":100,\"htmode\":\"VHT80\"}},{\"band\":\"5l\",\"cfg\":{\"disabled\":0,\"country\":\"DE\",\"channel"
                "\":36,\"htmode\":\"VHT80\"}}],\"ssid\":[{\"band\":[\"2\"],\"cfg\":{\"ssid\":\"uCentral-Guest\",\"encryption"
                "\":\"psk2\",\"key\":\"OpenWifi\",\"mode\":\"ap\",\"isolate\":1,\"network\":\"guest\",\"ieee80211r\":1,"
                "\"ieee80211v\":1,\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f57\"}"
                "},{\"band\":[\"5l\",\"5\"],\"cfg\":{\"ssid\":\"uCentral-NAT.200\",\"encryption\":\"psk2\",\"key\":\""
                "OpenWifi\",\"mode\":\"ap\",\"network\":\"nat200\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,"
                "\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}},{\"band\":[\"5l\",\"5\"],\"cfg\""
                ":{\"ssid\":\"uCentral-EAP\",\"encryption\":\"wpa2\",\"server\":\"148.251.188.218\",\"port\":1812,\""
                "auth_secret\":\"uSyncRad1u5\",\"mode\":\"ap\",\"network\":\"lan\",\"ieee80211r\":1,\"ieee80211v\":1,"
                "\"ieee80211k\":1,\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}},"
                "{\"band\":[\"5l\",\"5\"],\"cfg\":{\"ssid\":\"uCentral\",\"encryption\":\"psk2\",\"key\":\"OpenWifi\","
                "\"mode\":\"ap\",\"network\":\"wan\",\"ieee80211r\":1,\"ieee80211v\":1,\"ieee80211k\":1,"
                "\"ft_psk_generate_local\":1,\"ft_over_ds\":1,\"mobility_domain\":\"4f51\"}}],\"network\":"
                "[{\"mode\":\"wan\",\"cfg\":{\"proto\":\"dhcp\"}},{\"mode\":\"gre\",\"cfg\":{\"vid\":\"50\""
                ",\"peeraddr\":\"50.210.104.108\"}},{\"mode\":\"nat\",\"vlan\":200,\"cfg\":{\"proto\":\"static\""
                ",\"ipaddr\":\"192.168.16.1\",\"netmask\":\"255.255.255.0\",\"mtu\":1500,\"ip6assign\":60,\"dhcp\":"
                "{\"start\":10,\"limit\":100,\"leasetime\":\"6h\"},\"leases\":[{\"ip\":\"192.168.100.2\",\"mac\":"
                "\"00:11:22:33:44:55\",\"hostname\":\"test\"},{\"ip\":\"192.168.100.3\",\"mac\":\"00:11:22:33:44:56\","
                "\"hostname\":\"test2\"}]}},{\"mode\":\"guest\",\"cfg\":{\"proto\":\"static\",\"ipaddr\":\"192.168.12.11\","
                "\"dhcp\":{\"start\":10,\"limit\":100,\"leasetime\":\"6h\"}}}],\"ntp\":{\"enabled\":1,\"enable_server\":1,"
                "\"server\":[\"0.openwrt.pool.ntp.org\",\"1.openwrt.pool.ntp.org\"]},\"ssh\":{\"enable\":1,\"Port\":22},"
                "\"system\":{\"timezone\":\"CET-1CEST,M3.5.0,M10.5.0/3\"},\"log\":{\"_log_proto\":\"udp\",\"_log_ip\":"
                "\"192.168.11.23\",\"_log_port\":12345,\"_log_hostname\":\"foo\",\"_log_size\":128},\"rtty\":{\"host\":"
                "\"websocket.usync.org\",\"token\":\"7049cb6b7949ba06c6b356d76f0f6275\",\"interface\":\"wan\"}}"};
    }

    void WSConnection::ProcessMessage(std::string &Response) {
        Parser parser;

        auto result = parser.parse(IncomingMessage_);
        auto object = result.extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct ds = *object;

        if( Conn.SerialNumber.empty() ) {
            Conn.SerialNumber = ds["serial"].toString();
            uCentral::DeviceRegistry::Service::instance()->Register(Conn.SerialNumber, this);
        }

        if (ds.contains("state") && ds.contains("serial")) {
            Logger_.information(Conn.SerialNumber + ": updating statistics.");
            std::string NewStatistics{ds["state"].toString()};
            uCentral::Storage::Service::instance()->AddStatisticsData(Conn.SerialNumber, Conn.CfgUUID,
                                                                      NewStatistics);
        } else if (ds.contains("capab") && ds.contains("serial")) {
            Logger_.information(Conn.SerialNumber + ": updating capabilities.");
            std::string NewCapabilities{ds["capab"].toString()};
            uCentral::Storage::Service::instance()->UpdateDeviceCapabilities(Conn.SerialNumber, NewCapabilities);
        } else if (ds.contains("uuid") && ds.contains("serial") && ds.contains("active")) {
            Logger_.information(Conn.SerialNumber + ": updating active configuration.");
            Conn.CfgUUID = ds["uuid"];
            std::cout << "Waiting to apply configuration " << ds["active"].toString() << std::endl;
        } else if (ds.contains("uuid") && ds.contains("serial")) {
            Logger_.information(Conn.SerialNumber + ": configuration check.");
            Conn.CfgUUID = ds["uuid"];

            std::string NewConfig;
            uint64_t NewConfigUUID;

            if (uCentral::Storage::Service::instance()->ExistingConfiguration(Conn.SerialNumber, Conn.CfgUUID,
                                                                              NewConfig, NewConfigUUID)) {
                if (Conn.CfgUUID < NewConfigUUID) {
                    std::cout << "We have a newer configuration." << std::endl;
                    Response = "{ \"cfg\" : " + NewConfig + "}";
                }
            }
        } else if (ds.contains("log")) {
            auto log = ds["log"].toString();
            Logger_.warning("DEVICE-LOG(" + Conn.SerialNumber + "):" + log);
        } else {
            std::cout << "UNKNOWN_MESSAGE(" << Conn.SerialNumber << "): " << IncomingMessage_ << std::endl;
        }
    }

    void WSConnection::OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf)
    {
        int flags, Op;
        int IncomingSize = 0;

        std::lock_guard<std::mutex> guard(mutex_);
        memset(IncomingMessage_, 0, sizeof(IncomingMessage_));

        try {
            IncomingSize = WS_.receiveFrame(IncomingMessage_, sizeof(IncomingMessage_), flags);
            Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

            if(IncomingSize==0 && flags == 0 && Op == 0)
            {
                delete this;
                return;
            }

            Conn.messages++;

            switch (Op) {
                case Poco::Net::WebSocket::FRAME_OP_PING: {
                    Logger_.information("PING(" + Conn.SerialNumber + "): received.");
                    WS_.sendFrame("", 0, Poco::Net::WebSocket::FRAME_OP_PONG | Poco::Net::WebSocket::FRAME_FLAG_FIN);
                }
                    break;

                case Poco::Net::WebSocket::FRAME_OP_PONG: {
                    Logger_.information("PONG(" + Conn.SerialNumber + "): received.");
                }
                    break;

                case Poco::Net::WebSocket::FRAME_OP_TEXT: {
                    std::cout << "Incoming(" << Conn.SerialNumber << "): " << IncomingSize << " bytes." << std::endl;
                    Logger_.debug(
                            Poco::format("Frame received (length=%d, flags=0x%x).", IncomingSize, unsigned(flags)));
                    Conn.RX += IncomingSize;

                    std::string ResponseDocument;
                    ProcessMessage(ResponseDocument);

                    if (!ResponseDocument.empty()) {
                        Conn.TX += ResponseDocument.size();
                        std::cout << "Returning(" << Conn.SerialNumber << "): " << ResponseDocument.size() << " bytes"
                                  << std::endl;
                        WS_.sendFrame(ResponseDocument.c_str(), ResponseDocument.size());
                    }
                    }
                    break;

                default: {
                    Logger_.warning("UNKNOWN WS Frame operation: " + std::to_string(Op));
                    std::cout << "WS: Unknown frame: " << Op << " Flags: " << flags << std::endl;
                    Op = Poco::Net::WebSocket::FRAME_OP_CLOSE;
                }
            }

            if(!Conn.SerialNumber.empty())
                uCentral::DeviceRegistry::Service::instance()->SetState(Conn.SerialNumber,Conn);
        }
        catch (const Poco::Exception &exc) {
            std::cout << "Caught a more generic Poco exception: " << exc.message() << std::endl;
            delete this;
        }
    }

    bool WSConnection::SendCommand(const std::string &Cmd) {
        std::lock_guard<std::mutex> guard(mutex_);

        Logger_.information(Poco::format("Sending commnd to %s",Conn.SerialNumber));
        return true;
    }

    WSConnection::~WSConnection() {
        uCentral::DeviceRegistry::Service::instance()->UnRegister(Conn.SerialNumber,this);
        SocketReactor_.removeEventHandler(WS_,Poco::NObserver<WSConnection,Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
        SocketReactor_.removeEventHandler(WS_,Poco::NObserver<WSConnection,Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
        WS_.shutdown();
    }

    void WSRequestHandler::handleRequest(HTTPServerRequest &Request, HTTPServerResponse &Response) {

        Poco::Logger & Logger = Service::instance()->logger();
        std::string Address;

        auto NewWS = new WSConnection(Reactor_,Logger,Request,Response);
        Reactor_.addEventHandler(NewWS->WS(),Poco::NObserver<WSConnection,Poco::Net::ReadableNotification>(*NewWS,&WSConnection::OnSocketReadable));
        Reactor_.addEventHandler(NewWS->WS(),Poco::NObserver<WSConnection,Poco::Net::ShutdownNotification>(*NewWS,&WSConnection::OnSocketShutdown));
    }

    HTTPRequestHandler *WSRequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request) {

        Poco::Logger & Logger = Service::instance()->logger();

        Logger.information(Poco::format("%s from %s: %s",request.getMethod(),
                                        request.clientAddress().toString(),
                                        request.getURI()));

        for (const auto & it: request)
            Logger.information(it.first + ": " + it.second);

        if (request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
            return new WSRequestHandler(Logger,SocketReactor_);

        return nullptr;
    }

};      //namespace
