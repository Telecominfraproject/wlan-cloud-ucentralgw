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

    bool WSConnection::LookForUpgrade(std::string &Response) {

        std::string NewConfig;
        uint64_t NewConfigUUID;

        if (uCentral::Storage::ExistingConfiguration(SerialNumber_, Conn_->UUID,
                                                     NewConfig, NewConfigUUID)) {
            if (Conn_->UUID < NewConfigUUID) {
                if(Conn_->Protocol == DeviceRegistry::jsonrpc) {
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
            }
            return true;
        }

        return false;
    }


    void WSConnection::ProcessJSONRPCMessage(Poco::DynamicStruct &ds, std::string &Response) {

        if(ds.contains("method") && ds.contains("params"))
        {
            std::string Method = ds["method"].toString();

            if(Method=="connect") {
                Poco::DynamicStruct collection = ds["params"].extract<Poco::DynamicStruct>();
                Poco::DynamicStruct c2 = collection["params"].extract<Poco::DynamicStruct>();
                auto Serial = c2["serial"].toString();
                auto UUID = c2["uuid"];
                auto Firmware = c2["firmware"].toString();
                auto Capabilities = c2["capabilities"].toString();

                Conn_ = uCentral::DeviceRegistry::Register(Serial, this);
                SerialNumber_ = Serial;
                Conn_->SerialNumber = Serial;
                Conn_->UUID = UUID;
                Conn_->Firmware = Firmware;
                Conn_->PendingUUID = 0;
                Conn_->Protocol = DeviceRegistry::jsonrpc;

                uCentral::Storage::UpdateDeviceCapabilities(SerialNumber_, Capabilities);

                if(uCentral::instance()->AutoProvisioning() && !uCentral::Storage::DeviceExists(SerialNumber_))
                    uCentral::Storage::CreateDefaultDevice(SerialNumber_,Capabilities);

                LookForUpgrade(Response );

            } else if (Method=="state") {
                Poco::DynamicStruct collection = ds["params"].extract<Poco::DynamicStruct>();
                Poco::DynamicStruct c2 = collection["params"].extract<Poco::DynamicStruct>();
                auto Serial = c2["serial"].toString();
                auto UUID = c2["uuid"];
                auto State = c2["state"].toString();

                Conn_->UUID = UUID;

                uCentral::Storage::AddStatisticsData(Serial, UUID, State);
                uCentral::DeviceRegistry::SetStatistics(Serial, State);

                LookForUpgrade(Response );

            } else if (Method=="healthcheck") {
                Poco::DynamicStruct collection = ds["params"].extract<Poco::DynamicStruct>();
                Poco::DynamicStruct c2 = collection["params"].extract<Poco::DynamicStruct>();
                auto Serial = c2["serial"].toString();
                auto UUID = c2["uuid"];
                auto Sanity = c2["sanity"];
                auto CheckData = c2["data"].toString();

                Conn_->UUID = UUID;

                uCentralHealthcheck Check;

                Check.Recorded = time(nullptr);
                Check.UUID = UUID;
                Check.Data = CheckData;
                Check.Sanity = Sanity;

                uCentral::Storage::AddHealthCheckData(Serial,Check);
                LookForUpgrade(Response );

            } else if (Method=="log") {
                Poco::DynamicStruct collection = ds["params"].extract<Poco::DynamicStruct>();
                Poco::DynamicStruct c2 = collection["params"].extract<Poco::DynamicStruct>();
                auto Serial = c2["serial"].toString();
                auto Log = c2["log"].toString();
                auto Data = c2["data"].toString();
                auto Severity = c2["severity"];

                uCentralDeviceLog   DeviceLog;

                DeviceLog.Log = Log;
                DeviceLog.Data = Data;
                DeviceLog.Severity = Severity;
                DeviceLog.Recorded = time(nullptr);

                uCentral::Storage::AddLog(Serial, DeviceLog);
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

    void WSConnection::ProcessLegacyMessage(Poco::DynamicStruct &ds, std::string &Response) {
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

                if(uCentral::instance()->AutoProvisioning() && !uCentral::Storage::DeviceExists(SerialNumber_))
                {
                    uCentral::Storage::CreateDefaultDevice(SerialNumber_,NewCapabilities);
                }

            } else if (ds.contains("uuid") && ds.contains("serial") && ds.contains("active")) {
                Conn_->UUID = ds["uuid"];
                uint64_t Active = ds["active"];
                std::string Log = Poco::format("Waiting to apply configuration from %Lu to %Lu.", Active, Conn_->UUID);
                uCentral::Storage::AddLog(SerialNumber_, Log);
            } else if (ds.contains("uuid") && ds.contains("serial")) {
                Conn_->UUID = ds["uuid"];
                LookForUpgrade(Response);
            } else if (ds.contains("log")) {
                auto log = ds["log"].toString();
                uCentral::Storage::AddLog(SerialNumber_, log);
            } else {
                std::cout << "UNKNOWN_MESSAGE(" << SerialNumber_ << ")" << std::endl;
            }
        }
    }

    void WSConnection::OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf)
    {
        int flags, Op;
        int IncomingSize = 0;

        char IncomingMessage_[16000] = {0};

        std::lock_guard<std::mutex> guard(mutex_);

        try {
            IncomingSize = WS_.receiveFrame(IncomingMessage_,sizeof(IncomingMessage_), flags);
            Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

            // std::cout << "Received incoming message: " << IncomingMessage_ << std::endl;

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
                        Logger_.debug(
                                Poco::format("Frame received (length=%d, flags=0x%x).", IncomingSize, unsigned(flags)));

                        std::string ResponseDocument;
                        Parser parser;

                        try {
                            auto result = parser.parse(IncomingMessage_);
                            auto object = result.extract<Poco::JSON::Object::Ptr>();
                            Poco::DynamicStruct ds = *object;

                            uCentral::DeviceRegistry::ConnectionType Protocol;
                            if (ds.contains("jsonrpc")) {
                                Protocol = DeviceRegistry::jsonrpc;
                                ProcessJSONRPCMessage(ds, ResponseDocument);
                            } else {
                                Protocol = DeviceRegistry::legacy;
                                ProcessLegacyMessage(ds, ResponseDocument);
                            }

                            if (Conn_ != nullptr) {
                                Conn_->Protocol = Protocol;
                                Conn_->RX += IncomingSize;
                            }

                            if (!ResponseDocument.empty()) {
                                if (Conn_ != nullptr)
                                    Conn_->TX += ResponseDocument.size();
                                WS_.sendFrame(ResponseDocument.c_str(), ResponseDocument.size());
                            }
                        }
                        catch (const Poco::Exception & E) {
                            if((Conn_!=nullptr) && (Conn_->Protocol==DeviceRegistry::jsonrpc))
                            {
                                char Response[]={"{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32700, \"message\": \"Parse error\"}, \"id\": null}\""};
                                WS_.sendFrame(Response,sizeof(Response));
                            }
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
        catch (const Poco::Exception &E) {
            Logger_.warning( Poco::format("%s: Caught a more generic Poco exception: %s. Message: %s", SerialNumber_.c_str(), E.displayText().c_str(), IncomingMessage_ ));
            delete this;
        }
    }

    bool WSConnection::SendCommand(const std::string &Cmd) {
        std::lock_guard<std::mutex> guard(mutex_);

        Logger_.information(Poco::format("Sending command to %s",SerialNumber_.c_str()));
        return true;
    }

    WSConnection::~WSConnection() {
        uCentral::DeviceRegistry::UnRegister(SerialNumber_,this);
        Reactor_->Reactor()->removeEventHandler(WS_,
                                          Poco::NObserver<WSConnection,Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
        Reactor_->Reactor()->removeEventHandler(WS_,
                                          Poco::NObserver<WSConnection,Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
        WS_.shutdown();
    }

    void WSRequestHandler::handleRequest(HTTPServerRequest &Request, HTTPServerResponse &Response) {
        Poco::Logger & Logger = Service::instance()->logger();
        new WSConnection(Logger,Request,Response);
    }

    HTTPRequestHandler *WSRequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request) {

        Poco::Logger & Logger = Service::instance()->logger();

        Logger.information(Poco::format("%s from %s: %s",request.getMethod(),
                                        request.clientAddress().toString(),
                                        request.getURI()));

        for (const auto & it: request)
            Logger.information(it.first + ": " + it.second);

        if (request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
            return new WSRequestHandler(Logger);

        return nullptr;
    }

};      //namespace
