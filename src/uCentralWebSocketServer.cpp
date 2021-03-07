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
            uCentral::Storage::Service::instance()->AddStatisticsData(Conn.SerialNumber, Conn.UUID,
                                                                      NewStatistics);
        } else if (ds.contains("capab") && ds.contains("serial")) {
            Logger_.information(Conn.SerialNumber + ": updating capabilities.");
            std::string NewCapabilities{ds["capab"].toString()};
            uCentral::Storage::Service::instance()->UpdateDeviceCapabilities(Conn.SerialNumber, NewCapabilities);
        } else if (ds.contains("uuid") && ds.contains("serial") && ds.contains("active")) {
            Logger_.information(Conn.SerialNumber + ": updating active configuration.");
            Conn.UUID = ds["uuid"];
            std::cout << "Waiting to apply configuration " << ds["active"].toString() << std::endl;
        } else if (ds.contains("uuid") && ds.contains("serial")) {
            Logger_.information(Conn.SerialNumber + ": configuration check.");
            Conn.UUID = ds["uuid"];

            std::string NewConfig;
            uint64_t NewConfigUUID;

            if (uCentral::Storage::Service::instance()->ExistingConfiguration(Conn.SerialNumber, Conn.UUID,
                                                                              NewConfig, NewConfigUUID)) {
                if (Conn.UUID < NewConfigUUID) {
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

            Conn.MessageCount++;

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
