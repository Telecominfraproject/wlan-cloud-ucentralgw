//
// Created by stephane bourque on 2021-11-23.
//

#include "rttys/RTTYS_server.h"
#include "rttys/RTTYS_WebServer.h"

#include "AP_WS_Server.h"

#include "fmt/format.h"
#include "framework/MicroServiceFuncs.h"

#include "Poco/Net/SecureStreamSocketImpl.h"
#include "nlohmann/json.hpp"

#include "Poco/NObserver.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/WebSocketImpl.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketAcceptor.h"


#define DBGLINE                                                                                    \
	{ std::cout << __LINE__ << std::endl; }

namespace OpenWifi {

	int RTTYS_server::Start() {

		poco_information(Logger(),"Starting...");
		Internal_ = MicroServiceConfigGetBool("rtty.internal", false);
		if (Internal_) {
			int DSport = (int)MicroServiceConfigGetInt("rtty.port", 5912);
			int CSport = (int)MicroServiceConfigGetInt("rtty.viewport", 5913);
			RTTY_UIAssets_ = MicroServiceConfigPath("rtty.assets", "$OWGW_ROOT/rtty_ui");
			MaxConcurrentSessions_ = MicroServiceConfigGetInt("rtty.maxsessions", 0);
			enforce_mTLS_ = MicroServiceConfigGetBool("rtty.enforcemTLS", false);
			NoSecurity_ = MicroServiceNoAPISecurity();

			if (NoSecurity_) {
				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, DSport);
				ServerDeviceSocket_ = std::make_unique<Poco::Net::ServerSocket>(SockAddr, 64);
				Reactor_.addEventHandler(
					*ServerDeviceSocket_, Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
										*this, &RTTYS_server::onDeviceAccept));
			} else {
				const auto &CertFileName =
					MicroServiceConfigPath("ucentral.websocket.host.0.cert", "");
				const auto &KeyFileName =
					MicroServiceConfigPath("ucentral.websocket.host.0.key", "");
				const auto &RootCaFileName =
					MicroServiceConfigPath("ucentral.websocket.host.0.rootca", "");
				const auto &IssuerFileName =
					MicroServiceConfigPath("ucentral.websocket.host.0.issuer", "");
				const auto &KeyPassword =
					MicroServiceConfigPath("ucentral.websocket.host.0.key.password", "");
				const auto &RootCas =
					MicroServiceConfigPath("ucentral.websocket.host.0.rootca", "");
				const auto &Cas = MicroServiceConfigPath("ucentral.websocket.host.0.cas", "");

				Poco::Net::Context::Params P;

				P.verificationMode = Poco::Net::Context::VERIFY_ONCE;
				P.verificationDepth = 9;
				P.loadDefaultCAs = RootCas.empty();
				P.cipherList = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";
				// P.dhUse2048Bits = true;
				P.caLocation = Cas;

				auto DeviceSecureContext = Poco::AutoPtr<Poco::Net::Context>(
					new Poco::Net::Context(Poco::Net::Context::TLS_SERVER_USE, P));
				Poco::Crypto::X509Certificate Cert(CertFileName);
				Poco::Crypto::X509Certificate Root(RootCaFileName);
				Poco::Crypto::X509Certificate Issuing(IssuerFileName);
				Poco::Crypto::RSAKey Key("", KeyFileName, KeyPassword);

				DeviceSecureContext->useCertificate(Cert);
				DeviceSecureContext->addChainCertificate(Root);
				DeviceSecureContext->addCertificateAuthority(Root);
				DeviceSecureContext->addChainCertificate(Issuing);
				DeviceSecureContext->addCertificateAuthority(Issuing);
				DeviceSecureContext->addCertificateAuthority(Root);
				DeviceSecureContext->enableSessionCache(true);
				DeviceSecureContext->setSessionCacheSize(0);
				DeviceSecureContext->setSessionTimeout(120);
				DeviceSecureContext->enableExtendedCertificateVerification(false);
				DeviceSecureContext->usePrivateKey(Key);
				DeviceSecureContext->disableProtocols(Poco::Net::Context::PROTO_TLSV1 |
													  Poco::Net::Context::PROTO_TLSV1_1);

				SSL_CTX *SSLCtxDevice = DeviceSecureContext->sslContext();
				SSL_CTX_dane_enable(SSLCtxDevice);
				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, DSport);

				SecureServerDeviceSocket_ = std::make_unique<Poco::Net::SecureServerSocket>(
					SockAddr, 64, DeviceSecureContext);

