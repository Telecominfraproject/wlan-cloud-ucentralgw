//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralWebSocketServer.h"
#include "uStorageService.h"

namespace uCentral::WebSocket {

    Service *Service::instance_ = nullptr;

    int Start() {
        return uCentral::WebSocket::Service::instance()->Start();
    }

    void Stop() {
        uCentral::WebSocket::Service::instance()->Stop();
    }

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

        if( SerialNumber_.empty() ) {
            SerialNumber_ = ds["serial"].toString();
            Conn_ = uCentral::DeviceRegistry::Register(SerialNumber_, this);
        }

        if(Conn_!= nullptr) {
            if(Conn_->Address.empty())
                Conn_->Address = WS_.peerAddress().toString();

            if (ds.contains("state") && ds.contains("serial")) {
                Logger_.information(SerialNumber_ + ": updating statistics.");
                std::string NewStatistics{ds["state"].toString()};
                uCentral::Storage::AddStatisticsData(SerialNumber_, Conn_->UUID, NewStatistics);
                uCentral::DeviceRegistry::SetStatistics(SerialNumber_, NewStatistics);
            } else if (ds.contains("capab") && ds.contains("serial")) {
                std::string Log{"Updating capabilities."};
                uCentral::Storage::AddLog(SerialNumber_, Log);
                std::string NewCapabilities{ds["capab"].toString()};
                uCentral::Storage::UpdateDeviceCapabilities(SerialNumber_, NewCapabilities);
            } else if (ds.contains("uuid") && ds.contains("serial") && ds.contains("active")) {
                Conn_->UUID = ds["uuid"];
                uint64_t Active = ds["active"];
                std::string Log = Poco::format("Waiting to apply configuration from %Lu to %Lu.", Active, Conn_->UUID);
                uCentral::Storage::AddLog(SerialNumber_, Log);
            } else if (ds.contains("uuid") && ds.contains("serial")) {
                Conn_->UUID = ds["uuid"];
                std::string NewConfig;
                uint64_t NewConfigUUID;

                if (uCentral::Storage::ExistingConfiguration(SerialNumber_, Conn_->UUID,
                                                                                  NewConfig, NewConfigUUID)) {
                    if (Conn_->UUID < NewConfigUUID) {
                        std::string Log = Poco::format("Returning newer configuration %Lu.", Conn_->UUID);
                        uCentral::Storage::AddLog(SerialNumber_, Log);
                        Response = "{ \"cfg\" : " + NewConfig + "}";
                    }
                }
            } else if (ds.contains("log")) {
                auto log = ds["log"].toString();
                uCentral::Storage::AddLog(SerialNumber_, log);
            } else {
                std::cout << "UNKNOWN_MESSAGE(" << SerialNumber_ << "): " << IncomingMessage_ << std::endl;
            }
        }
    }

    void WSConnection::OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf)
    {
        int flags, Op;
        int IncomingSize = 0;

        std::lock_guard<std::mutex> guard(mutex_);
        memset(IncomingMessage_, 0, sizeof(IncomingMessage_));

        try {
            IncomingSize = WS_.receiveFrame(IncomingMessage_,sizeof(IncomingMessage_), flags);
            Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

            if(IncomingSize==0 && flags == 0 && Op == 0)
            {
                delete this;
                return;
            }

            switch (Op) {
                case Poco::Net::WebSocket::FRAME_OP_PING: {
                    Logger_.information("PING(" + SerialNumber_ + "): received.");
                    WS_.sendFrame("", 0, Poco::Net::WebSocket::FRAME_OP_PONG | Poco::Net::WebSocket::FRAME_FLAG_FIN);
                    }
                    break;

                case Poco::Net::WebSocket::FRAME_OP_PONG: {
                    Logger_.information("PONG(" + SerialNumber_ + "): received.");
                    }
                    break;

                case Poco::Net::WebSocket::FRAME_OP_TEXT: {
                        std::cout << "Incoming(" << SerialNumber_ << "): " << IncomingSize << " bytes." << std::endl;
                        Logger_.debug(
                                Poco::format("Frame received (length=%d, flags=0x%x).", IncomingSize, unsigned(flags)));

                        std::string ResponseDocument;
                        ProcessMessage(ResponseDocument);

                        if(Conn_!= nullptr)
                                Conn_->RX += IncomingSize;

                        if (!ResponseDocument.empty()) {
                            if(Conn_!= nullptr)
                                Conn_->TX += ResponseDocument.size();
                            std::cout << "Returning(" << SerialNumber_ << "): " << ResponseDocument.size() << " bytes"
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
                    break;
            }

            if(Conn_!= nullptr)
                Conn_->MessageCount++;
        }
        catch (const Poco::Exception &exc) {
            std::cout << "Caught a more generic Poco exception: " << exc.message() << "Message:" << IncomingMessage_ << std::endl;
            delete this;
        }
    }

    bool WSConnection::SendCommand(const std::string &Cmd) {
        std::lock_guard<std::mutex> guard(mutex_);

        Logger_.information(Poco::format("Sending commnd to %s",SerialNumber_.c_str()));
        return true;
    }

    WSConnection::~WSConnection() {
        uCentral::DeviceRegistry::UnRegister(SerialNumber_,this);
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
