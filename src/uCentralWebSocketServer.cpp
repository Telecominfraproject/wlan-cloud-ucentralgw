//
// Created by stephane bourque on 2021-02-28.
//
#include <thread>
#include <mutex>

#include "uCentralWebSocketServer.h"
#include "uStorageService.h"
#include "uAuthService.h"
#include "uCentral.h"

#include "Poco/Net/IPAddress.h"
#include "Poco/Net/SSLException.h"
#include "Poco/Net/HTTPServerSession.h"
#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/Net/HTTPServerRequestImpl.h"
#include "Poco/JSON/Array.h"
#include "Poco/zlib.h"
#include "base64util.h"


namespace uCentral::WebSocket {

    Service *Service::instance_ = nullptr;

    int Start() {
        return uCentral::WebSocket::Service::instance()->Start();
    }

    void Stop() {
        uCentral::WebSocket::Service::instance()->Stop();
    }

    Service::Service() noexcept:
            SubSystemServer("WebSocketServer", "WS-SVR", "ucentral.websocket"),
            Factory_(Logger_)
    {

    }

    int Service::Start() {

        for(const auto & Svr : ConfigServersList_ ) {
            Logger_.information(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.address(), std::to_string(Svr.port()),
											 Svr.key_file(),Svr.cert_file()));

            auto Sock{Svr.CreateSecureSocket()};
            auto NewSocketReactor = std::make_unique<Poco::Net::SocketReactor>();
            auto NewSocketAcceptor = std::make_unique<Poco::Net::SocketAcceptor<WSConnection>>( Sock, *NewSocketReactor);
            auto NewThread = std::make_unique<Poco::Thread>();
            NewThread->setName("WebSocketAcceptor."+Svr.address()+":"+std::to_string(Svr.port()));
            NewThread->start(*NewSocketReactor);

            WebSocketServerEntry WSE { .SocketReactor{std::move(NewSocketReactor)} ,
                                       .SocketAcceptor{std::move(NewSocketAcceptor)} ,
                                       .SocketReactorThread{std::move(NewThread)}};
            Servers_.push_back(std::move(WSE));
        }

        uint64_t MaxThreads = uCentral::ServiceConfig::getInt("ucentral.websocket.maxreactors",5);
        Factory_.Init(MaxThreads);

