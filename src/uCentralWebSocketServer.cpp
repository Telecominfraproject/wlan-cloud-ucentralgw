//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include <thread>
#include <cerrno>

#include "Poco/Net/IPAddress.h"
#include "Poco/Net/SSLException.h"
#include "Poco/Net/HTTPServerSession.h"
#include "Poco/Net/HTTPHeaderStream.h"
#include "Poco/Net/HTTPServerRequestImpl.h"
#include "Poco/JSON/Array.h"
#include "Poco/zlib.h"

#include "uAuthService.h"
#include "uCentral.h"
#include "uCentralWebSocketServer.h"
#include "uStorageService.h"
#include "uUtils.h"

namespace uCentral::WebSocket {

    Service *Service::instance_ = nullptr;

    int Start() {
        return Service::instance()->Start();
    }

    void Stop() {
        Service::instance()->Stop();
    }

    Service::Service() noexcept: uSubSystemServer("WebSocketServer", "WS-SVR", "ucentral.websocket"),
            Factory_(Logger_)
    {

    }

	bool Service::ValidateCertificate(const Poco::Crypto::X509Certificate & Certificate) {
		if(IsCertOk()) {
			return Certificate.issuedBy(*IssuerCert_);
		}
		return false;
	}

	int Service::Start() {

        for(const auto & Svr : ConfigServersList_ ) {
            Logger_.notice(Poco::format("Starting: %s:%s Keyfile:%s CertFile: %s", Svr.Address(), std::to_string(Svr.Port()),
											 Svr.KeyFile(),Svr.CertFile()));

			Svr.LogCert(Logger_);
			if(!Svr.RootCA().empty())
				Svr.LogCas(Logger_);

            auto Sock{Svr.CreateSecureSocket(Logger_)};

			if(!IsCertOk()) {
				IssuerCert_ = std::make_unique<Poco::Crypto::X509Certificate>(Svr.IssuerCertFile());
				Logger_.information(Poco::format("Certificate Issuer Name:%s",IssuerCert_->issuerName()));
			}

            auto NewSocketReactor = std::make_unique<Poco::Net::SocketReactor>();
            auto NewSocketAcceptor = std::make_unique<Poco::Net::SocketAcceptor<WSConnection>>( Sock, *NewSocketReactor);
            auto NewThread = std::make_unique<Poco::Thread>();
            NewThread->setName("WebSocketAcceptor."+Svr.Address()+":"+std::to_string(Svr.Port()));
            NewThread->start(*NewSocketReactor);

            WebSocketServerEntry WSE { .SocketReactor{std::move(NewSocketReactor)} ,
                                       .SocketAcceptor{std::move(NewSocketAcceptor)} ,
                                       .SocketReactorThread{std::move(NewThread)}};
            Servers_.push_back(std::move(WSE));
        }

        uint64_t MaxThreads = uCentral::ServiceConfig::GetInt("ucentral.websocket.maxreactors",5);
        Factory_.Init(MaxThreads);

        return 0;
    }

    void Service::Stop() {
        Logger_.notice("Stopping reactors...");

        for(auto const &Svr : Servers_) {
            Svr.SocketReactor->stop();
            Svr.SocketReactorThread->join();
        }
        Factory_.Close();
    }

    CountedReactor::CountedReactor()
    {
        Reactor_ = Service::instance()->GetAReactor();
    }

    CountedReactor::~CountedReactor()
    {
        Reactor_->Release();
    }

	void WSConnection::LogException(const Poco::Exception &E) {
		Logger_.information(Poco::format("EXCEPTION(%s): %s",CId_,E.displayText()));
	}

