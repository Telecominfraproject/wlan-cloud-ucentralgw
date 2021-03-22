//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralWebSocketServer.h"
#include "uStorageService.h"
#include "uAuthService.h"
#include "uCentral.h"
#include "Poco/Net/IPAddress.h"
#include "Poco/Net/SocketAddress.h"

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

        for(const auto & Svr : ConfigServersList_ ) {
            std::string l{
                    "Starting: " + Svr.address() + ":" + std::to_string(Svr.port()) +
                    " key:" + Svr.key_file() +
                    " cert:" + Svr.cert_file()};

            Logger_.information(l);

            std::shared_ptr<SecureServerSocket> Sock = Svr.CreateSecureSocket();

            auto NewServer = std::shared_ptr<Poco::Net::HTTPServer>(new Poco::Net::HTTPServer(new WSRequestHandlerFactory, *Sock, new HTTPServerParams));
            NewServer->start();
            HTTPServers_.push_back(NewServer);
        }

        uint64_t MaxThreads = uCentral::ServiceConfig::getInt("ucentral.websocket.maxreactors",5);

        Factory_ = std::shared_ptr<CountedSocketReactorFactory>(new CountedSocketReactorFactory(MaxThreads));

        return 0;
    }

    void Service::Stop() {
        SubSystemServer::logger().information("Stopping ");

        for(auto const & svr : HTTPServers_)
            svr->stop();
    }

    WSConnection::WSConnection(Poco::Logger   & Logger,
            HTTPServerRequest & Request,
            HTTPServerResponse & Response ):
                Logger_(Logger),
                Conn_(nullptr),
                RPC_(0)
    {
        WS_ = new Poco::Net::WebSocket(Request, Response);
        Reactor_ = std::shared_ptr<CountedReactor>(new CountedReactor(Service::instance()->GetAReactor()));
        Reactor_->Reactor()->addEventHandler(*WS_,
                                             Poco::NObserver<WSConnection, Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
        Reactor_->Reactor()->addEventHandler(*WS_,
                                             Poco::NObserver<WSConnection, Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
        Reactor_->Reactor()->addEventHandler(*WS_,
                                             Poco::NObserver<WSConnection, Poco::Net::ErrorNotification>(*this,&WSConnection::OnSocketError));
        auto TS = Poco::Timespan(90,0);
        WS_->setReceiveTimeout(TS);
        WS_->setNoDelay(true);
        WS_->setKeepAlive(true);
    }

    WSConnection::~WSConnection() {
        uCentral::DeviceRegistry::UnRegister(SerialNumber_,this);
        Reactor_->Reactor()->removeEventHandler(*WS_,
                                                Poco::NObserver<WSConnection,Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
        Reactor_->Reactor()->removeEventHandler(*WS_,
                                                Poco::NObserver<WSConnection,Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
        Reactor_->Reactor()->removeEventHandler(*WS_,
                                                Poco::NObserver<WSConnection,Poco::Net::ErrorNotification>(*this,&WSConnection::OnSocketError));
        WS_->shutdown();
        delete WS_;
    }

    bool WSConnection::LookForUpgrade(std::string &Response) {

        std::string NewConfig;
        uint64_t NewConfigUUID;

        if (uCentral::Storage::ExistingConfiguration(SerialNumber_, Conn_->UUID,
                                                     NewConfig, NewConfigUUID)) {
            if (Conn_->UUID < NewConfigUUID) {
                Conn_->PendingUUID = NewConfigUUID;
                std::string Log = Poco::format("Returning newer configuration %Lu.", Conn_->PendingUUID);
                uCentral::Storage::AddLog(SerialNumber_, Log);

                Poco::JSON::Object Params;
                Params.set("serial", SerialNumber_);
                Params.set("uuid", NewConfigUUID);
                Params.set("when", 0);
                Params.set("config", NewConfig);

                Poco::JSON::Object ReturnObject;
                ReturnObject.set("method", "configure");
                ReturnObject.set("jsonrpc", "2.0");
                ReturnObject.set("id", RPC_++);
                ReturnObject.set("params", Params);

                std::stringstream Ret;
                Poco::JSON::Stringifier::condense(ReturnObject, Ret);

                Response = Ret.str();
            }
            else
            {
                Conn_->PendingUUID = NewConfigUUID;
                std::string Log = Poco::format("Returning newer configuration %Lu.", Conn_->PendingUUID);
                uCentral::Storage::AddLog(SerialNumber_, Log);
                Response = "{ \"cfg\" : " + NewConfig + "}";
            }
            return true;
        }

        return false;
    }

    void WSConnection::ProcessJSONRPCMessage(Poco::DynamicStruct &ds, std::string &Response) {

        if(ds.contains("method") && ds.contains("params"))
        {
            std::string Method = ds["method"].toString();
            auto Params = ds["params"];

            if(!Poco::icompare(Method,"connect")) {
                try {
                    auto Serial = Params["serial"].toString();
                    auto UUID = Params["uuid"];
                    auto Firmware = Params["firmware"].toString();
                    auto Capabilities = Params["capabilities"].toString();

                    Logger_.information(Poco::format("CONNECT(%s): Starting.",Serial));

                    Conn_ = uCentral::DeviceRegistry::Register(Serial, this);
                    SerialNumber_ = Serial;
                    Conn_->SerialNumber = Serial;
                    Conn_->UUID = UUID;
                    Conn_->Firmware = Firmware;
                    Conn_->PendingUUID = 0;

                    uCentral::Storage::UpdateDeviceCapabilities(SerialNumber_, Capabilities);

                    if (uCentral::instance()->AutoProvisioning() && !uCentral::Storage::DeviceExists(SerialNumber_))
                        uCentral::Storage::CreateDefaultDevice(SerialNumber_, Capabilities);

                    LookForUpgrade(Response);
                }
                catch ( const Poco::Exception & E)
                {
                    Logger_.warning(Poco::format("CONNECT: Invalid payload. Error: %s",E.displayText()));
                }
            } else if (!Poco::icompare(Method,"state")) {
                try {
                    auto Serial = Params["serial"].toString();
                    auto UUID = Params["uuid"];
                    auto State = Params["state"].toString();

                    Logger_.information(Poco::format("STATE(%s): Updating.", Serial));

                    Conn_->UUID = UUID;

                    uCentral::Storage::AddStatisticsData(Serial, UUID, State);
                    uCentral::DeviceRegistry::SetStatistics(Serial, State);

                    LookForUpgrade(Response);
                }
                catch( const Poco::Exception & E )
                {
                    Logger_.warning(Poco::format("STATE: Invalid payload. Error: %s",E.displayText()));
                }
            } else if (!Poco::icompare(Method,"healthcheck")) {
                try {
                    auto Serial = Params["serial"].toString();
                    auto UUID = Params["uuid"];
                    auto Sanity = Params["sanity"];
                    auto CheckData = Params["data"].toString();

                    Logger_.information(Poco::format("HEALTHCHECK(%s): Updating", Serial));

                    Conn_->UUID = UUID;

                    uCentralHealthcheck Check;

                    Check.Recorded = time(nullptr);
                    Check.UUID = UUID;
                    Check.Data = CheckData;
                    Check.Sanity = Sanity;

                    uCentral::Storage::AddHealthCheckData(Serial, Check);
                    LookForUpgrade(Response);
                }
                catch( const Poco::Exception & E )
                {
                    Logger_.warning(Poco::format("HEALTHCHECK: Invalid payload. Error: %s",E.displayText()));
                }
            } else if (!Poco::icompare(Method,"log")) {
                try {
                    auto Serial = Params["serial"].toString();
                    auto Log = Params["log"].toString();
                    std::string Data;
                    if(ds.contains("data"))
                        Data = Params["data"].toString();
                    auto Severity = Params["severity"];

                    uCentralDeviceLog DeviceLog;

                    DeviceLog.Log = Log;
                    DeviceLog.Data = Data;
                    DeviceLog.Severity = Severity;
                    DeviceLog.Recorded = time(nullptr);

                    uCentral::Storage::AddLog(Serial, DeviceLog);
                }
                catch( const Poco::Exception & E )
                {
                    Logger_.warning(Poco::format("LOG: Invalid payload. Error: %s",E.displayText()));
                }
            } else if (!Poco::icompare(Method,"ping")) {
                try {
                    auto Serial = Params["serial"].toString();
                    uint64_t UUID = Params["uuid"];

                    Logger_.information(Poco::format("PING(%s): Current config is %Lu", Serial, UUID));

                }
                catch( const Poco::Exception & E )
                {
                    Logger_.warning(Poco::format("PING: Invalid payload. Error: %s",E.displayText()));
                }
            } else if (!Poco::icompare(Method,"cfgpending")) {
                try {
                    auto Serial = Params["serial"].toString();
                    uint64_t UUID = Params["uuid"];
                    uint64_t Active = Params["active"];

                    Logger_.information(Poco::format("CFG-PENDING(%s): Active: %Lu Target: %Lu", Serial, Active, UUID));
                }
                catch( const Poco::Exception & E )
                {
                    Logger_.warning(Poco::format("CFG-PENDING: Invalid payload. Error: %s",E.displayText()));
                }
            }
            else
            {
                Response = "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32601, \"message\": \"Method not found\"}, \"id\": \"1\"}";
            }

        } else if (ds.contains("result") && ds.contains("id"))
        {

        }
        else
        {
            Response = "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32601, \"message\": \"Method not found\"}, \"id\": \"1\"}";
        }
    }

    void WSConnection::OnSocketShutdown(const AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
        Logger_.information(Poco::format("SOCKET-SHUTDOWN(%s): Closing.",SerialNumber_));
        delete this;
    };

    void WSConnection::OnSocketError(const AutoPtr<Poco::Net::ErrorNotification>& pNf) {
        Logger_.information(Poco::format("SOCKET-ERROR(%s): Closing.",SerialNumber_));
        delete this;
    }

    void WSConnection::OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf)
    {
        int flags, Op;
        int IncomingSize = 0;

        char IncomingMessage_[16000] = {0};

        std::lock_guard<std::mutex> guard(mutex_);

        try {
            IncomingSize = WS_->receiveFrame(IncomingMessage_,sizeof(IncomingMessage_), flags);
            Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

            if(IncomingSize==0 && flags == 0 && Op == 0)
            {
                Logger_.information(Poco::format("DISCONNECT(%s)",SerialNumber_));
                delete this;
                return;
            }

            switch (Op) {
                case Poco::Net::WebSocket::FRAME_OP_PING: {
                        Logger_.information("WS-PING(" + SerialNumber_ + "): received.");
                        WS_->sendFrame("", 0, Poco::Net::WebSocket::FRAME_OP_PONG | Poco::Net::WebSocket::FRAME_FLAG_FIN);
                    }
                    break;

                case Poco::Net::WebSocket::FRAME_OP_PONG: {
                        Logger_.information("PONG(" + SerialNumber_ + "): received.");
                    }
                    break;

                case Poco::Net::WebSocket::FRAME_OP_TEXT: {
                        Logger_.debug(
                                Poco::format("Frame received (length=%d, flags=0x%x).", IncomingSize, unsigned(flags)));

                        std::string ResponseDocument;
                        Parser parser;

                        try {
                            auto result = parser.parse(IncomingMessage_);
                            auto object = result.extract<Poco::JSON::Object::Ptr>();
                            Poco::DynamicStruct ds = *object;

                            if (ds.contains("jsonrpc")) {
                                ProcessJSONRPCMessage(ds, ResponseDocument);

                                if (Conn_ != nullptr) {
                                    Conn_->RX += IncomingSize;
                                }

                                if (!ResponseDocument.empty()) {
                                    if (Conn_ != nullptr)
                                        Conn_->TX += ResponseDocument.size();
                                    WS_->sendFrame(ResponseDocument.c_str(), ResponseDocument.size());
                                }
                            }
                            else
                            {
                                Logger_.warning("INVALID-PAYLOAO: Payload is not JSON-RPC 2.0");
                            }
                        }
                        catch (const Poco::Exception & E) {
                            char Response[]={"{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32700, \"message\": \"Parse error\"}, \"id\": null}\""};
                            WS_->sendFrame(Response,sizeof(Response));
                        }
                    }
                    break;

                default: {
                        Logger_.warning("UNKNOWN WS Frame operation: " + std::to_string(Op));
                        std::cerr << "WS: Unknown frame: " << Op << " Flags: " << flags << std::endl;
                        Op = Poco::Net::WebSocket::FRAME_OP_CLOSE;
                    }
                    break;
            }

            if(Conn_!= nullptr)
                Conn_->MessageCount++;
        }
        catch (const Poco::Exception &E) {
            std::string Msg{IncomingMessage_};
            Logger_.warning( Poco::format("%s(%s): Caught a more generic Poco exception: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), Msg ));
            delete this;
        }
    }

    bool WSConnection::SendCommand(const std::string &Cmd) {
        std::lock_guard<std::mutex> guard(mutex_);

        Logger_.information(Poco::format("Sending command to %s",SerialNumber_));

        return true;
    }

    void WSRequestHandler::handleRequest(HTTPServerRequest &Request, HTTPServerResponse &Response) {
        new WSConnection(Logger_,Request,Response);
    }

    HTTPRequestHandler *WSRequestHandlerFactory::createRequestHandler(const HTTPServerRequest & Request) {

        Poco::Logger & Logger = Service::instance()->logger();

        std::string ConnectionInfo = "New connection from " + Request.clientAddress().toString();

        if ( (Request.find("Sec-WebSocket-Protocol") != Request.end() && Poco::icompare(Request["Sec-WebSocket-Protocol"], "ucentral-broker") == 0) &&
             (Request.find("Upgrade") != Request.end() && Poco::icompare(Request["Upgrade"], "websocket") == 0)) {
            Logger.information(ConnectionInfo);
            return new WSRequestHandler(Logger);
        }

        Logger.information( "REJECTED: " + ConnectionInfo);

        return nullptr;
    }

};      //namespace
