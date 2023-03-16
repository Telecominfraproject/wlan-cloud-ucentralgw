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
			Poco::Net::SocketAddress Client;
			Poco::Net::StreamSocket NewSocket = pNf->socket().impl()->acceptConnection(Client);
			if (NewSocket.impl()->secure()) {
				auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(NewSocket.impl());
				auto PeerAddress_ = SS->peerAddress().host();
				auto CId_ = Utils::FormatIPv6(SS->peerAddress().toString());
				poco_debug(Logger(),fmt::format("Completing TLS handshake: {}", CId_));
				while (true) {
					auto V = SS->completeHandshake();
					if (V == 1)
						break;
				}
				poco_debug(Logger(),fmt::format("Completed TLS handshake: {}", CId_));

				if (enforce_mTLS_) {
					if (SS->havePeerCertificate()) {
						Poco::Crypto::X509Certificate PeerCert(SS->peerCertificate());
						auto CN = Poco::trim(Poco::toLower(PeerCert.commonName()));
						if (AP_WS_Server()->ValidateCertificate(CId_, PeerCert)) {
							poco_debug(
								Logger(),
								fmt::format("Device {} has been validated from {}.", CN, CId_));
							AddNewSocket(NewSocket);
							return;
						}
					}
					poco_debug(Logger(), fmt::format("Device cannot be validated from {}.", CId_));
				} else {
					AddNewSocket(NewSocket);
					return;
				}
				AddNewSocket(NewSocket);
				return;
			}
			NewSocket.close();
		} catch (const Poco::Exception &E) {
			std::cout << "Exception onDeviceAccept: " << E.what() << std::endl;
			Logger().log(E);
		}
	}

	void RTTYS_server::RemoveClientEventHandlers(Poco::Net::StreamSocket &Socket) {
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

	void RTTYS_server::AddNewSocket(Poco::Net::Socket &Socket) {
		std::lock_guard		G(ServerMutex_);

		Socket.setNoDelay(true);
		Socket.setKeepAlive(true);
		Socket.setBlocking(false);
		Socket.setReceiveBufferSize(RTTY_DEVICE_BUFSIZE);
		Socket.setSendBufferSize(RTTY_DEVICE_BUFSIZE);
		Poco::Timespan	TS2(0,100);
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

		Sockets_[Socket.impl()->sockfd()] = Socket;

	}

	void RTTYS_server::RemoveSocket(const Poco::Net::StreamSocket &Socket) {
		std::lock_guard		G(ServerMutex_);

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

	void RTTYS_server::AddClientEventHandlers(Poco::Net::StreamSocket &Socket,
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
		std::lock_guard	G(ServerMutex_);

		auto hint = Sockets_.find(fd);
		if(hint==end(Sockets_)) {
			poco_error(Logger(),fmt::format("Cannot find this socket: {}",fd));
			return -1;
		}
		return hint->second.impl()->sendBytes(buffer,len);
	}

	int RTTYS_server::SendBytes(const Poco::Net::Socket &Socket, const unsigned char *buffer, std::size_t len) {
		return Socket.impl()->sendBytes(buffer,len);
	}

	bool RTTYS_server::do_msgTypeRegister(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		bool good = true;
		try {

			auto fd = Socket.impl()->sockfd();
			std::string id_ = ReadString(Buffer, BufferCurrentSize, BufferPos);
			std::string desc_ = ReadString(Buffer, BufferCurrentSize, BufferPos);
			std::string token_ = ReadString(Buffer, BufferCurrentSize, BufferPos);

			poco_debug(Logger(),fmt::format("Device registration: description:{} id:{} token:{}", desc_, id_, token_));
			if (id_.size() != RTTY_DEVICE_TOKEN_LENGTH ||
				token_.size() != RTTY_DEVICE_TOKEN_LENGTH || desc_.empty()) {
				poco_warning(Logger(),fmt::format("Wrong register header. {} {} {}", id_,desc_,token_));
				return false;
			}

			//	find this device in our connectio end points...
			auto ConnectionEp = FindRegisteredEndPoint(id_, token_);
			if (ConnectionEp == nullptr) {
				poco_warning(Logger(), fmt::format("Unknown session {} from device.", id_));
				return false;
			}

			if (ConnectionEp->mTLS_) {
				auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(Socket.impl());
				auto PeerAddress_ = SS->peerAddress().host();
				auto CId_ = Utils::FormatIPv6(SS->peerAddress().toString());
				if (SS->havePeerCertificate()) {
					Poco::Crypto::X509Certificate PeerCert(SS->peerCertificate());
					auto CN = Poco::trim(Poco::toLower(PeerCert.commonName()));
					if (AP_WS_Server()->ValidateCertificate(CId_, PeerCert)) {
						poco_debug(
							Logger(),
							fmt::format("Device mTLS {} has been validated from {}.", CN, CId_));
					} else {
						poco_warning(Logger(), fmt::format("Device failed mTLS validation {}. Certificate fails validation.", CId_));
						return false;
					}
				} else {
					poco_warning(Logger(), fmt::format("Device failed mTLS validation {} (no certificate).", CId_));
					return false;
				}
			}

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
			if(ConnectionEp->WSSocket_!= nullptr) {
				Login(Socket, ConnectionEp);
			}
			return true;
		} catch (...) {
			good = false;
		}

		return good;
	}

	void RTTYS_server::onConnectedDeviceSocketReadable(
		const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {

		std::shared_ptr<RTTYS_EndPoint> ConnectionPtr;

		try {
			unsigned char 	Buffer[64000];
			std::size_t 	BufferCurrentSize=0, BufferPos=0;

			try {
				BufferCurrentSize = pNf->socket().impl()->receiveBytes(Buffer, sizeof(Buffer));
				if(BufferCurrentSize==0) {
					poco_warning(Logger(), "Device Closing connection - 0 bytes received.");
					EndConnection( pNf->socket(), __LINE__ );
					return;
				}
			} catch (const Poco::TimeoutException &E) {
				poco_warning(Logger(), "Receive timeout");
				EndConnection( pNf->socket(), __LINE__ );
				return;
			} catch (const Poco::Net::NetException &E) {
				Logger().log(E);
				EndConnection( pNf->socket(), __LINE__ );
				return;
			}

			std::lock_guard	Lock(ServerMutex_);
			bool good = true;

			while (BufferPos<BufferCurrentSize && good) {
				unsigned char LastCommand=0;
				if (BufferCurrentSize >= RTTY_HDR_SIZE) {
					LastCommand = Buffer[BufferPos+0];
					BufferPos+=RTTY_HDR_SIZE;
				} else {
					good = false;
					std::cout << "Funky..." << BufferCurrentSize << std::endl;
					continue;
				}

				switch (LastCommand) {
					case RTTYS_EndPoint::msgTypeRegister: {
						good = do_msgTypeRegister(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeLogin: {
						good = do_msgTypeLogin(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeLogout: {
						good = do_msgTypeLogout(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeTermData: {
						good = do_msgTypeTermData(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeWinsize: {
						good = do_msgTypeWinsize(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeCmd: {
						good = do_msgTypeCmd(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeHeartbeat: {
						good = do_msgTypeHeartbeat(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeFile: {
						good = do_msgTypeFile(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeHttp: {
						good = do_msgTypeHttp(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeAck: {
						good = do_msgTypeAck(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					case RTTYS_EndPoint::msgTypeMax: {
						good = do_msgTypeMax(pNf->socket(), Buffer, BufferCurrentSize, BufferPos);
					} break;
					default: {
						poco_warning(Logger(),
									 fmt::format("Unknown command {}. GW closing connection.",
												 (int)LastCommand));
						good = false;
					}
				}
			}

			if (!good) {
				EndConnection(pNf->socket(), __LINE__);
			}
		} catch (const Poco::Exception &E) {
			Logger().log(E);
			EndConnection(pNf->socket(),__LINE__);
		} catch (...) {
			EndConnection(pNf->socket(),__LINE__);
		}
	}

	void RTTYS_server::onConnectedDeviceSocketShutdown(
		const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		EndConnection(pNf->socket(),__LINE__);
	}

	void RTTYS_server::onConnectedDeviceSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		EndConnection(pNf->socket(),__LINE__);
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
			if (Connection == nullptr) {
				std::cout << "NULL EndPoint Client" << std::endl;
				return;
			}

			if (Connection->WSSocket_ == nullptr) {
				std::cout << "NULL WS Client" << std::endl;
				return;
			}

			int flags;
			unsigned char FrameBuffer[1024];

			auto ReceivedBytes =
				Connection->WSSocket_->receiveFrame(FrameBuffer, sizeof(FrameBuffer), flags);
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
					poco_trace(Logger(), "Client closing connection.");
					EndConnection(Connection,__LINE__);
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
									poco_error(Logger(), "Winsize shutdown.");
									EndConnection(Connection,__LINE__);
									return;
								}
							}
						}
					} catch (...) {
						// just ignore parse errors
						poco_error(Logger(), "Frame text exception shutdown.");
						EndConnection(Connection,__LINE__);
						return;
					}
				}
			} break;
			case Poco::Net::WebSocket::FRAME_OP_BINARY: {
				if (ReceivedBytes == 0) {
					poco_error(Logger(), "Client closing connection.");
					EndConnection(Connection,__LINE__);
					return;
				} else {
					poco_trace(Logger(),
							   fmt::format("Sending {} key strokes to device.", ReceivedBytes));
					if (!RTTYS_server().KeyStrokes(Connection, FrameBuffer, ReceivedBytes)) {
						poco_error(Logger(), "Cannot send keys to device. Close connection.");
						EndConnection(Connection,__LINE__);
						return;
					}
				}
			} break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
				poco_trace(Logger(), "Frame close shutdown.");
				EndConnection(Connection,__LINE__);
				return;
			} break;

			default: {
			}
			}
		} catch (...) {
			poco_error(Logger(), "Frame readable shutdown.");
			if (Client != Clients_.end() && Connection != nullptr) {
				EndConnection(Connection,__LINE__);
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
		EndConnection(Client->second,__LINE__);
	}

	void RTTYS_server::onClientSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		std::lock_guard	Lock(ServerMutex_);
		auto Client = Clients_.find(pNf->socket().impl()->sockfd());
		if (Client == end(Clients_)) {
			poco_warning(Logger(), fmt::format("Cannot find client socket: {}",
											   pNf->socket().impl()->sockfd()));
			return;
		}
		EndConnection(Client->second,__LINE__);
	}

	void RTTYS_server::SendData(std::shared_ptr<RTTYS_EndPoint> &Connection, const u_char *Buf,
								size_t len) {
		if (Connection->WSSocket_ != nullptr) {
			try {
				Connection->WSSocket_->sendFrame(Buf, len,
												 Poco::Net::WebSocket::FRAME_FLAG_FIN |
													 Poco::Net::WebSocket::FRAME_OP_BINARY);
				return;
			} catch (...) {
				poco_error(Logger(), "SendData shutdown.");
			}
		}
		EndConnection(Connection,__LINE__);
	}

	void RTTYS_server::SendData(std::shared_ptr<RTTYS_EndPoint> &Connection, const std::string &s) {
		if (Connection->WSSocket_ != nullptr) {
			try {
				Connection->WSSocket_->sendFrame(s.c_str(), s.length());
				return;
			} catch (...) {
				poco_error(Logger(), "SendData shutdown.");
			}
		}
		EndConnection(Connection,__LINE__);
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
			EndPoint->second->WSSocket_ = std::make_unique<Poco::Net::WebSocket>(request, response);
			EndPoint->second->ClientConnected_ = std::chrono::high_resolution_clock::now();
			EndPoint->second->WSSocket_->setBlocking(false);
			EndPoint->second->WSSocket_->setNoDelay(true);
			EndPoint->second->WSSocket_->setKeepAlive(true);
			AddClientEventHandlers(*EndPoint->second->WSSocket_, EndPoint->second);
			if (EndPoint->second->DeviceIsAttached_ && !EndPoint->second->completed_) {
				auto hint = Sockets_.find(EndPoint->second->Device_fd);
				if(hint!=end(Sockets_))
					Login(hint->second, EndPoint->second);
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
				EndPoint = EndConnection(EndPoint->second,__LINE__);
			} else {
				++EndPoint;
			}
		}

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

	std::map<std::string, std::shared_ptr<RTTYS_EndPoint>>::iterator RTTYS_server::EndConnection(std::shared_ptr<RTTYS_EndPoint> Connection, std::uint64_t Line) {

		std::lock_guard	G(ServerMutex_);
		auto hint1 = Sockets_.find(Connection->Device_fd);
		if(hint1!=end(Sockets_))
			RemoveSocket(hint1->second);

		//	find the client linked to this one...
		if(Connection->WSSocket_!= nullptr) {
			RemoveClientEventHandlers(*Connection->WSSocket_);
			Connection->WSSocket_->close();
		}
		poco_debug(Logger(),fmt::format("Closing connection at line {}",Line));
		auto hint2 = EndPoints_.find(Connection->Id_);
		return EndPoints_.erase(hint2);
	}

	void RTTYS_server::EndConnection(const Poco::Net::Socket &Socket, std::uint32_t Line) {
		//	remove the device
		auto fd = Socket.impl()->sockfd();
		std::lock_guard	G(ServerMutex_);
		RemoveSocket(Socket);

		//	find the client linked to this one...
		auto hint = Connected_.find(fd);
		if(hint!=end(Connected_)) {
			if(hint->second->WSSocket_!= nullptr) {
				RemoveClientEventHandlers(*hint->second->WSSocket_);
				hint->second->WSSocket_->close();
			}
		} else {
			std::cout << "Cannot find the associated WS..." << std::endl;
		}
		EndPoints_.erase(hint->second->Id_);
		poco_debug(Logger(),fmt::format("Closing connection at line {}",Line));
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
			auto Sent = SendBytes(Socket,outBuf,3);
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

	bool RTTYS_server::SendToClient(Poco::Net::WebSocket &WebSocket, const u_char *Buf, int len) {
		WebSocket.sendFrame(
			Buf, len, Poco::Net::WebSocket::FRAME_FLAG_FIN | Poco::Net::WebSocket::FRAME_OP_BINARY);
		return true;
	}

	bool RTTYS_server::SendToClient(Poco::Net::WebSocket &WebSocket, const std::string &s) {
		WebSocket.sendFrame(s.c_str(), s.length());
		return true;
	}

	bool RTTYS_server::do_msgTypeLogin(const Poco::Net::Socket &Socket, unsigned char *Buffer, [[maybe_unused]] std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		poco_debug(Logger(), "Asking for login");
		auto EndPoint = Connected_.find(Socket.impl()->sockfd());
		if (EndPoint!=end(Connected_) && EndPoint->second->WSSocket_ != nullptr) {
			try {
				nlohmann::json doc;
				unsigned char Error = Buffer[BufferPos++];
				if(Error==0) {
					EndPoint->second->sid_ = Buffer[BufferPos++];
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

	bool RTTYS_server::do_msgTypeLogout([[maybe_unused]] const Poco::Net::Socket &Socket, unsigned char *Buffer, [[maybe_unused]] std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		poco_debug(Logger(), "Logout");
		[[maybe_unused]] unsigned char logout_session_id = Buffer[BufferPos++];

		return false;
	}

	bool RTTYS_server::do_msgTypeTermData(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		auto EndPoint = Connected_.find(Socket.impl()->sockfd());
		if (EndPoint!=end(Connected_) && EndPoint->second->WSSocket_ != nullptr) {
			try {
				BufferPos++;
				auto good = SendToClient(*EndPoint->second->WSSocket_, &Buffer[BufferPos],
										 BufferCurrentSize - BufferPos);
				BufferPos = BufferCurrentSize;
				return good;
			} catch (const Poco::Exception &E) {
				Logger().log(E);
				return false;
			} catch (const std::exception &E) {
				LogStdException(E, "Cannot send data to UI Client");
				return false;
			}
		}
		return false;
	}

	bool RTTYS_server::do_msgTypeWinsize([[maybe_unused]] const Poco::Net::Socket &Socket, [[maybe_unused]] unsigned char *Buffer,[[maybe_unused]] std::size_t  BufferCurrentSize, [[maybe_unused]] std::size_t  &BufferPos) {
		poco_debug(Logger(), "Asking for msgTypeWinsize");
		BufferPos = BufferCurrentSize;
		return true;
	}

	bool RTTYS_server::do_msgTypeCmd([[maybe_unused]] const Poco::Net::Socket &Socket, [[maybe_unused]] unsigned char *Buffer,[[maybe_unused]]  std::size_t  BufferCurrentSize, [[maybe_unused]] std::size_t  &BufferPos) {
		poco_debug(Logger(), "Asking for msgTypeCmd");
		BufferPos = BufferCurrentSize;
		return true;
	}

	bool RTTYS_server::do_msgTypeHeartbeat(const Poco::Net::Socket &Socket, [[maybe_unused]] unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		try {
			u_char MsgBuf[RTTY_HDR_SIZE + 16]{0};
			BufferPos = BufferCurrentSize;
			MsgBuf[0] = RTTYS_EndPoint::msgTypeHeartbeat;
			MsgBuf[1] = 0;
			MsgBuf[2] = 0;
			auto Sent = SendBytes(Socket,MsgBuf, RTTY_HDR_SIZE);
			return Sent == RTTY_HDR_SIZE;
		} catch (const Poco::Exception &E) {
			Logger().log(E);
			return false;
		} catch (const std::exception &E) {
			LogStdException(E, "Cannot send heartbeat");
			return false;
		}
		return false;
	}

	bool RTTYS_server::do_msgTypeFile([[maybe_unused]] const Poco::Net::Socket &Socket, [[maybe_unused]] unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		poco_debug(Logger(), "Asking for msgTypeFile");
		BufferPos = BufferCurrentSize;
		return true;
	}

	bool RTTYS_server::do_msgTypeHttp([[maybe_unused]] const Poco::Net::Socket &Socket, [[maybe_unused]] unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		poco_debug(Logger(), "Asking for msgTypeHttp");
		BufferPos = BufferCurrentSize;
		return true;
	}

	bool RTTYS_server::do_msgTypeAck([[maybe_unused]] const Poco::Net::Socket &Socket, [[maybe_unused]] unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		poco_debug(Logger(), "Asking for msgTypeAck");
		BufferPos = BufferCurrentSize;
		return true;
	}

	bool RTTYS_server::do_msgTypeMax([[maybe_unused]] const Poco::Net::Socket &Socket, [[maybe_unused]] unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos) {
		BufferPos = BufferCurrentSize;
		poco_debug(Logger(), "Asking for msgTypeMax");
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