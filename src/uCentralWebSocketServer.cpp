//
// Created by stephane bourque on 2021-02-28.
//

#include "uCentralWebSocketServer.h"
#include "uStorageService.h"
#include "uAuthService.h"
#include "uCentral.h"
#include "Poco/Net/IPAddress.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SSLException.h"
#include "Poco/Net/HTTPServerSession.h"
#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/Net/HTTPServerRequestImpl.h"
#include "Poco/Base64Decoder.h"
#include "Poco/zlib.h"
#include "base64util.h"

#include <thread>
#include <mutex>

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
            std::string l{
                    "Starting: " + Svr.address() + ":" + std::to_string(Svr.port()) +
                    " key:" + Svr.key_file() +
                    " cert:" + Svr.cert_file()};

            Logger_.information(l);

            std::shared_ptr<SecureServerSocket> Sock = Svr.CreateSecureSocket();

            auto NewSocketReactor = std::shared_ptr<Poco::Net::SocketReactor>(new Poco::Net::SocketReactor);
            auto NewSocketAcceptor = std::shared_ptr<Poco::Net::SocketAcceptor<WSConnection>>( new Poco::Net::SocketAcceptor<WSConnection>(*Sock, *NewSocketReactor));
            auto NewThread = std::shared_ptr<Poco::Thread>(new Poco::Thread);

            NewThread->setName("WebSocketAcceptor."+Svr.address());

            NewThread->start(*NewSocketReactor);

            Servers_.push_back(std::make_tuple(NewThread,NewSocketReactor,NewSocketAcceptor));
        }

        uint64_t MaxThreads = uCentral::ServiceConfig::getInt("ucentral.websocket.maxreactors",5);

        Factory_.Init(MaxThreads);

        return 0;
    }

    void Service::Stop() {
        Logger_.information("Stopping reactors...");

        for(auto const &[ReactorThread,SocketReactor,SocketAcceptor] : Servers_) {
            SocketReactor->stop();
            ReactorThread->join();
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

    WSConnection::WSConnection(StreamSocket& socket, SocketReactor& reactor):
            Socket_(socket),
            ParentAcceptorReactor_(reactor),
            Logger_(Service::instance()->Logger()),
            RPC_(time(nullptr)),
            Registered_(false),
            Conn_(nullptr),
            WS_(nullptr),
            IncomingMessage_(nullptr),
            Reactor_(nullptr)
    {
        auto Params = Poco::AutoPtr<HTTPServerParams>(new HTTPServerParams());
        Poco::Net::HTTPServerSession        Session(Socket_, Params);
        Poco::Net::HTTPServerResponseImpl   Response(Session);
        Poco::Net::HTTPServerRequestImpl    Request(Response,Session,Params);

        auto Now = time(nullptr);
        Response.setDate(Now);
        Response.setVersion(Request.getVersion());
        Response.setKeepAlive(Params->getKeepAlive() && Request.getKeepAlive() && Session.canKeepAlive());
//        try {
            WS_ = std::shared_ptr<Poco::Net::WebSocket>(new Poco::Net::WebSocket(Request, Response));
            IncomingMessage_ = std::shared_ptr<char>(new char [BufSize]);
            Reactor_ = std::shared_ptr<CountedReactor>(new CountedReactor);
            Register();
/*        }
        catch(const Poco::Exception &E)
        {

        }
*/
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
            Reactor_->Reactor()->addEventHandler(*WS_,
                                                 Poco::NObserver<WSConnection,
                                                 Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
/*            Reactor_.Reactor()->addEventHandler(*WS_,
                                                 Poco::NObserver<WSConnection,
                                                 Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
              Reactor_.Reactor()->addEventHandler(*WS_,
                                                 Poco::NObserver<WSConnection,
                                                 Poco::Net::ErrorNotification>(*this,&WSConnection::OnSocketError));
*/            Registered_ = true ;
        }
    }

    void WSConnection::DeRegister() {
        std::lock_guard<std::mutex> guard(Mutex_);
        if(Registered_ && WS_)
        {
            Reactor_->Reactor()->removeEventHandler(*WS_,
                                                    Poco::NObserver<WSConnection,
                                                    Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
/*            Reactor_.Reactor()->removeEventHandler(*WS_,
                                                    Poco::NObserver<WSConnection,
                                                    Poco::Net::ShutdownNotification>(*this,&WSConnection::OnSocketShutdown));
            Reactor_.Reactor()->removeEventHandler(*WS_,
                                                    Poco::NObserver<WSConnection,
                                                    Poco::Net::ErrorNotification>(*this,&WSConnection::OnSocketError));
          if(WS_)
                WS_->shutdown();
*/            Registered_ = false ;
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

                Parser  parser;
                auto ParsedConfig = parser.parse(NewConfig).extract<Poco::JSON::Object::Ptr>();

                Poco::JSON::Object Params;
                Params.set("serial", SerialNumber_);
                Params.set("uuid", NewConfigUUID);
                Params.set("when", 0);
                Params.set("config", ParsedConfig);

                Poco::JSON::Object ReturnObject;
                ReturnObject.set("method", "configure");
                ReturnObject.set("jsonrpc", "2.0");
                ReturnObject.set("id", RPC_++);
                ReturnObject.set("params", Params);

                std::stringstream Ret;
                Poco::JSON::Stringifier::condense(ReturnObject, Ret);

                Response = Ret.str();
                return true;
            }
        }

        return false;
    }

    Poco::DynamicStruct WSConnection::ExtractCompressedData(const std::string & CompressedData)
    {
        std::vector<std::uint8_t> OB = base64::decode(CompressedData);

        unsigned long MaxSize=OB.size()*10;
        auto UncompressedBuffer = new Bytef [MaxSize];
        unsigned long FinalSize = MaxSize;
        uncompress(UncompressedBuffer, & FinalSize, (Bytef *)&OB[0],OB.size());

        UncompressedBuffer[FinalSize]=0;
        Poco::JSON::Parser parser;
        auto result = parser.parse((char*)UncompressedBuffer).extract<Poco::JSON::Object::Ptr>();
        Poco::DynamicStruct Vars = *result;

        delete [] UncompressedBuffer;
        return Vars;
    }

    void WSConnection::ProcessJSONRPCResult(Poco::DynamicStruct Vars) {

        std::cout << "Processing Result" << std::endl;

        for(auto i : Vars)
        {
            std::cout << "-->" << i.first << ", " << i.second.toString() << std::endl;
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
                auto UUID = ParamsObj["uuid"];
                auto Firmware = ParamsObj["firmware"].toString();
                auto Capabilities = ParamsObj["capabilities"].toString();

                Logger_.information(Poco::format("CONNECT(%s): Starting.", Serial));

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

                auto UUID = ParamsObj["uuid"];
                auto State = ParamsObj["state"].toString();

                Logger_.information(Poco::format("STATE(%s): Updating.", Serial));
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
                auto UUID = ParamsObj["uuid"];
                auto Sanity = ParamsObj["sanity"];
                auto CheckData = ParamsObj["data"].toString();

                Logger_.information(Poco::format("HEALTHCHECK(%s): Updating: Data=%s", Serial, CheckData));

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

    void WSConnection::OnSocketShutdown(const AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
        std::lock_guard<std::mutex> guard(Mutex_);

//        std::cout << "OnSocketShutdown" << std::endl;
//        Logger_.information(Poco::format("SOCKET-SHUTDOWN(%s): Closing.",SerialNumber_));
//        delete this;
    };

    void WSConnection::OnSocketError(const AutoPtr<Poco::Net::ErrorNotification>& pNf) {
        std::lock_guard<std::mutex> guard(Mutex_);

        std::cout << "OnSocketError" << std::endl;
        Logger_.information(Poco::format("SOCKET-ERROR(%s): Closing.",SerialNumber_));
        delete this;
    }

    void WSConnection::OnSocketReadable(const AutoPtr<Poco::Net::ReadableNotification>& pNf) {
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

        try {

            std::lock_guard<std::mutex> guard(Mutex_);

            IncomingSize = WS_->receiveFrame(IncomingMessage_.get(), BufSize, flags);
            Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

            if (IncomingSize == 0 && flags == 0 && Op == 0) {
                Logger_.information(Poco::format("DISCONNECT(%s)", SerialNumber_));
                MustDisconnect = true;
            } else {
                IncomingMessage_.get()[IncomingSize]=0;
                if (Conn_ != nullptr) {
                    Conn_->RX += IncomingSize;
                }

                switch (Op) {
                    case Poco::Net::WebSocket::FRAME_OP_PING: {
                        Logger_.information("WS-PING(" + SerialNumber_ + "): received.");
                        WS_->sendFrame("", 0,
                                       Poco::Net::WebSocket::FRAME_OP_PONG | Poco::Net::WebSocket::FRAME_FLAG_FIN);
                    }
                        break;

                    case Poco::Net::WebSocket::FRAME_OP_PONG: {
                        Logger_.information("PONG(" + SerialNumber_ + "): received.");
                    }
                        break;

                    case Poco::Net::WebSocket::FRAME_OP_TEXT: {
                        Logger_.debug(
                                Poco::format("Frame received (length=%d, flags=0x%x).", IncomingSize, unsigned(flags)));

                        Parser parser;
                        std::string InMsg(IncomingMessage_.get());

                        auto ParsedMessage = parser.parse(InMsg);
                        auto Result = ParsedMessage.extract<Poco::JSON::Object::Ptr>();
                        Poco::DynamicStruct vars = *Result;

                        // for(auto i : vars)
                        //    std::cout << "VARS: " << i.first << " " << vars.contains(i.first) << std::endl;

                        // Logger_.information("Got message...: %s", InMsg);

                        if (vars.contains("jsonrpc") &&
                            vars.contains("method") &&
                            vars.contains("params")) {

                            ProcessJSONRPCEvent(vars);

                        } else if (vars.contains("jsonrpc") &&
                                   vars.contains("result") &&
                                   vars.contains("id")) {
                            ProcessJSONRPCResult(vars);
                        } else {
                            std::string Msg(IncomingMessage_.get());
                            Logger_.warning(Poco::format("INVALID-PAYLOAD: Payload is not JSON-RPC 2.0: %s",Msg));
                        }
                        break;
                    }

                    default: {
                            Logger_.warning("UNKNOWN WS Frame operation: " + std::to_string(Op));
                            std::cerr << "WS: Unknown frame: " << Op << " Flags: " << flags << std::endl;
                            Op = Poco::Net::WebSocket::FRAME_OP_CLOSE;
                        }
                        break;
                    }

                    if (Conn_ != nullptr)
                        Conn_->MessageCount++;
                }
            }
        catch (const Poco::Net::ConnectionResetException & E)
        {
            Logger_.warning( "Caught a ConnectionResetException");
            MustDisconnect= true;
        }
        catch (const Poco::JSON::JSONException & E)
        {
            std::string Msg(IncomingMessage_.get());
            Logger_.warning( Poco::format("%s(%s): Caught a JSONException: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), Msg ));
        }
        catch (const Poco::Net::WebSocketException & E)
        {
            std::string Msg(IncomingMessage_.get());
            Logger_.warning( Poco::format("%s(%s): Caught a websocket exception: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), Msg ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::SSLConnectionUnexpectedlyClosedException & E)
        {
            std::string Msg(IncomingMessage_.get());
            Logger_.warning( Poco::format("%s(%s): Caught a SSLConnectionUnexpectedlyClosedException: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), Msg ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::SSLException & E)
        {
            std::string Msg(IncomingMessage_.get());
            Logger_.warning( Poco::format("%s(%s): Caught a SSL exception: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), Msg ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::NetException & E) {
            std::string Msg(IncomingMessage_.get());
            Logger_.warning( Poco::format("%s(%s): Caught a NetException: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), Msg ));
            MustDisconnect = true ;
        }
        catch (const Poco::IOException & E) {
            std::string Msg(IncomingMessage_.get());
            Logger_.warning( Poco::format("%s(%s): Caught a IOException: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), Msg ));
            MustDisconnect = true ;
        }
        catch (const Poco::Exception &E) {
            std::string Msg(IncomingMessage_.get());
            Logger_.warning( Poco::format("%s(%s): Caught a more generic Poco exception: %s. Message: %s", std::string(__func__), SerialNumber_, E.displayText(), Msg ));
            MustDisconnect = true ;
        }
        catch (const std::exception & E) {
            std::string Msg(IncomingMessage_.get());
            Logger_.warning( Poco::format("%s(%s): Caught a std::exception: %s. Message: %s", std::string{__func__}, SerialNumber_, std::string{E.what()}, Msg) );
            MustDisconnect = true ;
        }

        if(!MustDisconnect)
            return;

        delete this;
    }

    bool WSConnection::SendCommand(const std::string &Cmd) {
        std::lock_guard<std::mutex> guard(Mutex_);

        Logger_.information(Poco::format("Sending command to %s",SerialNumber_));

        return true;
    }

    void WSConnection::StartHandshake() {
        try {
            return;
        }
        catch (const Poco::Net::ConnectionResetException & E)
        {
            Logger_.warning("CONNECTION1: Caught a ConnectionResetException: %s. Message: %s", E.displayText());
        }
        catch (const Poco::InvalidArgumentException & E) {
            Logger_.warning( Poco::format("CONNECTION1: Caught a InvalidArgumentException: %s", E.displayText()));
        }
        catch (const Poco::Net::SSLConnectionUnexpectedlyClosedException & E) {
            Logger_.warning( "CONNECTION1: Caught a SSLConnectionUnexpectedlyClosedException: %s.",E.displayText());
        }
        catch(const Poco::Net::WebSocketException & E) {
            Logger_.warning("CONNECTION1: WebSocketException(). %s",E.displayText());
        }
        catch(const Poco::Net::SSLException & E ) {
            Logger_.warning("CONNECTION1: SSLException(). %s",E.displayText());
        }
        catch (const Poco::Net::NetException & E) {
            Logger_.warning( "CONNECTION1: Caught a NetException: %s.", E.displayText());
        }
        catch (const Poco::IOException & E) {
            Logger_.warning( "CONNECTION1: Caught a IOException: %s.", E.displayText());
        }
        catch(const Poco::Exception & E ) {
            Logger_.warning("CONNECTION1: Exception(). %s", E.displayText());
        }
        catch (const std::exception & E) {
            Logger_.warning( "CONNECTION1: Caught a std::exception: %s. ", E.what());
        }
        std::cout << __LINE__ << std::endl;
        delete this;
    }
};      //namespace