				Reactor_.addEventHandler(
					*SecureServerDeviceSocket_,
					Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
						*this, &RTTYS_server::onDeviceAccept));
			}

			ReactorThread_.start(Reactor_);
			Utils::SetThreadName(ReactorThread_, "rt:devreactor");

			auto WebServerHttpParams = new Poco::Net::HTTPServerParams;
			WebServerHttpParams->setMaxThreads(50);
			WebServerHttpParams->setMaxQueued(200);
			WebServerHttpParams->setKeepAlive(true);
			WebServerHttpParams->setName("rt:dispatch");

			if (NoSecurity_) {
				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, CSport);
				Poco::Net::ServerSocket ClientSocket(SockAddr, 64);
				ClientSocket.setNoDelay(true);
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(
					new RTTYS_Client_RequestHandlerFactory(Logger()), ClientSocket,
					WebServerHttpParams);
			} else {
				const auto &CertFileName =
					MicroServiceConfigPath("openwifi.restapi.host.0.cert", "");
				const auto &KeyFileName = MicroServiceConfigPath("openwifi.restapi.host.0.key", "");
				const auto &RootCaFileName =
					MicroServiceConfigPath("openwifi.restapi.host.0.rootca", "");

				auto WebClientSecureContext =
					new Poco::Net::Context(Poco::Net::Context::SERVER_USE, KeyFileName,
										   CertFileName, "", Poco::Net::Context::VERIFY_RELAXED);
				Poco::Crypto::X509Certificate WebRoot(RootCaFileName);
				WebClientSecureContext->addCertificateAuthority(WebRoot);
				WebClientSecureContext->disableStatelessSessionResumption();
				WebClientSecureContext->enableSessionCache();
				WebClientSecureContext->setSessionCacheSize(0);
				WebClientSecureContext->setSessionTimeout(10);
				WebClientSecureContext->enableExtendedCertificateVerification(true);
				SSL_CTX *SSLCtxClient = WebClientSecureContext->sslContext();
				SSL_CTX_dane_enable(SSLCtxClient);

				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, CSport);
				Poco::Net::SecureServerSocket ClientSocket(SockAddr, 64, WebClientSecureContext);
				ClientSocket.setNoDelay(true);
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(
					new RTTYS_Client_RequestHandlerFactory(Logger()), ClientSocket,
					WebServerHttpParams);
			};
			WebServer_->start();
		}

		GCCallBack_ =
			std::make_unique<Poco::TimerCallback<RTTYS_server>>(*this, &RTTYS_server::onTimer);
		Timer_.setStartInterval(30 * 1000); // first run in 30 seconds
		Timer_.setPeriodicInterval(60 * 1000);
		Timer_.start(*GCCallBack_, MicroServiceTimerPool());
		Running_ = true;
		return 0;
	}

	void RTTYS_server::Stop() {
		poco_information(Logger(),"Stopping...");
		Timer_.stop();
		Running_ = false;
		if (Internal_) {
			WebServer_->stopAll(true);
			WebServer_->stop();
			Reactor_.stop();
			ReactorThread_.join();
		}
		poco_information(Logger(),"Stopped...");
	}

	void RTTYS_server::onDeviceAccept(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		try {
			std::lock_guard	Lock(ServerMutex_);
			Poco::Net::SocketAddress Client;
			Poco::Net::StreamSocket NewSocket(pNf->socket().impl()->acceptConnection(Client));
			if (NewSocket.secure()) {
				auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(NewSocket.impl());
				auto PeerAddress_ = SS->peerAddress().host();
				auto CId_ = Utils::FormatIPv6(SS->peerAddress().toString());
				std::string cn;
				poco_debug(Logger(),fmt::format("{}: Completing TLS handshake.", CId_));
				while (true) {
					auto V = SS->completeHandshake();
					if (V == 1)
						break;
				}
				poco_debug(Logger(),fmt::format("{}: Completed TLS handshake.", CId_));
				std::unique_ptr<Poco::Crypto::X509Certificate>	Cert;
				if (SS->havePeerCertificate()) {
					poco_debug(Logger(),fmt::format("{}: Device has certificate.", CId_));
					Cert = std::make_unique<Poco::Crypto::X509Certificate>(SS->peerCertificate());
					cn = Poco::trim(Poco::toLower(Cert->commonName()));
					if (AP_WS_Server()->ValidateCertificate(CId_, *Cert)) {
						poco_information(
							Logger(),
							fmt::format("{}: Device {} has been validated.", cn, CId_));
						AddNewSocket(NewSocket, std::move(Cert), true, CId_, cn);
					} else {
						poco_warning(Logger(),fmt::format("{}: Device {} cannot be validate", CId_, cn));
						AddNewSocket(NewSocket, std::move(Cert), false, CId_, cn);
					}
				} else {
					poco_warning(Logger(), fmt::format("{}: Device has no certificate.", CId_));
					AddNewSocket(NewSocket, std::move(Cert), false, CId_, cn);
				}
				return;
			}
			NewSocket.close();
		} catch (const Poco::Exception &E) {
			std::cout << "Exception onDeviceAccept: " << E.what() << std::endl;
			Logger().log(E);
		}
	}

	void RTTYS_server::RemoveClientEventHandlers(Poco::Net::WebSocket &Socket) {
		int fd = Socket.impl()->sockfd();
		if(Reactor_.has(Socket)) {
			Reactor_.removeEventHandler(
				Socket, Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
							*this, &RTTYS_server::onClientSocketReadable));
			Reactor_.removeEventHandler(
				Socket, Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
							*this, &RTTYS_server::onClientSocketShutdown));
			Reactor_.removeEventHandler(Socket,
										Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
											*this, &RTTYS_server::onClientSocketError));
		}
		Clients_.erase(fd);
	}

	void RTTYS_server::AddNewSocket(Poco::Net::StreamSocket &Socket, std::unique_ptr<Poco::Crypto::X509Certificate> P, bool valid, const std::string &cid, const std::string &cn) {
		Socket.setNoDelay(true);
		Socket.setKeepAlive(true);
		Socket.setBlocking(false);
		Socket.setReceiveBufferSize(RTTY_RECEIVE_BUFFER);
		Socket.setSendBufferSize(RTTY_RECEIVE_BUFFER);
		Poco::Timespan TS2(300, 100);
		Socket.setReceiveTimeout(TS2);

		Reactor_.addEventHandler(Socket,
								 Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
									 *this, &RTTYS_server::onConnectedDeviceSocketReadable));
		Reactor_.addEventHandler(Socket,
								 Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
									 *this, &RTTYS_server::onConnectedDeviceSocketShutdown));
		Reactor_.addEventHandler(Socket,
								 Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
									 *this, &RTTYS_server::onConnectedDeviceSocketError));
		int fd = Socket.impl()->sockfd();
		Sockets_[fd] = std::make_unique<SecureSocketPair>(Socket, std::move(P), valid, cid, cn);
	}


	void RTTYS_server::RemoveSocket(const Poco::Net::Socket &Socket) {
		auto hint = Sockets_.find(Socket.impl()->sockfd());
		if(hint!=end(Sockets_)) {
			Reactor_.removeEventHandler(
				Socket, Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
							*this, &RTTYS_server::onConnectedDeviceSocketReadable));
			Reactor_.removeEventHandler(
				Socket, Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
							*this, &RTTYS_server::onConnectedDeviceSocketShutdown));
			Reactor_.removeEventHandler(Socket,
										Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
											*this, &RTTYS_server::onConnectedDeviceSocketError));
			Sockets_.erase(hint);
		}
	}

	void RTTYS_server::AddClientEventHandlers(Poco::Net::WebSocket &Socket,
											  std::shared_ptr<RTTYS_EndPoint> EndPoint) {
		Clients_[Socket.impl()->sockfd()] = EndPoint;
		Reactor_.addEventHandler(Socket,
								 Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
									 *this, &RTTYS_server::onClientSocketReadable));
		Reactor_.addEventHandler(Socket,
								 Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
									 *this, &RTTYS_server::onClientSocketShutdown));
		Reactor_.addEventHandler(Socket,
								 Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
									 *this, &RTTYS_server::onClientSocketError));
	}

	int RTTYS_server::SendBytes(int fd, const unsigned char *buffer, std::size_t len) {
		auto hint = Sockets_.find(fd);
		if(hint==end(Sockets_)) {
			poco_error(Logger(),fmt::format("Cannot find this socket: {}",fd));
			return -1;
		}
		return hint->second->socket.impl()->sendBytes(buffer,len);
	}

	int RTTYS_server::SendBytes(const Poco::Net::Socket &Socket, const unsigned char *buffer, std::size_t len) {
		return Socket.impl()->sendBytes(buffer,len);
	}

	std::shared_ptr<RTTYS_EndPoint> RTTYS_server::FindRegisteredEndPoint(const std::string &Id,
														   const std::string &Token) {
		auto EndPoint = EndPoints_.find(Id);
		if (EndPoint != end(EndPoints_) && EndPoint->second->Token_ == Token) {
			return EndPoint->second;
		}
		return nullptr;
	}


	bool RTTYS_server::do_msgTypeRegister(const Poco::Net::Socket &Socket, Poco::FIFOBuffer &Buffer, [[maybe_unused]] std::size_t msg_len) {
		bool good = true;
		try {

			auto fd = Socket.impl()->sockfd();
			std::string id_ = ReadString(Buffer);
			std::string desc_ = ReadString(Buffer);
			std::string token_ = ReadString(Buffer);

			poco_information(Logger(),fmt::format("Device registration: description:{} id:{} token:{}", desc_, id_, token_));
			if (id_.size() != RTTY_DEVICE_TOKEN_LENGTH ||
				token_.size() != RTTY_DEVICE_TOKEN_LENGTH || desc_.empty()) {
				poco_warning(Logger(),fmt::format("Wrong register header. {} {} {}", id_,desc_,token_));
				return false;
			}

			//	find this device in our connectio end points...
			poco_information(Logger(),fmt::format("{}: Looking for session", id_));

			auto ConnectionHint = EndPoints_.find(id_);
			if (ConnectionHint == end(EndPoints_) || ConnectionHint->second->Token_ != token_) {
				poco_warning(Logger(), fmt::format("{}: Unknown session from device.", id_));
				return false;
			}

			auto SocketHint = Sockets_.find(Socket.impl()->sockfd());
			if(SocketHint==end(Sockets_)) {
				poco_warning(Logger(), fmt::format("{}: Unknown socket from device.", id_));
				return false;
			}

			auto ConnectionEp = ConnectionHint->second;
			poco_information(Logger(),fmt::format("{}: Evaluation of mTLS requirements",id_));
			if (ConnectionEp->mTLS_) {
				if(SocketHint->second->valid) {
					poco_information(Logger(),
									 fmt::format("Device mTLS {} has been validated from {}.",
												 SocketHint->second->cn, SocketHint->second->cid));
				} else {
					poco_error(Logger(),
							   fmt::format("{}: Device failed certificate validation", id_));
					return false;
				}
			} else {
				poco_information(Logger(),
								 fmt::format("Device mTLS {} does not require mTLS from {}.",
											 SocketHint->second->cn, SocketHint->second->cid));
			}
			Poco::Thread::trySleep(50);

			ConnectionEp->Device_fd = fd;
			Connected_[fd] = ConnectionEp;

			u_char OutBuf[8];
			OutBuf[0] = RTTYS_EndPoint::msgTypeRegister;
			OutBuf[1] = 0; //	Data length
			OutBuf[2] = 4; //
			OutBuf[3] = 0; //	Error
			OutBuf[4] = 'O';
			OutBuf[5] = 'K';
			OutBuf[6] = 0;

			if (SendBytes(Socket,OutBuf, 7) != 7) {
				poco_error(
					Logger(),
					fmt::format("{}: Description:{} Could not send data to complete registration",
								id_, desc_));
				return false;
			}
			ConnectionEp->DeviceConnected_ = std::chrono::high_resolution_clock::now();
			ConnectionEp->DeviceIsAttached_ = true;
			ConnectionEp->DeviceSocket_ = Socket;
			if(ConnectionEp->WSSocket_!= nullptr && ConnectionEp->WSSocket_->impl()!= nullptr) {
				poco_information(Logger(),fmt::format("REG{}: Device registered, Client Registered - sending login", ConnectionEp->SerialNumber_));
				Login(Socket, ConnectionEp);
			} else {
				poco_information(Logger(),fmt::format("REG{}: Device registered, Client Not Registered", ConnectionEp->SerialNumber_));
			}
			return true;
		} catch (...) {
			good = false;
		}

		return good;
	}

	void RTTYS_server::onConnectedDeviceTimeOut(const Poco::AutoPtr<Poco::Net::TimeoutNotification> &pNf) {
		try {
			u_char MsgBuf[RTTY_HDR_SIZE];
			MsgBuf[0] = RTTYS_EndPoint::msgTypeHeartbeat;
			MsgBuf[1] = 0;
			MsgBuf[2] = 0;
			pNf->socket().impl()->sendBytes(MsgBuf, RTTY_HDR_SIZE);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (const std::exception &E) {
			LogStdException(E, "Cannot send heartbeat");
		}
	}

	void RTTYS_server::onConnectedDeviceSocketReadable(
		const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {

		std::shared_ptr<RTTYS_EndPoint> ConnectionPtr;
		std::lock_guard	Lock(ServerMutex_);

		try {

			int fd = pNf->socket().impl()->sockfd();
			auto hint = Sockets_.find(fd);
			if(hint==end(Sockets_)) {
				poco_error(Logger(),fmt::format("{}: unknown socket",fd));
				return;
			}

			Poco::FIFOBuffer &buffer = *hint->second->buffer;

			int received_bytes=0, line=0;
			try {
				std::cout << "Available: " << buffer.available() << "  ";
				received_bytes = hint->second->socket.receiveBytes(*hint->second->buffer);
				if(received_bytes==0) {
					poco_warning(Logger(), "Device Closing connection - 0 bytes received.");
					EndConnection( pNf->socket(), __func__, __LINE__ );
					return;
				}
			} catch (const Poco::TimeoutException &E) {
				poco_warning(Logger(), "Receive timeout");
				EndConnection( pNf->socket(), __func__, __LINE__ );
				return;
			} catch (const Poco::Net::NetException &E) {
				Logger().log(E);
				EndConnection( pNf->socket(), __func__, __LINE__ );
				return;
			}

			bool good = true;

			while (!buffer.isEmpty() && good) {

				if(buffer.used() < RTTY_HDR_SIZE) {
					poco_debug(Logger(),fmt::format("Not enough data in the pipe for header",buffer.used()));
					std::cout << "Not enough in header: " << buffer.used() << std::endl;
					return;
				}

				std::uint8_t header[RTTY_HDR_SIZE];
				buffer.peek((char*)header,RTTY_HDR_SIZE);

				std::uint8_t LastCommand = header[0];
				std::uint16_t msg_len = (header[1] << 8) + header[2];

				if(buffer.used()<(RTTY_HDR_SIZE+msg_len)) {
					poco_debug(Logger(),fmt::format("Not enough data in the pipe for command data",buffer.used()));
					std::cout << "Not enough in header: " << buffer.used() << "  msg length: " << msg_len << std::endl;
					return;
				}

				if(!(line & 0x003f)) {
					do_msgTypeHeartbeat(pNf->socket(), buffer, msg_len);
				}

				buffer.drain(RTTY_HDR_SIZE);

				std::cout << line++ << "  Available: " << buffer.available() << "  Cmd: " << (int) LastCommand << "  Received: " << received_bytes
						  << "  MsgLen: " << msg_len << "  Data in buffer: " << buffer.used() << std::endl;
				switch (LastCommand) {
					case RTTYS_EndPoint::msgTypeRegister: {
						good = do_msgTypeRegister(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeLogin: {
						good = do_msgTypeLogin(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeLogout: {
						good = do_msgTypeLogout(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeTermData: {
						good = do_msgTypeTermData(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeWinsize: {
						good = do_msgTypeWinsize(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeCmd: {
						good = do_msgTypeCmd(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeHeartbeat: {
						good = do_msgTypeHeartbeat(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeFile: {
						good = do_msgTypeFile(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeHttp: {
						good = do_msgTypeHttp(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeAck: {
						good = do_msgTypeAck(pNf->socket(), buffer, msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeMax: {
						good = do_msgTypeMax(pNf->socket(), buffer, msg_len);
					} break;
					default: {
						poco_warning(Logger(),
									 fmt::format("Unknown command {}. GW closing connection.",
												 (int)LastCommand));
						good = false;
					}
				}
			}

			std::cout << "Empty: " << buffer.isEmpty() << std::endl;

			if (!good) {
				EndConnection(pNf->socket(), __func__, __LINE__);
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
			EndConnection(pNf->socket(), __func__,__LINE__);
		} catch (...) {
			EndConnection(pNf->socket(), __func__,__LINE__);
		}
	}

	void RTTYS_server::onConnectedDeviceSocketShutdown(
		const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		std::lock_guard	Lock(ServerMutex_);
		EndConnection(pNf->socket(), __func__,__LINE__);
	}

	void RTTYS_server::onConnectedDeviceSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		std::lock_guard	Lock(ServerMutex_);
		EndConnection(pNf->socket(), __func__,__LINE__);
	}

	void RTTYS_server::onClientSocketReadable(
		const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {

		std::lock_guard	Lock(ServerMutex_);

		auto Client = Clients_.end();
		std::shared_ptr<RTTYS_EndPoint> Connection;
		try {
			Client = Clients_.find(pNf->socket().impl()->sockfd());
			if (Client == end(Clients_)) {
				poco_warning(Logger(), fmt::format("Cannot find client socket: {}",
												   pNf->socket().impl()->sockfd()));
				return;
			}
			Connection = Client->second;
			if(Connection->WSSocket_==nullptr || Connection->WSSocket_->impl()==nullptr) {
				poco_warning(Logger(), fmt::format("WebSocket is no valid: {}",
												   Connection->SerialNumber_));
				return;
			}

			int flags;
			unsigned char FrameBuffer[1024];

			auto ReceivedBytes = Connection->WSSocket_->receiveFrame(FrameBuffer, sizeof(FrameBuffer), flags);
			auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;
			switch (Op) {

			case Poco::Net::WebSocket::FRAME_OP_PING: {
				Connection->WSSocket_->sendFrame("", 0,
											   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
												   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
			} break;
			case Poco::Net::WebSocket::FRAME_OP_PONG: {
			} break;
			case Poco::Net::WebSocket::FRAME_OP_TEXT: {
				if (ReceivedBytes == 0) {
					EndConnection(Connection,__func__,__LINE__);
					return;
				} else {
					std::string Frame((const char *)FrameBuffer, ReceivedBytes);
					try {
						auto Doc = nlohmann::json::parse(Frame);
						if (Doc.contains("type")) {
							auto Type = Doc["type"];
							if (Type == "winsize") {
								auto cols = Doc["cols"];
								auto rows = Doc["rows"];
								if (!RTTYS_server().WindowSize(Connection,cols, rows)) {
									EndConnection(Connection,__func__,__LINE__);
									return;
								}
							}
						}
					} catch (...) {
						// just ignore parse errors
						EndConnection(Connection,__func__,__LINE__);
						return;
					}
				}
			} break;
			case Poco::Net::WebSocket::FRAME_OP_BINARY: {
				if (ReceivedBytes == 0) {
					EndConnection(Connection,__func__,__LINE__);
					return;
				} else {
					poco_trace(Logger(),
							   fmt::format("Sending {} key strokes to device.", ReceivedBytes));
					if (!RTTYS_server().KeyStrokes(Connection, FrameBuffer, ReceivedBytes)) {
						EndConnection(Connection,__func__,__LINE__);
						return;
					}
				}
			} break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
				EndConnection(Connection,__func__,__LINE__);
				return;
			} break;

			default: {
			}
			}
		} catch (...) {
			poco_error(Logger(), "Frame readable shutdown.");
			if (Client != Clients_.end() && Connection != nullptr) {
				EndConnection(Connection,__func__,__LINE__);
			}
			return;
		}
	}

	void RTTYS_server::onClientSocketShutdown(
		const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		std::lock_guard	Lock(ServerMutex_);
		auto Client = Clients_.find(pNf->socket().impl()->sockfd());
		if (Client == end(Clients_)) {
			poco_warning(Logger(), fmt::format("Cannot find client socket: {}",
											   pNf->socket().impl()->sockfd()));
			return;
		}
		EndConnection(Client->second,__func__,__LINE__);
	}

	void RTTYS_server::onClientSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		std::lock_guard	Lock(ServerMutex_);
		auto Client = Clients_.find(pNf->socket().impl()->sockfd());
		if (Client == end(Clients_)) {
			poco_warning(Logger(), fmt::format("Cannot find client socket: {}",
											   pNf->socket().impl()->sockfd()));
			return;
		}
		EndConnection(Client->second,__func__,__LINE__);
	}

	void RTTYS_server::SendData(std::shared_ptr<RTTYS_EndPoint> &Connection, const u_char *Buf,
								size_t len) {
		if (Connection->WSSocket_ != nullptr && Connection->WSSocket_->impl()!= nullptr) {
			try {
				Connection->WSSocket_->sendFrame(Buf, len,
												 Poco::Net::WebSocket::FRAME_FLAG_FIN |
													 Poco::Net::WebSocket::FRAME_OP_BINARY);
				return;
			} catch (...) {
				poco_error(Logger(), "SendData shutdown.");
			}
		}
		EndConnection(Connection,__func__,__LINE__);
	}

	void RTTYS_server::SendData(std::shared_ptr<RTTYS_EndPoint> &Connection, const std::string &s) {
		if (Connection->WSSocket_ != nullptr && Connection->WSSocket_->impl()!= nullptr) {
			try {
				Connection->WSSocket_->sendFrame(s.c_str(), s.length());
				return;
			} catch (...) {
				poco_error(Logger(), "SendData shutdown.");
			}
		}
		EndConnection(Connection,__func__,__LINE__);
	}

	void RTTYS_server::LogStdException(const std::exception &E, const std::string & msg) {
		poco_warning(Logger(), fmt::format("{}: std::exception: {}", msg, E.what()));
	}

	void RTTYS_server::CreateWSClient(Poco::Net::HTTPServerRequest &request,
									  Poco::Net::HTTPServerResponse &response,
									  const std::string &Id) {

		std::lock_guard	Lock(ServerMutex_);

		auto EndPoint = EndPoints_.find(Id);
		if (EndPoint == end(EndPoints_)) {
			poco_warning(Logger(), fmt::format("Session {} is invalid.", Id));
			return;
		}

		if (EndPoint->second->WSSocket_ != nullptr) {
			poco_warning(Logger(), fmt::format("Session {} is a duplicate.", Id));
			return;
		}

		//	OK Create and register this WS client
		try {
			// EndPoint->second->WSSocket_ = std::make_unique<Poco::Net::WebSocket>(request, response);
			EndPoint->second->WSSocket_ = std::make_unique<Poco::Net::WebSocket>(request, response);
			EndPoint->second->ClientConnected_ = std::chrono::high_resolution_clock::now();
			EndPoint->second->WSSocket_->setBlocking(false);
			EndPoint->second->WSSocket_->setNoDelay(true);
			EndPoint->second->WSSocket_->setKeepAlive(true);
			AddClientEventHandlers(*EndPoint->second->WSSocket_, EndPoint->second);
			if (EndPoint->second->DeviceIsAttached_ && !EndPoint->second->completed_) {
				poco_information(Logger(),fmt::format("CLN{}: Device registered, Client Registered - sending login", EndPoint->second->SerialNumber_));
				auto hint = Sockets_.find(EndPoint->second->Device_fd);
				if(hint!=end(Sockets_))
					Login(hint->second->socket, EndPoint->second);
			} else {
				poco_information(Logger(),fmt::format("CLN{}: Device not registered, Client Registered", EndPoint->second->SerialNumber_));
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (const std::exception &E) {
			LogStdException(E, "Cannot create RTTY-WS-UI client");
		}
	}

	void RTTYS_server::onTimer([[maybe_unused]] Poco::Timer &timer) {
		poco_trace(Logger(), "Removing stale connections.");
		Utils::SetThreadName("rt:janitor");
		static auto LastStats = Utils::Now();

		std::lock_guard	Lock(ServerMutex_);
		auto Now = std::chrono::high_resolution_clock::now();
		for (auto EndPoint = EndPoints_.begin(); EndPoint != EndPoints_.end();) {
			if ((Now - EndPoint->second->Created_) > 2min && !EndPoint->second->completed_) {
				EndPoint = EndConnection(EndPoint->second,__func__,__LINE__);
			} else {
				++EndPoint;
			}
		}

		poco_information(Logger(),fmt::format("EndPoints:{} Connected:{} Sockets:{} Clients:{}",
											   EndPoints_.size(),Connected_.size(),
											   Sockets_.size(), Clients_.size()));

		if (Utils::Now() - LastStats > (60 * 1)) {
			LastStats = Utils::Now();
			poco_information(Logger(),fmt::format(
				"Statistics: Total connections:{} Current-connections:{} Avg-Device-Connection "
				"Time: {:.2f}ms Avg-Client-Connection Time: {:.2f}ms #Sockets: {}. Connecting "
				"devices: {}",
				TotalEndPoints_, EndPoints_.size(),
				TotalEndPoints_ ? TotalConnectedDeviceTime_.count() / (double)TotalEndPoints_ : 0.0,
				TotalEndPoints_ ? TotalConnectedClientTime_.count() / (double)TotalEndPoints_ : 0.0,
				Sockets_.size(), 0));
		}
	}

	std::map<std::string, std::shared_ptr<RTTYS_EndPoint>>::iterator RTTYS_server::EndConnection(std::shared_ptr<RTTYS_EndPoint> Connection, const char * func, std::uint64_t Line) {
		auto hint1 = Sockets_.find(Connection->Device_fd);
		if(hint1!=end(Sockets_))
			RemoveSocket(hint1->second->socket);

		Connected_.erase(Connection->Device_fd);

		//	find the client linked to this one...
		if(Connection->WSSocket_!= nullptr && Connection->WSSocket_->impl()!= nullptr) {
			RemoveClientEventHandlers(*Connection->WSSocket_);
			Connection->WSSocket_->close();
		}
		poco_debug(Logger(),fmt::format("Closing connection {}:{}", func, Line));
		auto hint2 = EndPoints_.find(Connection->Id_);
		return EndPoints_.erase(hint2);
	}

	void RTTYS_server::EndConnection(const Poco::Net::Socket &Socket, const char * func, std::uint32_t Line) {
		//	remove the device
		auto fd = Socket.impl()->sockfd();
		RemoveSocket(Socket);

		//	find the client linked to this one...
		auto hint = Connected_.find(fd);
		if(hint!=end(Connected_)) {
			if(hint->second->WSSocket_!= nullptr && hint->second->WSSocket_->impl()!= nullptr) {
				RemoveClientEventHandlers(*hint->second->WSSocket_);
				hint->second->WSSocket_->close();
				auto id = hint->second->Id_;
				Connected_.erase(hint);
				EndPoints_.erase(id);
			} else {
				EndPoints_.erase(hint->second->Id_);
			}
		} else {
			std::cout << "Cannot find the associated WS..." << std::endl;
		}
		poco_debug(Logger(),fmt::format("Closing connection at {}:{}", func, Line));
	}

	bool RTTYS_server::CreateEndPoint(const std::string &Id, const std::string &Token,
									  const std::string &UserName,
									  const std::string &SerialNumber,
									  bool mTLS) {

		auto Start = Utils::Now();
		while(!ServerMutex_.try_lock()) {
			Poco::Thread::yield();
			Poco::Thread::trySleep(100);
			if((Utils::Now()-Start)>10) {
				poco_warning(Logger(),"RTTY too busy");
				return false;
			}
		}

		if (MaxConcurrentSessions_ != 0 && EndPoints_.size() == MaxConcurrentSessions_) {
			ServerMutex_.unlock();
			return false;
		}

		EndPoints_[Id] = std::make_shared<RTTYS_EndPoint>(Id, Token, SerialNumber, UserName, mTLS);
		++TotalEndPoints_;
		ServerMutex_.unlock();
		return true;
	}

	bool RTTYS_server::ValidId(const std::string &Id) {
		std::lock_guard	Lock(ServerMutex_);
		return EndPoints_.find(Id) != EndPoints_.end();
	}

	bool RTTYS_server::KeyStrokes(std::shared_ptr<RTTYS_EndPoint> Conn, const u_char *buf, size_t len) {

		if (len <= (sizeof(Conn->small_buf_) - RTTY_HDR_SIZE - 1)) {
			Conn->small_buf_[0] = RTTYS_EndPoint::msgTypeTermData;
			Conn->small_buf_[1] = ((len - 1 + 1) & 0xff00) >> 8;
			Conn->small_buf_[2] = ((len - 1 + 1) & 0x00ff);
			Conn->small_buf_[3] = Conn->sid_;
			memcpy(&Conn->small_buf_[RTTY_HDR_SIZE + 1], &buf[1], len - 1);
			try {
				auto Sent = SendBytes(Conn->DeviceSocket_, Conn->small_buf_,
													 RTTY_HDR_SIZE + 1 + len - 1);
				return (Sent == (int)(RTTY_HDR_SIZE + 1 + len - 1));
			} catch (const Poco::Exception &E) {
				Logger().log(E);
				return false;
			} catch (const std::exception &E) {
				LogStdException(E, "Cannot send keystrokes.");
				return false;
			}
		} else {
			auto Msg = std::make_unique<unsigned char[]>(len + RTTY_HDR_SIZE + 1);
			Msg.get()[0] = RTTYS_EndPoint::msgTypeTermData;
			Msg.get()[1] = ((len - 1 + 1) & 0xff00) >> 8;
			Msg.get()[2] = ((len - 1 + 1) & 0x00ff);
			Msg.get()[3] = Conn->sid_;
			memcpy((Msg.get() + RTTY_HDR_SIZE + 1), &buf[1], len - 1);
			try {
				auto Sent = SendBytes(Conn->DeviceSocket_,Msg.get(),
													 RTTY_HDR_SIZE + 1 + len - 1);
				return (Sent == (int)(RTTY_HDR_SIZE + 1 + len - 1));
			} catch (const Poco::Exception &E) {
				Logger().log(E);
				return false;
			} catch (const std::exception &E) {
				LogStdException(E, "Cannot send keystrokes");
				return false;
			}
		}
		return false;
	}

	bool RTTYS_server::WindowSize(std::shared_ptr<RTTYS_EndPoint> Conn, int cols, int rows) {
		u_char outBuf[8 + RTTY_SESSION_ID_LENGTH]{0};
		outBuf[0] = RTTYS_EndPoint::msgTypeWinsize;
		outBuf[1] = 0;
		outBuf[2] = 4 + 1;
		outBuf[3] = Conn->sid_;
		outBuf[RTTY_HDR_SIZE + 0 + 1] = cols >> 8;
		outBuf[RTTY_HDR_SIZE + 1 + 1] = cols & 0x00ff;
		outBuf[RTTY_HDR_SIZE + 2 + 1] = rows >> 8;
		outBuf[RTTY_HDR_SIZE + 3 + 1] = rows & 0x00ff;
		try {
			auto Sent = SendBytes(Conn->DeviceSocket_, outBuf, RTTY_HDR_SIZE + 4 + 1);
			return (Sent == (int)(RTTY_HDR_SIZE + 4 + 1));
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (const std::exception &E) {
			LogStdException(E, "Cannot send window size");
		}
		return false;
	}

	bool RTTYS_server::Login(const Poco::Net::Socket &Socket, std::shared_ptr<RTTYS_EndPoint> Conn) {
		Poco::Thread::sleep(500);
		u_char outBuf[RTTY_HDR_SIZE + RTTY_SESSION_ID_LENGTH]{0};
		outBuf[0] = RTTYS_EndPoint::msgTypeLogin;
		outBuf[1] = 0;
		outBuf[2] = 0;
		try {
			poco_debug(Logger(), fmt::format("TID:{} Starting login on device.",Conn->TID_));
			auto Sent = SendBytes(Socket,outBuf,RTTY_HDR_SIZE);
			Conn->completed_ = true;
			return Sent == RTTY_HDR_SIZE;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (const std::exception &E) {
			LogStdException(E, fmt::format("TID:{} Cannot send login.", Conn->TID_));
		}
		return false;
	}

	bool RTTYS_server::Logout(const Poco::Net::Socket &Socket, std::shared_ptr<RTTYS_EndPoint> Conn) {
		u_char outBuf[4 + RTTY_SESSION_ID_LENGTH]{0};
		outBuf[0] = RTTYS_EndPoint::msgTypeLogout;
		outBuf[1] = 0;
		outBuf[2] = 1;
		outBuf[3] = Conn->sid_;
		poco_debug(Logger(), fmt::format("{}: Logout", Conn->TID_));
		try {
			auto Sent = SendBytes(Socket, outBuf, RTTY_HDR_SIZE + 1);
			return Sent == (int)(RTTY_HDR_SIZE + 1);
		} catch (const Poco::Exception &E) {
			Logger().log(E);
			return false;
		} catch (const std::exception &E) {
			LogStdException(E, "Cannot send logout");
			return false;
		}
		return false;
	}

	std::string RTTYS_server::ReadString(unsigned char *Buffer, std::size_t BufferCurrentSize, std::size_t &BufferPos) {
		std::string Res;
		while(BufferPos<BufferCurrentSize) {
			auto c = Buffer[BufferPos++];
			if(c==0) {
				break;
			}
			Res += c;
		}
		return Res;
	}

	std::string RTTYS_server::ReadString(Poco::FIFOBuffer &Buffer) {
		std::string Res;
		while(Buffer.isReadable()) {
			auto c = *Buffer.begin();
			if(c==0) {
				Buffer.drain(1);
				break;
			}
			Res += c;
			Buffer.drain(1);
		}
		return Res;
	}

	bool RTTYS_server::SendToClient(Poco::Net::WebSocket &WebSocket, const u_char *Buf, int len) {
		WebSocket.sendFrame(
			Buf, len, Poco::Net::WebSocket::FRAME_FLAG_FIN | Poco::Net::WebSocket::FRAME_OP_BINARY);
		return true;
	}

	bool RTTYS_server::SendToClient(Poco::Net::WebSocket &WebSocket, const std::string &s) {
		WebSocket.sendFrame(s.c_str(), s.length());
		return true;
	}

	bool RTTYS_server::do_msgTypeLogin(const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, [[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger(), "Asking for login");
		auto EndPoint = Connected_.find(Socket.impl()->sockfd());
		if (EndPoint!=end(Connected_) && EndPoint->second->WSSocket_!= nullptr && EndPoint->second->WSSocket_->impl() != nullptr) {
			try {
				nlohmann::json doc;
				unsigned char Error = *buffer.begin();
				buffer.drain(1);
				if(Error==0) {
					EndPoint->second->sid_ = *buffer.begin();
					buffer.drain(1);
				} else {
					poco_error(Logger(),"Device login failed.");
					return false;
				}
				doc["type"] = "login";
				doc["err"] = Error;
				const auto login_msg = to_string(doc);
				return SendToClient(*EndPoint->second->WSSocket_, login_msg);
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			} catch (const std::exception &E) {
				LogStdException(E, "Cannot send login");
			}
		}
		return false;
	}

	bool RTTYS_server::do_msgTypeLogout([[maybe_unused]] const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, std::size_t msg_len) {
		poco_debug(Logger(), "Logout");
		// [[maybe_unused]] unsigned char logout_session_id = Buffer[BufferPos];
		buffer.drain(msg_len);
		return false;
	}

	bool RTTYS_server::do_msgTypeTermData(const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, std::size_t msg_len) {
		auto EndPoint = Connected_.find(Socket.impl()->sockfd());
		if (EndPoint!=end(Connected_) && EndPoint->second->WSSocket_!= nullptr && EndPoint->second->WSSocket_->impl() != nullptr) {
			try {
				buffer.drain(1);
				msg_len--;
/*				char temp_buf[RTTY_RECEIVE_BUFFER];
				buffer.peek(temp_buf,msg_len); */
				auto good = SendToClient(*EndPoint->second->WSSocket_, (unsigned char*) buffer.begin(), (int) msg_len );
				buffer.drain(msg_len);
				return good;
			} catch (const Poco::Exception &E) {
				Logger().log(E);
			} catch (const std::exception &E) {
				LogStdException(E, "Cannot send data to UI Client");
			}
		}
		return false;
	}

	bool RTTYS_server::do_msgTypeWinsize([[maybe_unused]] const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, std::size_t msg_len) {
		poco_debug(Logger(), "Asking for msgTypeWinsize");
		buffer.drain(msg_len);
		return true;
	}

	bool RTTYS_server::do_msgTypeCmd([[maybe_unused]] const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, std::size_t msg_len) {
		poco_debug(Logger(), "Asking for msgTypeCmd");
		buffer.drain(msg_len);
		return true;
	}

	bool RTTYS_server::do_msgTypeHeartbeat(const Poco::Net::Socket &Socket, [[maybe_unused]] Poco::FIFOBuffer &buffer, [[maybe_unused]] std::size_t msg_len) {
		try {
			std::cout << "Sending heartbeat" << std::endl;
			u_char MsgBuf[RTTY_HDR_SIZE + 16]{0};
			MsgBuf[0] = RTTYS_EndPoint::msgTypeHeartbeat;
			MsgBuf[1] = 0;
			MsgBuf[2] = 0;
			auto Sent = SendBytes(Socket,MsgBuf, RTTY_HDR_SIZE);
			return Sent == RTTY_HDR_SIZE;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
		} catch (const std::exception &E) {
			LogStdException(E, "Cannot send heartbeat");
		}
		return false;
	}

	bool RTTYS_server::do_msgTypeFile([[maybe_unused]] const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, std::size_t msg_len) {
		poco_debug(Logger(), "Asking for msgTypeFile");
		buffer.drain(msg_len);
		return true;
	}

	bool RTTYS_server::do_msgTypeHttp([[maybe_unused]] const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, std::size_t msg_len) {
		poco_debug(Logger(), "Asking for msgTypeHttp");
		buffer.drain(msg_len);
		return true;
	}

	bool RTTYS_server::do_msgTypeAck([[maybe_unused]] const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, std::size_t msg_len) {
		poco_debug(Logger(), "Asking for msgTypeAck");
		buffer.drain(msg_len);
		return true;
	}

	bool RTTYS_server::do_msgTypeMax([[maybe_unused]] const Poco::Net::Socket &Socket, Poco::FIFOBuffer &buffer, std::size_t msg_len) {
		poco_debug(Logger(), "Asking for msgTypeMax");
		buffer.drain(msg_len);
		return true;
	}


	RTTYS_EndPoint::RTTYS_EndPoint(const std::string &Id, const std::string &Token,
								   const std::string &SerialNumber, const std::string &UserName,
								   bool mTLS)
		: Id_(Id), Token_(Token), SerialNumber_(SerialNumber), UserName_(UserName),
	  	mTLS_(mTLS) {
		Created_ = std::chrono::high_resolution_clock::now();
	}

	RTTYS_EndPoint::~RTTYS_EndPoint() {
	}

} // namespace OpenWifi