	WSConnection::WSConnection(Poco::Net::StreamSocket & socket, Poco::Net::SocketReactor & reactor):
            Socket_(socket),
            Logger_(Service::instance()->Logger())
    {
		auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(Socket_.impl());

		SS->completeHandshake();

		CId_ = uCentral::Utils::FormatIPv6(SS->peerAddress().toString());

		if(!SS->secure()) {
			Logger_.error(Poco::format("%s: Connection is NOT secure.",CId_));
		} else {
			Logger_.debug(Poco::format("%s: Connection is secure.",CId_));
		}

		if(SS->havePeerCertificate()) {
			// Get the cert info...
			CertValidation_ = Objects::VALID_CERTIFICATE;
			try {
				Poco::Crypto::X509Certificate	PeerCert(SS->peerCertificate());

				if(uCentral::WebSocket::Service::instance()->ValidateCertificate(PeerCert)) {
					CN_ = PeerCert.commonName();
					CertValidation_ = Objects::MISMATCH_SERIAL;
					Logger_.debug(Poco::format("%s: Valid certificate: CN=%s", CId_, PeerCert.commonName()));
				} else {
					Logger_.debug( Poco::format("%s: Certificate is not valid", CId_));
				}
			} catch (const Poco::Exception &E) {
				LogException(E);
			}
		} else {
			Logger_.error(Poco::format("%s: No certificates available..",CId_));
		}

		auto Params = Poco::AutoPtr<Poco::Net::HTTPServerParams>(new Poco::Net::HTTPServerParams);
        Poco::Net::HTTPServerSession        Session(Socket_, Params);
        Poco::Net::HTTPServerResponseImpl   Response(Session);
        Poco::Net::HTTPServerRequestImpl    Request(Response,Session,Params);

		auto Now = time(nullptr);
        Response.setDate(Now);
        Response.setVersion(Request.getVersion());
        Response.setKeepAlive(Params->getKeepAlive() && Request.getKeepAlive() && Session.canKeepAlive());
        WS_ = std::make_unique<Poco::Net::WebSocket>(Request, Response);
		WS_->setMaxPayloadSize(BufSize);

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
            Registered_ = true ;
        }
    }

    void WSConnection::DeRegister() {
        std::lock_guard<std::mutex> guard(Mutex_);
        if(Registered_ && WS_)
        {
            Reactor_.Reactor()->removeEventHandler(*WS_,
                                                    Poco::NObserver<WSConnection,
                                                    Poco::Net::ReadableNotification>(*this,&WSConnection::OnSocketReadable));
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
				ParsedConfig->set("uuid",NewConfigUUID);

				Poco::JSON::Object Params;
                Params.set("serial", SerialNumber_);
                Params.set("uuid", NewConfigUUID);
                Params.set("when", 0);
                Params.set("config", ParsedConfig);

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
                uCentral::Objects::CommandDetails  Cmd;

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
                    Logger_.debug(Poco::format("Submitted configure command %Lu for %s",CommandID, SerialNumber_));
					CommandIDPair	C{ 	.UUID = Cmd.UUID ,
										.Full = true };
                    RPCs_[CommandID] = C;
                }

                return true;
            }
        }
        return false;
    }

	Poco::JSON::Object::Ptr WSConnection::ExtractCompressedData(const std::string & CompressedData)
    {
        std::vector<uint8_t> OB = uCentral::Utils::base64decode(CompressedData);

        unsigned long MaxSize=OB.size()*10;
        std::vector<char> UncompressedBuffer(MaxSize);
        unsigned long FinalSize = MaxSize;
        if(uncompress((Bytef *)&UncompressedBuffer[0], & FinalSize, (Bytef *)&OB[0],OB.size())==Z_OK) {
			UncompressedBuffer[FinalSize] = 0;
			Poco::JSON::Parser parser;
			auto result = parser.parse(&UncompressedBuffer[0]).extract<Poco::JSON::Object::Ptr>();
			return result;
		} else {
			Poco::JSON::Object::Ptr Vars;
			return Vars;
		}
    }

    void WSConnection::ProcessJSONRPCResult(Poco::JSON::Object::Ptr Doc) {
        uint64_t ID = Doc->get("id");
        auto RPC = RPCs_.find(ID);

        if(RPC!=RPCs_.end())
        {
            Logger_.debug(Poco::format("RPC(%s): Completed outstanding RPC %Lu",SerialNumber_,ID));
            uCentral::Storage::CommandCompleted(RPC->second.UUID,Doc,RPC->second.Full);
			RPCs_.erase(RPC);
        }
        else
        {
            Logger_.warning(Poco::format("RPC(%s): Could not find outstanding RPC %Lu",SerialNumber_,ID));
        }
    }

    void WSConnection::ProcessJSONRPCEvent(Poco::JSON::Object::Ptr Doc) {

        std::string Response;

        auto Method = Doc->get("method").toString();
        auto Params = Doc->get("params");
/*        if(!Params.isStruct())
        {
            Logger_.warning(Poco::format("MISSING-PARAMS(%s): params must be an object.",CId_));
            return;
        }
*/
        //  expand params if necessary
        auto ParamsObj = Params.extract<Poco::JSON::Object::Ptr>();
        if(ParamsObj->has("compress_64"))
        {
            Logger_.debug(Poco::format("EVENT(%s): Found compressed payload.",CId_));
            ParamsObj = ExtractCompressedData(ParamsObj->get("compress_64").toString());
        }

        if(!ParamsObj->has("serial"))
        {
            Logger_.warning(Poco::format("MISSING-PARAMS(%s): Serial number is missing in message.",CId_));
            return;
        }

		auto Serial = ParamsObj->get("serial").toString();
		if(uCentral::Storage::IsBlackListed(Serial)) {
			Poco::Exception	E(Poco::format("BLACKLIST(%s): device is blacklisted and not allowed to connect.",Serial), EACCES);
			E.rethrow();
		}

        if (!Poco::icompare(Method, "connect")) {
            if( ParamsObj->has("uuid") &&
                ParamsObj->has("firmware") &&
                ParamsObj->has("capabilities")) {
                uint64_t UUID = ParamsObj->get("uuid");
                auto Firmware = ParamsObj->get("firmware").toString();
                auto Capabilities = ParamsObj->get("capabilities").toString();

                Conn_ = uCentral::DeviceRegistry::Register(Serial, this);
                SerialNumber_ = Serial;
                Conn_->SerialNumber = Serial;
                Conn_->UUID = UUID;
                Conn_->Firmware = Firmware;
                Conn_->PendingUUID = 0;
				Conn_->Address = uCentral::Utils::FormatIPv6(WS_->peerAddress().toString());
				CId_ = SerialNumber_ + "@" + CId_ ;

				//	We need to verify the certificate if we have one
				if(!CN_.empty() && CN_==SerialNumber_) {
					Conn_->VerifiedCertificate = Objects::VERIFIED;
					Logger_.information(Poco::format("CONNECT(%s): Fully validated and authenticated device..", CId_));
				} else {
					if(CN_.empty())
						Logger_.information(Poco::format("CONNECT(%s): Not authenticated or validated.", CId_));
					else
						Logger_.information(Poco::format("CONNECT(%s): Authenticated but not validated.", CId_));
				}

                if (uCentral::instance()->AutoProvisioning() && !uCentral::Storage::DeviceExists(SerialNumber_))
                    uCentral::Storage::CreateDefaultDevice(SerialNumber_, Capabilities);

				uCentral::Storage::UpdateDeviceCapabilities(SerialNumber_, Capabilities);

				if(!Firmware.empty())
					uCentral::Storage::SetFirmware(SerialNumber_, Firmware);

                LookForUpgrade(Response);
            } else {
                Logger_.warning(Poco::format("CONNECT(%s): Missing one of uuid, firmware, or capabilities",CId_));
                return;
            }
        } else if (!Poco::icompare(Method, "state")) {
             if (ParamsObj->has("uuid") &&
                ParamsObj->has("state")) {

                uint64_t UUID = ParamsObj->get("uuid");
                auto State = ParamsObj->get("state").toString();

				std::string request_uuid;
				if(ParamsObj->has("request_uuid"))
					request_uuid = ParamsObj->get("request_uuid").toString();

				if(request_uuid.empty())
                	Logger_.debug(Poco::format("STATE(%s): UUID=%Lu Updating.", CId_, UUID));
				else
					Logger_.debug(Poco::format("STATE(%s): UUID=%Lu Updating for CMD=%s.", CId_, UUID,request_uuid));

                Conn_->UUID = UUID;
                uCentral::Storage::AddStatisticsData(Serial, UUID, State);
                uCentral::DeviceRegistry::SetStatistics(Serial, State);

				if(!request_uuid.empty()) {
					uCentral::Storage::SetCommandResult(request_uuid,State);
				}
                LookForUpgrade(Response);
            } else {
                Logger_.warning(Poco::format("STATE(%s): Invalid request. Missing serial, uuid, or state",
											 CId_));
            }
        } else if (!Poco::icompare(Method, "healthcheck")) {
            if( ParamsObj->has("uuid") &&
                ParamsObj->has("sanity") &&
                ParamsObj->has("data"))
            {
                uint64_t UUID = ParamsObj->get("uuid");
                auto Sanity = ParamsObj->get("sanity");
				auto CheckData = ParamsObj->get("data").toString();
				if(CheckData.empty())
					CheckData = "{}";

				std::string request_uuid;
				if(ParamsObj->has("request_uuid"))
					request_uuid = ParamsObj->get("request_uuid").toString();

				if(request_uuid.empty())
					Logger_.debug(Poco::format("HEALTHCHECK(%s): UUID=%Lu Updating.", CId_, UUID));
				else
					Logger_.debug(Poco::format("HEALTHCHECK(%s): UUID=%Lu Updating for CMD=%s.", CId_, UUID,request_uuid));

                Conn_->UUID = UUID;

				uCentral::Objects::HealthCheck Check;

                Check.Recorded = time(nullptr);
                Check.UUID = UUID;
                Check.Data = CheckData;
                Check.Sanity = Sanity;

                uCentral::Storage::AddHealthCheckData(Serial, Check);

				if(!request_uuid.empty()) {
					uCentral::Storage::SetCommandResult(request_uuid,CheckData);
				}

                LookForUpgrade(Response);
            }
            else
            {
                Logger_.warning(Poco::format("HEALTHCHECK(%s): Missing parameter",CId_));
                return;
            }
        } else if (!Poco::icompare(Method, "log")) {
            if( ParamsObj->has("log") &&
                ParamsObj->has("severity")) {

                Logger_.debug(Poco::format("LOG(%s): new entry.", CId_));

                auto Log = ParamsObj->get("log").toString();
                auto Severity = ParamsObj->get("severity");
				std::string DataStr = "{}";

				if(ParamsObj->has("data")) {
					auto DataObj = ParamsObj->get("data");
					if(DataObj.isStruct())
						DataStr = DataObj.toString();
				}

                uCentral::Objects::DeviceLog DeviceLog{
					.Log = Log,
					.Data = DataStr,
					.Severity = Severity,
					.Recorded = (uint64_t ) time(nullptr),
					.LogType = 0 };

                uCentral::Storage::AddLog(Serial, DeviceLog);
            }
            else
            {
                Logger_.warning(Poco::format("LOG(%s): Missing parameters.",CId_));
                return;
            }
        } else if (!Poco::icompare(Method, "crashlog")) {
            if( ParamsObj->has("uuid") &&
                ParamsObj->has("loglines")) {

                Logger_.debug(Poco::format("CRASH-LOG(%s): new entry.", CId_));
                auto LogLines = ParamsObj->get("loglines");
				std::string LogText;
				if(LogLines.isArray()) {
					auto LogLinesArray = LogLines.extract<Poco::JSON::Array::Ptr>();
					for(const auto & i : *LogLinesArray)
						LogText += i.toString() + "\r\n";
				}

				uCentral::Objects::DeviceLog DeviceLog{
					.Log = LogText,
					.Data = "",
					.Severity = uCentral::Objects::DeviceLog::LOG_EMERG,
					.Recorded = (uint64_t )time(nullptr),
					.LogType = 1};

				uCentral::Storage::AddLog(Serial, DeviceLog, true);
            }
            else
            {
                Logger_.warning(Poco::format("LOG(%s): Missing parameters.",CId_));
                return;
            }
        } else if (!Poco::icompare(Method, "ping")) {
            if(ParamsObj->has("uuid")) {
                uint64_t UUID = ParamsObj->get("uuid");
                Logger_.debug(Poco::format("PING(%s): Current config is %Lu", CId_, UUID));
            }
            else
            {
                Logger_.warning(Poco::format("PING(%s): Missing parameter.",CId_));
            }
        } else if (!Poco::icompare(Method, "cfgpending")) {
            if( ParamsObj->has("uuid") &&
                ParamsObj->has("active")) {

                uint64_t UUID = ParamsObj->get("uuid");
                uint64_t Active = ParamsObj->get("active");

                Logger_.debug(Poco::format("CFG-PENDING(%s): Active: %Lu Target: %Lu", CId_, Active, UUID));
            }
            else {
                Logger_.warning(Poco::format("CFG-PENDING(%s): Missing some parameters",CId_));
            }
        }

        if (!Response.empty()) {
            if (Conn_ != nullptr)
                Conn_->TX += Response.size();
            try {
				Logger_.debug(Poco::format("RESPONSE(%s): %s",CId_, Response));
                WS_->sendFrame(Response.c_str(), (int)Response.size());
            }
            catch( Poco::Exception & E )
            {
                Logger_.log(E);
                delete this;
            }
        }
    }

    void WSConnection::OnSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
        std::lock_guard<std::mutex> guard(Mutex_);

        Logger_.information(Poco::format("SOCKET-SHUTDOWN(%s): Closing.",CId_));
        delete this;
    }

    void WSConnection::OnSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
        std::lock_guard<std::mutex> guard(Mutex_);

        Logger_.information(Poco::format("SOCKET-ERROR(%s): Closing.",CId_));
        delete this;
    }

    void WSConnection::OnSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
        try
        {
            ProcessIncomingFrame();
        }
        catch ( const Poco::Exception & E )
        {
            Logger_.log(E);
            delete this;
        }
		catch ( const std::exception & E) {
			std::string W = E.what();
			Logger_.information(Poco::format("std::exception caught: %s. Connection terminated with %s",W,CId_));
			delete this;
		}
    }

	std::string asString(Poco::Buffer<char> & buf ) {
		if(buf.sizeBytes()>0) {
			buf.append(0);
			return buf.begin();
		}
		return "";
	}

    void WSConnection::ProcessIncomingFrame() {
        int flags, Op;
        int IncomingSize;

        bool MustDisconnect=false;
		Poco::Buffer<char>			IncomingFrame(0);

        try {
            std::lock_guard<std::mutex> guard(Mutex_);

			IncomingSize = WS_->receiveFrame(IncomingFrame,flags);
            Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

            if (IncomingSize == 0 && flags == 0 && Op == 0) {
                Logger_.information(Poco::format("DISCONNECT(%s)", CId_));
                MustDisconnect = true;
            } else {
                switch (Op) {
                    case Poco::Net::WebSocket::FRAME_OP_PING: {
                        Logger_.debug(Poco::format("WS-PING(%s): received. PONG sent back.", CId_));
                        WS_->sendFrame("", 0,(int)Poco::Net::WebSocket::FRAME_OP_PONG | (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
                        }
                        break;

                    case Poco::Net::WebSocket::FRAME_OP_PONG: {
                        Logger_.debug(Poco::format("PONG(%s): received and ignored.",CId_));
                        }
                        break;

                    case Poco::Net::WebSocket::FRAME_OP_TEXT: {
						std::string IncomingMessageStr = asString(IncomingFrame);
						Logger_.debug(Poco::format("FRAME(%s): Frame received (length=%d, flags=0x%x). Msg=%s",
							   		CId_, IncomingSize, unsigned(flags),IncomingMessageStr));

                        Poco::JSON::Parser parser;
						auto ParsedMessage = parser.parse(IncomingMessageStr);
                        auto IncomingJSON = ParsedMessage.extract<Poco::JSON::Object::Ptr>();

                        if (IncomingJSON->has("jsonrpc")) {
							if(IncomingJSON->has("method") &&
								IncomingJSON->has("params")) {
                            		ProcessJSONRPCEvent(IncomingJSON);
                        	} else if (IncomingJSON->has("result") &&
								IncomingJSON->has("id")) {
								Logger_.debug(Poco::format("RPC-RESULT(%s): payload: %s",CId_,IncomingMessageStr));
                            	ProcessJSONRPCResult(IncomingJSON);
                        	} else {
								Logger_.warning(Poco::format(
									"INVALID-PAYLOAD(%s): Payload is not JSON-RPC 2.0: %s", CId_,
									IncomingMessageStr));
							}
                        } else {
							Logger_.error(Poco::format("FRAME(%s): illegal transaction header, missing 'jsonrpc'",CId_));
							Errors_++;
						}
                        break;
                    }

                    default: {
                            Logger_.warning(Poco::format("UNKNOWN(%s): unknownWS Frame operation: %s",CId_, std::to_string(Op)));
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
			std::string IncomingMessageStr = asString(IncomingFrame);
            Logger_.warning(Poco::format("%s(%s): Caught a ConnectionResetException: %s, Message: %s",
							std::string(__func__), CId_, E.displayText(),IncomingMessageStr));
            MustDisconnect= true;
        }
        catch (const Poco::JSON::JSONException & E)
        {
			std::string IncomingMessageStr = asString(IncomingFrame);
			Logger_.warning(Poco::format("%s(%s): Caught a JSONException: %s. Message: %s",
								std::string(__func__), CId_, E.displayText(), IncomingMessageStr ));
        }
        catch (const Poco::Net::WebSocketException & E)
        {
			std::string IncomingMessageStr = asString(IncomingFrame);
			Logger_.warning(Poco::format("%s(%s): Caught a websocket exception: %s. Message: %s",
							std::string(__func__), CId_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::SSLConnectionUnexpectedlyClosedException & E)
        {
			std::string IncomingMessageStr = asString(IncomingFrame);
			Logger_.warning(Poco::format("%s(%s): Caught a SSLConnectionUnexpectedlyClosedException: %s. Message: %s",
							std::string(__func__), CId_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::SSLException & E)
        {
			std::string IncomingMessageStr = asString(IncomingFrame);
			Logger_.warning(Poco::format("%s(%s): Caught a SSL exception: %s. Message: %s",
							std::string(__func__), CId_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::Net::NetException & E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			Logger_.warning( Poco::format("%s(%s): Caught a NetException: %s. Message: %s",
							 std::string(__func__), CId_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::IOException & E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
			Logger_.warning( Poco::format("%s(%s): Caught a IOException: %s. Message: %s",
							 std::string(__func__), CId_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const Poco::Exception &E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
            Logger_.warning( Poco::format("%s(%s): Caught a more generic Poco exception: %s. Message: %s",
							 std::string(__func__), CId_, E.displayText(), IncomingMessageStr ));
            MustDisconnect = true ;
        }
        catch (const std::exception & E) {
			std::string IncomingMessageStr = asString(IncomingFrame);
            Logger_.warning( Poco::format("%s(%s): Caught a std::exception: %s. Message: %s",
							 std::string{__func__}, CId_, std::string{E.what()}, IncomingMessageStr) );
            MustDisconnect = true ;
        }

        if(!MustDisconnect || Errors_>10)
            return;

        delete this;
    }

    bool WSConnection::SendCommand(uCentral::Objects::CommandDetails & Command) {
        std::lock_guard<std::mutex> guard(Mutex_);

        Logger_.debug(Poco::format("Sending command to %s",CId_));
        try {
            Poco::JSON::Object Obj;

            auto ID = RPC_++;

            Obj.set("jsonrpc","2.0");
            Obj.set("id",ID);
            Obj.set("method", Command.Custom ? "perform" : Command.Command );

			bool FullCommand = true;
			if(Command.Command=="request")
				FullCommand = false;

            // the params section was composed earlier... just include it here
            Poco::JSON::Parser  parser;
            auto ParsedMessage = parser.parse(Command.Details);
            const auto & ParamsObj = ParsedMessage.extract<Poco::JSON::Object::Ptr>();
            Obj.set("params",ParamsObj);
            std::stringstream ToSend;
            Poco::JSON::Stringifier::stringify(Obj,ToSend);

            auto BytesSent = WS_->sendFrame(ToSend.str().c_str(),(int)ToSend.str().size());
            if(BytesSent == ToSend.str().size())
            {
				CommandIDPair	C{ .UUID = Command.UUID,
									.Full = FullCommand };
                RPCs_[ID] = C;
                uCentral::Storage::CommandExecuted(Command.UUID);
                return true;
            }
            else
            {
                Logger_.warning(Poco::format("COMMAND(%s): Could not send the entire command.",CId_));
                return false;
            }
        }
        catch( const Poco::Exception & E )
        {
            Logger_.warning(Poco::format("COMMAND(%s): Exception while sending a command.",CId_));
        }
        return false;
    }
}      //namespace