        return 0;
    }

    void Service::Stop() {
        Logger_.information("Stopping reactors...");

        for(auto const &Svr : Servers_) {
            Svr.SocketReactor->stop();
            Svr.SocketReactorThread->join();
        }
        Factory_.Close();
    }

    CountedReactor::CountedReactor()
    {
        Reactor_ = Service::instance()->GetAReactor();
    };

    CountedReactor::~CountedReactor()
    {
        Reactor_->Release();
    }

    WSConnection::WSConnection(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor):
            Socket_(socket),
            ParentAcceptorReactor_(reactor),
            Logger_(Service::instance()->Logger())
    {
        auto Params = Poco::AutoPtr<Poco::Net::HTTPServerParams>(new Poco::Net::HTTPServerParams());
        Poco::Net::HTTPServerSession        Session(Socket_, Params);
        Poco::Net::HTTPServerResponseImpl   Response(Session);
        Poco::Net::HTTPServerRequestImpl    Request(Response,Session,Params);

        auto Now = time(nullptr);
        Response.setDate(Now);
        Response.setVersion(Request.getVersion());
        Response.setKeepAlive(Params->getKeepAlive() && Request.getKeepAlive() && Session.canKeepAlive());
        WS_ = std::make_unique<Poco::Net::WebSocket>(Request, Response);
        Register();
    }

    WSConnection::~WSConnection() {
        uCentral::DeviceRegistry::UnRegister(SerialNumber_,this);
        DeRegister();
    }

    void WSConnection::Register() {
        std::lock_guard<std::mutex> guard(Mutex_);
        if(!Registered_ && WS_)
        {
            auto TS = Poco::Timespan();

            WS_->setReceiveTimeout(TS);
            WS_->setNoDelay(true);
            WS_->setKeepAlive(true);
            Reactor_.Reactor()->addEventHandler(*WS_,
                                                 Poco::NObserver<WSConnection,
                                                 Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
/*            Reactor_.Reactor()->addEventHandler(*WS_,
                                                 Poco::NObserver<WSConnection,
                                                 Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
              Reactor_.Reactor()->addEventHandler(*WS_,
                                                 Poco::NObserver<WSConnection,
                                                 Poco::Net::ErrorNotification>(*this,&WSConnection::OnSocketError));
*/
            Registered_ = true ;
            WSup_ = true;
        }
    }

    void WSConnection::DeRegister() {
        std::lock_guard<std::mutex> guard(Mutex_);
        if(Registered_ && WS_)
        {
            Reactor_.Reactor()->removeEventHandler(*WS_,
                                                    Poco::NObserver<WSConnection,
                                                    Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
/*          Reactor_.Reactor()->removeEventHandler(*WS_,
                                                    Poco::NObserver<WSConnection,
                                                    Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
            Reactor_.Reactor()->removeEventHandler(*WS_,
                                                    Poco::NObserver<WSConnection,
                                                    Poco::Net::ErrorNotification>(*this,&WSConnection::OnSocketError));

            if(WS_ && WSup_)
                WS_->shutdown();
*/
            (*WS_).close();
            Registered_ = false ;
        }
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

                Poco::JSON::Parser  parser;
                auto ParsedConfig = parser.parse(NewConfig).extract<Poco::JSON::Object::Ptr>();
                Poco::DynamicStruct Vars = *ParsedConfig;
                Vars["uuid"] = NewConfigUUID;

                Poco::JSON::Object Params;
                Params.set("serial", SerialNumber_);
                Params.set("uuid", NewConfigUUID);
                Params.set("when", 0);
                Params.set("config", Vars);

                uint64_t CommandID = RPC_++;

                Poco::JSON::Object ReturnObject;
                ReturnObject.set("method", "configure");
                ReturnObject.set("jsonrpc", "2.0");
                ReturnObject.set("id", CommandID);
                ReturnObject.set("params", Params);

                std::stringstream Ret;
                Poco::JSON::Stringifier::condense(ReturnObject, Ret);

                Response = Ret.str();

                // create the command stub...
                uCentralCommandDetails  Cmd;

                Cmd.SerialNumber = SerialNumber_;
                Cmd.UUID = uCentral::instance()->CreateUUID();
                Cmd.SubmittedBy = "*system";
                Cmd.ErrorCode = 0 ;
                Cmd.Status = "Pending";
                Cmd.Command = "configure";
                Cmd.Custom = 0;
                Cmd.RunAt = 0;

                std::stringstream ParamStream;
                Params.stringify(ParamStream);
                Cmd.Details = ParamStream.str();
                if(!uCentral::Storage::AddCommand(SerialNumber_,Cmd,true))
                {
                    Logger_.warning(Poco::format("Could not submit configure command for %s",SerialNumber_));
                } else {
                    Logger_.information(Poco::format("Submitted configure command %Lu for %s",CommandID, SerialNumber_));
                    RPCs_[CommandID] = Cmd.UUID;
                }

                return true;
            }
        }

        return false;
    }

    Poco::DynamicStruct WSConnection::ExtractCompressedData(const std::string & CompressedData)
    {
        std::vector<uint8_t> OB = base64::decode(CompressedData);

        unsigned long MaxSize=OB.size()*10;
        std::vector<char> UncompressedBuffer(MaxSize);
        unsigned long FinalSize = MaxSize;
        uncompress((Bytef *)&UncompressedBuffer[0], & FinalSize, (Bytef *)&OB[0],OB.size());

        UncompressedBuffer[FinalSize]=0;
        Poco::JSON::Parser parser;
        auto result = parser.parse(&UncompressedBuffer[0]).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct Vars = *result;

        return Vars;
    }

    void WSConnection::ProcessJSONRPCResult(Poco::DynamicStruct Vars) {
        uint64_t ID = Vars["id"];
        auto RPC = RPCs_.find(ID);

        if(RPC!=RPCs_.end())
        {
            Logger_.information(Poco::format("RPC(%s): Completed outstanding RPC %Lu",SerialNumber_,ID));
            uCentral::Storage::CommandCompleted(RPC->second,Vars);
        }
        else
        {
            Logger_.warning(Poco::format("RPC(%s): Could not find outstanding RPC %Lu",SerialNumber_,ID));
        }
    }

    void WSConnection::ProcessJSONRPCEvent(Poco::DynamicStruct Vars) {

        std::string Response;

        auto Method = Vars["method"].toString();
        auto Params = Vars["params"];

        if(!Params.isStruct())
        {
            Logger_.warning(Poco::format("MISSING-PARAMS(%s): params must be an object.",SerialNumber_));
            return;
        }

        //  expand params if necessary
        Poco::DynamicStruct ParamsObj = Params.extract<Poco::DynamicStruct>();
        if(ParamsObj.contains("compress_64"))
        {
            Logger_.information(Poco::format("EVENT(%s): Found compressed paylod.",SerialNumber_));
            ParamsObj = ExtractCompressedData(ParamsObj["compress_64"].toString());
        }

        if(!ParamsObj.contains("serial"))
        {
            Logger_.warning(Poco::format("MISSING-PARAMS(%s): Serial number is missing in message.",SerialNumber_));
            return;
        }
        auto Serial = ParamsObj["serial"].toString();

        if (!Poco::icompare(Method, "connect")) {
            if( ParamsObj.contains("uuid") &&
                ParamsObj.contains("firmware") &&
                ParamsObj.contains("capabilities")) {
                uint64_t UUID = ParamsObj["uuid"];
                auto Firmware = ParamsObj["firmware"].toString();
                auto Capabilities = ParamsObj["capabilities"].toString();

                Logger_.information(Poco::format("CONNECT(%s): UUID=%Lu Starting.", Serial, UUID));

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
            } else {
                Logger_.warning(Poco::format("CONNECT(%s): Missing one of uuid, firmware, or capabilities",SerialNumber_));
                return;
            }
        } else if (!Poco::icompare(Method, "state")) {
             if (ParamsObj.contains("uuid") &&
                ParamsObj.contains("state")) {

                uint64_t UUID = ParamsObj["uuid"];
                auto State = ParamsObj["state"].toString();

                Logger_.information(Poco::format("STATE(%s): UUID=%Lu Updating.", Serial, UUID));
                Conn_->UUID = UUID;
                uCentral::Storage::AddStatisticsData(Serial, UUID, State);
                uCentral::DeviceRegistry::SetStatistics(Serial, State);
                LookForUpgrade(Response);
            } else {
                Logger_.warning(Poco::format("STATE(%s): Invalid request. Missing serial, uuid, or state",
                                             SerialNumber_));
            }
        } else if (!Poco::icompare(Method, "healthcheck")) {
            if( ParamsObj.contains("uuid") &&
                ParamsObj.contains("sanity") &&
                ParamsObj.contains("data"))
            {
                uint64_t UUID = ParamsObj["uuid"];
                auto Sanity = ParamsObj["sanity"];
                auto CheckData = ParamsObj["data"].toString();

                Logger_.information(Poco::format("HEALTHCHECK(%s): UUID=%Lu. Updating: Data=%s", Serial, UUID, CheckData));

                Conn_->UUID = UUID;

                uCentralHealthcheck Check;

                Check.Recorded = time(nullptr);
                Check.UUID = UUID;
                Check.Data = CheckData;
                Check.Sanity = Sanity;

                uCentral::Storage::AddHealthCheckData(Serial, Check);
                LookForUpgrade(Response);
            }
            else
            {
                Logger_.warning(Poco::format("HEALTHCHECK(%s): Missing parameter",SerialNumber_));
                return;
            }
        } else if (!Poco::icompare(Method, "log")) {
            if( ParamsObj.contains("log") &&
                ParamsObj.contains("severity")) {

                Logger_.information(Poco::format("LOG(%s): new entry.", Serial));

                auto Log = ParamsObj["log"].toString();
                auto Severity = ParamsObj["severity"];

                std::string Data;
                if (ParamsObj.contains("data"))
                    Data = ParamsObj["data"].toString();

                uCentralDeviceLog DeviceLog;

                DeviceLog.Log = Log;
                DeviceLog.Data = Data;
                DeviceLog.Severity = Severity;
                DeviceLog.Recorded = time(nullptr);

                uCentral::Storage::AddLog(Serial, DeviceLog);
            }
            else
            {
                Logger_.warning(Poco::format("LOG(%s): Missing parameters.",SerialNumber_));
                return;
            }
        } else if (!Poco::icompare(Method, "crashlog")) {
            if( ParamsObj.contains("uuid") &&
                ParamsObj.contains("loglines")) {

                Logger_.information(Poco::format("CRASH-LOG(%s): new entry.", Serial));

                auto LogLines = ParamsObj["loglines"];
                uint64_t UUID = ParamsObj["uuid"];

                if(LogLines.isArray()) {
                    auto LogLinesArray = LogLines.extract<Poco::Dynamic::Array>();

                    uCentralDeviceLog DeviceLog;
                    std::string LogText;

                    for(const auto & i : LogLinesArray)
                        LogText += i.toString() + "\r\n";

                    DeviceLog.Log = LogText;
                    DeviceLog.Data = "";
                    DeviceLog.Severity = uCentralDeviceLog::LOG_EMERG;
                    DeviceLog.Recorded = time(nullptr);
                    DeviceLog.LogType = 1;

                    uCentral::Storage::AddLog(Serial, DeviceLog, true);
                } else {
                    Logger_.warning(Poco::format("CRASH-LOG(%s): parameter loglines must be an array.",SerialNumber_));
                    return;
                }
            }
            else
            {
                Logger_.warning(Poco::format("LOG(%s): Missing parameters.",SerialNumber_));
                return;
            }
        } else if (!Poco::icompare(Method, "ping")) {
            if(ParamsObj.contains("uuid")) {
                uint64_t UUID = ParamsObj["uuid"];
                Logger_.information(Poco::format("PING(%s): Current config is %Lu", Serial, UUID));
            }
            else
            {
                Logger_.warning(Poco::format("PING(%s): Missing parameter.",SerialNumber_));
            }
        } else if (!Poco::icompare(Method, "cfgpending")) {
            if( ParamsObj.contains("uuid") &&
                ParamsObj.contains("active")) {

                uint64_t UUID = ParamsObj["uuid"];
                uint64_t Active = ParamsObj["active"];

                Logger_.information(Poco::format("CFG-PENDING(%s): Active: %Lu Target: %Lu", Serial, Active, UUID));
            }
            else {
                Logger_.warning(Poco::format("CFG-PENDING(%s): Missing some parameters",SerialNumber_));
            }
        }

        if (!Response.empty()) {
            if (Conn_ != nullptr)
                Conn_->TX += Response.size();
            Logger_.information(Poco::format("RESPONSE(%s): %s",SerialNumber_,Response));
            try {
                WS_->sendFrame(Response.c_str(), Response.size());
            }
            catch( Poco::Exception & E )
            {
                std::cout << "Caught exception while sending." << std::endl;
                delete this;
            }
        }
    }

    void WSConnection::OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
        std::lock_guard<std::mutex> guard(Mutex_);

        Logger_.information(Poco::format("SOCKET-SHUTDOWN(%s): Closing.",SerialNumber_));
        WSup_ = false;
        delete this;
    };

    void WSConnection::OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
        std::lock_guard<std::mutex> guard(Mutex_);

        std::cout << "OnSocketError" << std::endl;
        Logger_.information(Poco::format("SOCKET-ERROR(%s): Closing.",SerialNumber_));
        delete this;
    }

    void WSConnection::OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
        try
        {
//            std::thread Processor([Obj=this]() { Obj->ProcessIncomingFrame(); });
//            Processor.detach();

            ProcessIncomingFrame();
        }
        catch ( const Poco::Exception & E )
        {
            std::cout << "OnSocketReadable" << std::endl;
            delete this;
        }
    }

    void WSConnection::ProcessIncomingFrame() {
        int flags, Op;
        int IncomingSize = 0;

        bool MustDisconnect=false;

        std::array <char,BufSize>   IncomingMessage{0};
        std::string IncomingMessageStr;

        try {

            std::lock_guard<std::mutex> guard(Mutex_);

            IncomingSize = WS_->receiveFrame(&IncomingMessage[0], BufSize, flags);
            Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

            if (IncomingSize == 0 && flags == 0 && Op == 0) {
                Logger_.information(Poco::format("DISCONNECT(%s)", SerialNumber_));
                MustDisconnect = true;
            } else {
                IncomingMessage[IncomingSize]=0;
                switch (Op) {
                    case Poco::Net::WebSocket::FRAME_OP_PING: {
                        Logger_.information("WS-PING(" + SerialNumber_ + "): received. PONG sent back.");
                        WS_->sendFrame("", 0,Poco::Net::WebSocket::FRAME_OP_PONG | Poco::Net::WebSocket::FRAME_FLAG_FIN);
                        }
                        break;

                    case Poco::Net::WebSocket::FRAME_OP_PONG: {
                        Logger_.information("PONG(" + SerialNumber_ + "): received and ignored.");
                        }
                        break;

                    case Poco::Net::WebSocket::FRAME_OP_TEXT: {
                        Logger_.debug(
                                Poco::format("Frame received (length=%d, flags=0x%x).", IncomingSize, unsigned(flags)));

                        Poco::JSON::Parser parser;
                        std::string InMsg(&IncomingMessage[0]);

                        IncomingMessageStr = std::string(&IncomingMessage[0]);

                        auto ParsedMessage = parser.parse(InMsg);
                        auto Result = ParsedMessage.extract<Poco::JSON::Object::Ptr>();
                        Poco::DynamicStruct vars = *Result;

                        if (vars.contains("jsonrpc") &&
                            vars.contains("method") &&
                            vars.contains("params")) {
                            ProcessJSONRPCEvent(vars);

                        } else if (vars.contains("jsonrpc") &&
                                   vars.contains("result") &&
                                   vars.contains("id")) {
                            ProcessJSONRPCResult(vars);
                        } else {
                            Logger_.warning(Poco::format("INVALID-PAYLOAD: Payload is not JSON-RPC 2.0: %s",IncomingMessageStr));
                        }
                        break;
                    }

                    default: {
                            Logger_.warning("UNKNOWN WS Frame operation: " + std::to_string(Op));
                        }
                        break;
                    }

                    if (Conn_ != nullptr) {
                        Conn_->RX += IncomingSize;
                        Conn_->MessageCount++;
                    }
                }
            }
        catch (const Poco::Net::ConnectionResetException & E)
        {
            Logger_.warning( Poco::format("%s(%s): Caught a ConnectionResetException: %s", std::string(__func__), SerialNumber_, E.displayText()));
            MustDisconnect= true;
        }
        catch (const Poco::JSON::JSONException & E)
        {
            Logger_.warning( Poco::format("%s(%s): Caught a JSONException: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), IncomingMessageStr ));
        }
        catch (const Poco::Net::WebSocketException & E)
        {
            Logger_.warning( Poco::format("%s(%s): Caught a websocket exception: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::SSLConnectionUnexpectedlyClosedException & E)
        {
            Logger_.warning( Poco::format("%s(%s): Caught a SSLConnectionUnexpectedlyClosedException: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::SSLException & E)
        {
            Logger_.warning( Poco::format("%s(%s): Caught a SSL exception: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::NetException & E) {
            Logger_.warning( Poco::format("%s(%s): Caught a NetException: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::IOException & E) {
            Logger_.warning( Poco::format("%s(%s): Caught a IOException: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::Exception &E) {
            Logger_.warning( Poco::format("%s(%s): Caught a more generic Poco exception: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const std::exception & E) {
            Logger_.warning( Poco::format("%s(%s): Caught a std::exception: %s. Message: %s", std::string{__func__}, SerialNumber_, std::string{E.what()}, IncomingMessageStr) );
            MustDisconnect = true ;
        }

        if(!MustDisconnect)
            return;

        delete this;
    }

    bool WSConnection::SendCommand(uCentralCommandDetails & Command) {
        std::lock_guard<std::mutex> guard(Mutex_);

        Logger_.information(Poco::format("Sending command to %s",SerialNumber_));
        try {
            Poco::JSON::Object Obj;

            auto ID = RPC_++;

            Obj.set("jsonrpc","2.0");
            Obj.set("id",ID);
            Obj.set("method", Command.Custom ? "perform" : Command.Command );

            // the params section was composed earlier... just include it here
            Poco::JSON::Parser  parser;
            auto ParsedMessage = parser.parse(Command.Details);
            const auto & ParamsObj = ParsedMessage.extract<Poco::JSON::Object::Ptr>();
            Obj.set("params",ParamsObj);
            std::stringstream ToSend;
            Poco::JSON::Stringifier::stringify(Obj,ToSend);

            auto BytesSent = WS_->sendFrame(ToSend.str().c_str(),ToSend.str().size());
            if(BytesSent == ToSend.str().size())
            {
                RPCs_[ID] = Command.UUID;
                uCentral::Storage::CommandExecuted(Command.UUID);
                return true;
            }
            else
            {
                Logger_.warning(Poco::format("COMMAND(%s): Could not send the entire command.",SerialNumber_));
                return false;
            }
        }
        catch( const Poco::Exception & E )
        {
            Logger_.warning(Poco::format("COMMAND(%s): Exception while sending a command.",SerialNumber_));
        }
        return false;
    }
};      //namespace
