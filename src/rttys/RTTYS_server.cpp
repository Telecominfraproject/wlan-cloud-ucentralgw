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
				DeviceSocket_ = std::make_unique<Poco::Net::ServerSocket>(SockAddr, 64);
				Reactor_.addEventHandler(
					*DeviceSocket_, Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
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

				SecureDeviceSocket_ = std::make_unique<Poco::Net::SecureServerSocket>(
					SockAddr, 64, DeviceSecureContext);
				Reactor_.addEventHandler(
					*SecureDeviceSocket_,
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
							auto NewDevice = std::make_shared<RTTYS_EndPoint>(NewSocket, ++CurrentTID_,Logger());
							std::lock_guard	Lock(ServerMutex_);
							AddConnectedDeviceEventHandlers(NewDevice);
							return;
						}
					}
					poco_debug(Logger(), fmt::format("Device cannot be validated from {}.", CId_));
				} else {
					auto NewDevice = std::make_shared<RTTYS_EndPoint>(NewSocket, ++CurrentTID_, Logger());
					std::lock_guard	Lock(ServerMutex_);
					AddConnectedDeviceEventHandlers(NewDevice);
					return;
				}
				auto NewDevice = std::make_shared<RTTYS_EndPoint>(NewSocket, ++CurrentTID_, Logger());
				std::lock_guard	Lock(ServerMutex_);
				AddConnectedDeviceEventHandlers(NewDevice);
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

	void RTTYS_server::RemoveConnectedDeviceEventHandlers(Poco::Net::StreamSocket &Socket) {
		int fd = Socket.impl()->sockfd();
		if(Reactor_.has(Socket)) {
			Reactor_.removeEventHandler(Socket,
										Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
											*this, &RTTYS_server::onConnectedDeviceSocketReadable));
			Reactor_.removeEventHandler(Socket,
										Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
											*this, &RTTYS_server::onConnectedDeviceSocketShutdown));
			Reactor_.removeEventHandler(Socket,
										Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
											*this, &RTTYS_server::onConnectedDeviceSocketError));
		}
		ConnectedDevices_.erase(fd);
	}

	void RTTYS_server::AddConnectedDeviceEventHandlers(std::shared_ptr<RTTYS_EndPoint> ep) {
		int fd = ep->DeviceSocket_->impl()->sockfd();
		Reactor_.addEventHandler(*ep->DeviceSocket_,
								 Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
									 *this, &RTTYS_server::onConnectedDeviceSocketReadable));
		Reactor_.addEventHandler(*ep->DeviceSocket_,
								 Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
									 *this, &RTTYS_server::onConnectedDeviceSocketShutdown));
		Reactor_.addEventHandler(*ep->DeviceSocket_,
								 Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
									 *this, &RTTYS_server::onConnectedDeviceSocketError));
		ep->DeviceSocket_->setNoDelay(true);
		ep->DeviceSocket_->setKeepAlive(true);
		ep->DeviceSocket_->setBlocking(false);
		ep->DeviceSocket_->setReceiveBufferSize(RTTY_DEVICE_BUFSIZE);
		ep->DeviceSocket_->setSendBufferSize(RTTY_DEVICE_BUFSIZE);
		Poco::Timespan	TS2(0,100);
		ep->DeviceSocket_->setReceiveTimeout(TS2);
		ConnectingDevices_[fd] = ep;
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

	int RTTYS_EndPoint::send_ssl_bytes(unsigned char *b,int size) {

		return DeviceSocket_->sendBytes(b,size);
	}

	bool RTTYS_EndPoint::do_msgTypeRegister(int fd) {
		bool good = true;
		try {
//			std::cout << __LINE__ << std::endl;
			std::string id_ = ReadString();
//			std::cout << __LINE__ << std::endl;
			std::string desc_ = ReadString();
//			std::cout << __LINE__ << std::endl;
			std::string token_ = ReadString();
//			std::cout << __LINE__ << std::endl;

			poco_debug(Logger_,fmt::format("Device registration: description:{} id:{} token:{}", desc_, id_, token_));
//			std::cout << __LINE__ << std::endl;
			if (id_.size() != RTTY_DEVICE_TOKEN_LENGTH ||
				token_.size() != RTTY_DEVICE_TOKEN_LENGTH || desc_.empty()) {
				poco_warning(Logger_,fmt::format("Wrong register header. {} {} {}", id_,desc_,token_));
				return false;
			}
//			std::cout << __LINE__ << std::endl;

			//	find this device in our connectio end points...
			auto Connection = RTTYS_server()->FindConnection(id_, token_);
//			std::cout << __LINE__ << std::endl;
			if (Connection == nullptr) {
				poco_warning(Logger_, fmt::format("Unknown session {} from device.", id_));
				std::cout << "Session '" << id_ << "' invalid" << std::endl;
				return false;
			}
//			std::cout << __LINE__ << std::endl;

			//	are we connected or not ?
			{
				auto ConnectingEp = RTTYS_server()->FindConnectingDevice(fd);
				if (ConnectingEp == nullptr) {
					poco_warning(Logger_, fmt::format("Unknown socket {} from device.", fd));
					return false;
				} else {
					Connection->DeviceSocket_ = std::make_unique<Poco::Net::StreamSocket>(*ConnectingEp->DeviceSocket_);
					Connection->TID_ = ConnectingEp->TID_;
					RTTYS_server()->RemoveConnectingDevice(fd);
				}
			}

			if (Connection->mTLS_) {
				auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(Connection->DeviceSocket_->impl());
				auto PeerAddress_ = SS->peerAddress().host();
				auto CId_ = Utils::FormatIPv6(SS->peerAddress().toString());
				if (SS->havePeerCertificate()) {
					Poco::Crypto::X509Certificate PeerCert(SS->peerCertificate());
					auto CN = Poco::trim(Poco::toLower(PeerCert.commonName()));
					if (AP_WS_Server()->ValidateCertificate(CId_, PeerCert)) {
						poco_debug(
							Logger_,
							fmt::format("Device mTLS {} has been validated from {}.", CN, CId_));
					} else {
						poco_warning(Logger_, fmt::format("Device failed mTLS validation {}. Certificate fails validation.", CId_));
						return false;
					}
				} else {
					poco_warning(Logger_, fmt::format("Device failed mTLS validation {} (no certificate).", CId_));
					return false;
				}
			}

			RTTYS_server()->AddConnectedDevice(fd,Connection);

			u_char OutBuf[8];
			OutBuf[0] = RTTYS_EndPoint::msgTypeRegister;
			OutBuf[1] = 0; //	Data length
			OutBuf[2] = 4; //
			OutBuf[3] = 0; //	Error
			OutBuf[4] = 'O';
			OutBuf[5] = 'K';
			OutBuf[6] = 0;
			if (Connection->send_ssl_bytes(OutBuf, 7) != 7) {
				poco_error(
					Logger_,
					fmt::format("{}: Description:{} Could not send data to complete registration",
								id_, desc_));
				return false;
			}
			Connection->DeviceConnected_ = std::chrono::high_resolution_clock::now();
			if(Connection->WSSocket_!= nullptr) {
				Connection->Login();
			}
			return true;
		} catch (...) {
			good = false;
		}

		return good;
	}

	void RTTYS_server::onConnectedDeviceSocketReadable(
		const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {

		int fd = pNf->socket().impl()->sockfd();

		// std::cout << __LINE__ << " " << "fd=" << fd << std::endl;

		std::lock_guard	Lock(ServerMutex_);
		std::shared_ptr<RTTYS_EndPoint> ConnectionPtr;
		try {
			ConnectionPtr = FindConnectedDevice(fd);
			if (ConnectionPtr == nullptr) {
				ConnectionPtr = FindConnectingDevice(fd);
				if(ConnectionPtr == nullptr) {
					poco_warning(Logger(), fmt::format("Cannot find device socket: {}",
													   fd));
					return;
				} else {
					std::cout << "Connecting device found" << std::endl;
				}
			} else {
				std::cout << "Connected device found" << std::endl;
			}

			if (ConnectionPtr->DeviceSocket_ == nullptr) {
				poco_warning(Logger(), fmt::format("Connection has invalid socket: {}",
												   pNf->socket().impl()->sockfd()));
				return;
			}

			RTTYS_EndPoint * Connection = ConnectionPtr.get();

			bool good = true;

			std::size_t received_bytes=0;
			try {
//				std::cout << __LINE__ << std::endl;
				poco_warning(Logger(), "About to receive bytes");
//				std::cout << __LINE__ << std::endl;
//					Connection->DeviceSocket_->receiveBytes(*Connection->DeviceInBuf_);
				Connection->BufPos_=0;
//				std::cout << __LINE__ << std::endl;
				received_bytes = Connection->BufferCurrentSize_ =  // recv(fd, Connection->Buffer_, sizeof(Connection->Buffer_),0);
						  Connection->DeviceSocket_->receiveBytes( Connection->Buffer_, sizeof(Connection->Buffer_));
//				std::cout << __LINE__ << std::endl;

				poco_warning(Logger(), fmt::format("Received {} bytes", received_bytes));
//				std::cout << __LINE__ << std::endl;
			} catch (const Poco::TimeoutException &E) {
				poco_warning(Logger(), "Receive timeout");
				return;
			} catch (const Poco::Net::NetException &E) {
				Logger().log(E);
				EndConnection(ConnectionPtr,__LINE__);
				return;
			}

//			std::cout << __LINE__ << std::endl;

			if (received_bytes == 0) {
//				std::cout << __LINE__ << std::endl;
				good = false;
				poco_debug(Logger(), "Device Closing connection - 0 bytes received.");
			} else {
//				std::cout << __LINE__ << std::endl;
				while (Connection->BufPos_<Connection->BufferCurrentSize_ && good) {
//					std::cout << __LINE__ << std::endl;
					uint32_t msg_len = 0;
//					std::cout << __LINE__ << std::endl;
					if (Connection->waiting_for_bytes_ != 0) {
//						std::cout << __LINE__ << std::endl;
						poco_warning(Logger(),fmt::format("Waiting for {} bytes",Connection->waiting_for_bytes_));
//						std::cout << __LINE__ << std::endl;
					} else {
//						std::cout << __LINE__ << std::endl;
						if (Connection->BufferCurrentSize_ >= RTTY_HDR_SIZE) {
//							std::cout << __LINE__ << " BP:" << Connection->BufPos_ << std::endl;
							Connection->last_command_ = Connection->Buffer_[Connection->BufPos_+0];
							msg_len = Connection->Buffer_[Connection->BufPos_+1] * 256 + Connection->Buffer_[Connection->BufPos_+2];
//							std::cout << __LINE__ << std::endl;
							Connection->BufPos_+=RTTY_HDR_SIZE;
						} else {
							good = false;
							std::cout << "Funky..." << Connection->BufferCurrentSize_ << std::endl;
							continue;
						}
					}

					std::cout << __LINE__ << " RB:" << received_bytes << " " << (int) Connection->last_command_ << " " << (int) Connection->Buffer_[0] << " " << msg_len << std::endl;
					switch (Connection->last_command_) {

					case RTTYS_EndPoint::msgTypeRegister: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeRegister(fd);
					} break;
					case RTTYS_EndPoint::msgTypeLogin: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeLogin(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeLogout: {
//						std::cout << __LINE__ << std::endl;
						// good = EndPoint->do_msgTypeLogout(msg_len);
						good = false;
					} break;
					case RTTYS_EndPoint::msgTypeTermData: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeTermData(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeWinsize: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeWinsize(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeCmd: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeCmd(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeHeartbeat: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeHeartbeat(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeFile: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeFile(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeHttp: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeHttp(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeAck: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeAck(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeMax: {
//						std::cout << __LINE__ << std::endl;
						good = Connection->do_msgTypeMax(msg_len);
					} break;
					default: {
//						std::cout << __LINE__ << std::endl;
						poco_warning(Logger(),
									 fmt::format("Unknown command {}. GW closing connection.",
												 (int)Connection->last_command_));
						good = false;
					}
					}
				}
			}

//			std::cout << __LINE__ << std::endl;
			if (!good) {
//				std::cout << __LINE__ << std::endl;
				EndConnection(ConnectionPtr, __LINE__);
			}
		} catch (const Poco::Exception &E) {
//			std::cout << __LINE__ << std::endl;
			Logger().log(E);
			if (ConnectionPtr != nullptr) {
				EndConnection(ConnectionPtr,__LINE__);
			}
		} catch (...) {
//			std::cout << __LINE__ << std::endl;
			if (ConnectionPtr != nullptr) {
				EndConnection(ConnectionPtr,__LINE__);
			}
		}
	}

	void RTTYS_server::onConnectedDeviceSocketShutdown(
		const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		std::lock_guard	Lock(ServerMutex_);

		auto Device = ConnectedDevices_.find(pNf->socket().impl()->sockfd());
		if (Device == end(ConnectedDevices_)) {
			poco_warning(Logger(), fmt::format("Cannot find device socket: {}",
											   pNf->socket().impl()->sockfd()));
			return;
		}
		EndConnection(Device->second,__LINE__);
	}

	void RTTYS_server::onConnectedDeviceSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		std::lock_guard	Lock(ServerMutex_);

		auto Device = ConnectedDevices_.find(pNf->socket().impl()->sockfd());
		if (Device == end(ConnectedDevices_)) {
			poco_warning(Logger(), fmt::format("Cannot find device socket: {}",
											   pNf->socket().impl()->sockfd()));
			return;
		}
		EndConnection(Device->second,__LINE__);
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
					if (Connection->DeviceSocket_ != nullptr) {
						std::string Frame((const char *)FrameBuffer, ReceivedBytes);
						try {
							auto Doc = nlohmann::json::parse(Frame);
							if (Doc.contains("type")) {
								auto Type = Doc["type"];
								if (Type == "winsize") {
									auto cols = Doc["cols"];
									auto rows = Doc["rows"];
									if (!Connection->WindowSize(cols, rows)) {
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
					if (Connection->DeviceSocket_ != nullptr) {
						if (!Connection->KeyStrokes(FrameBuffer, ReceivedBytes)) {
							poco_error(Logger(), "Cannot send keys to device. Close connection.");
							EndConnection(Connection,__LINE__);
							return;
						}
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
			if (EndPoint->second->DeviceSocket_ != nullptr && !EndPoint->second->completed_) {
				EndPoint->second->Login();
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
				ConnectedDevices_.size(), ConnectingDevices_.size()));
		}
	}

	std::map<std::string, std::shared_ptr<RTTYS_EndPoint>>::iterator  RTTYS_server::EndConnection(std::shared_ptr<RTTYS_EndPoint> Connection, std::uint64_t line) {
		poco_trace(Logger(),fmt::format("Ending connection: line {}",line));
		if(Connection->DeviceSocket_!= nullptr) {
			Connection->DeviceDisconnected_ = std::chrono::high_resolution_clock::now();
			TotalConnectedDeviceTime_ += Connection->DeviceDisconnected_ - Connection->DeviceConnected_;
			RemoveConnectedDeviceEventHandlers(*Connection->DeviceSocket_);
		}
		if(Connection->WSSocket_!= nullptr) {
			Connection->ClientDisconnected_ = std::chrono::high_resolution_clock::now();
			TotalConnectedClientTime_ += Connection->ClientDisconnected_ - Connection->ClientConnected_;
			RemoveClientEventHandlers(*Connection->WSSocket_);
		}
		auto hint = EndPoints_.find(Connection->Id_);
		return EndPoints_.erase(hint);
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
		EndPoints_[Id] = std::make_shared<RTTYS_EndPoint>(Id, Token, SerialNumber, UserName, mTLS, Logger());
		++TotalEndPoints_;
		ServerMutex_.unlock();
		return true;
	}

	bool RTTYS_server::ValidId(const std::string &Id) {
		std::lock_guard	Lock(ServerMutex_);
		return EndPoints_.find(Id) != EndPoints_.end();
	}

	bool RTTYS_EndPoint::KeyStrokes(const u_char *buf, size_t len) {

		if (DeviceSocket_ != nullptr) {
			if (len <= (sizeof(small_buf_) - RTTY_HDR_SIZE - 1)) {
				small_buf_[0] = msgTypeTermData;
				small_buf_[1] = ((len - 1 + 1) & 0xff00) >> 8;
				small_buf_[2] = ((len - 1 + 1) & 0x00ff);
				small_buf_[3] = sid_;
				memcpy(&small_buf_[RTTY_HDR_SIZE + 1], &buf[1], len - 1);
				try {
					auto Sent = send_ssl_bytes(small_buf_,
														 RTTY_HDR_SIZE + 1 + len - 1);
					return (Sent == (int)(RTTY_HDR_SIZE + 1 + len - 1));
				} catch (const Poco::Exception &E) {
					Logger_.log(E);
					return false;
				} catch (const std::exception &E) {
					RTTYS_server()->LogStdException(E, "Cannot send keystrokes.");
					return false;
				}
			} else {
				auto Msg = std::make_unique<unsigned char[]>(len + RTTY_HDR_SIZE + 1);
				Msg.get()[0] = msgTypeTermData;
				Msg.get()[1] = ((len - 1 + 1) & 0xff00) >> 8;
				Msg.get()[2] = ((len - 1 + 1) & 0x00ff);
				Msg.get()[3] = sid_;
				memcpy((Msg.get() + RTTY_HDR_SIZE + 1), &buf[1], len - 1);
				try {
					auto Sent = send_ssl_bytes(Msg.get(),
														 RTTY_HDR_SIZE + 1 + len - 1);
					return (Sent == (int)(RTTY_HDR_SIZE + 1 + len - 1));
				} catch (const Poco::Exception &E) {
					Logger_.log(E);
					return false;
				} catch (const std::exception &E) {
					RTTYS_server()->LogStdException(E, "Cannot send keystrokes");
					return false;
				}
			}
		}
		return false;
	}

	bool RTTYS_EndPoint::WindowSize(int cols, int rows) {
		if (DeviceSocket_ != nullptr) {
			u_char outBuf[8 + RTTY_SESSION_ID_LENGTH]{0};
			outBuf[0] = msgTypeWinsize;
			outBuf[1] = 0;
			outBuf[2] = 4 + 1;
			outBuf[3] = sid_;
			outBuf[RTTY_HDR_SIZE + 0 + 1] = cols >> 8;
			outBuf[RTTY_HDR_SIZE + 1 + 1] = cols & 0x00ff;
			outBuf[RTTY_HDR_SIZE + 2 + 1] = rows >> 8;
			outBuf[RTTY_HDR_SIZE + 3 + 1] = rows & 0x00ff;
			try {
				auto Sent = send_ssl_bytes(outBuf, RTTY_HDR_SIZE + 4 + 1);
				return (Sent == (int)(RTTY_HDR_SIZE + 4 + 1));
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				return false;
			} catch (const std::exception &E) {
				RTTYS_server()->LogStdException(E, "Cannot send window size");
				return false;
			}
		}
		return false;
	}

	bool RTTYS_EndPoint::Login() {
		if (DeviceSocket_ != nullptr) {
			Poco::Thread::sleep(500);
			u_char outBuf[RTTY_HDR_SIZE + RTTY_SESSION_ID_LENGTH]{0};
			outBuf[0] = msgTypeLogin;
			outBuf[1] = 0;
			outBuf[2] = 0;
			try {
				poco_debug(Logger_, fmt::format("TID:{} Starting loggin on device.",TID_));
				auto Sent = send_ssl_bytes(outBuf,3);
				completed_ = true;
				return Sent == RTTY_HDR_SIZE;
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				return false;
			} catch (const std::exception &E) {
				RTTYS_server()->LogStdException(E, fmt::format("TID:{} Cannot send login.", TID_));
				return false;
			}
		}
		return false;
	}

	bool RTTYS_EndPoint::Logout() {
		if (DeviceSocket_ != nullptr) {
			u_char outBuf[4 + RTTY_SESSION_ID_LENGTH]{0};
			outBuf[0] = msgTypeLogout;
			outBuf[1] = 0;
			outBuf[2] = 1;
			outBuf[3] = sid_;
			poco_debug(Logger_, "{}: Logout");
			try {
				auto Sent = send_ssl_bytes(outBuf, RTTY_HDR_SIZE + 1);
				return Sent == (int)(RTTY_HDR_SIZE + 1);
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				return false;
			} catch (const std::exception &E) {
				RTTYS_server()->LogStdException(E, "Cannot send logout");
				return false;
			}
		}
		return false;
	}

	std::string RTTYS_EndPoint::ReadString() {
		std::string Res;
		while(BufPos_<BufferCurrentSize_) {
			auto c = Buffer_[BufPos_++];
			if(c==0) {
				break;
			}
			Res += c;
		}
		return Res;
	}

	bool RTTYS_EndPoint::do_msgTypeLogin([[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger_, "Asking for login");
		if (WSSocket_ != nullptr) {
			try {
				nlohmann::json doc;
				unsigned char Error = Buffer_[BufPos_++];
				if(Error==0) {
					sid_ = Buffer_[BufPos_++];
				} else {
					poco_error(Logger_,"Device login failed.");
					return false;
				}
				doc["type"] = "login";
				doc["err"] = Error;
				const auto login_msg = to_string(doc);
				return SendToClient(login_msg);
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				return false;
			} catch (const std::exception &E) {
				RTTYS_server()->LogStdException(E, "Cannot send login");
				return false;
			}
		}
		return false;
	}

	bool RTTYS_EndPoint::do_msgTypeLogout([[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger_, "Logout");
		[[maybe_unused]] unsigned char logout_session_id = Buffer_[BufPos_++];

		return false;
	}

	bool RTTYS_EndPoint::do_msgTypeTermData(std::size_t msg_len) {
		bool good;
		try {
			if (waiting_for_bytes_ > 0) {
				if ((BufferCurrentSize_-BufPos_) < waiting_for_bytes_) {
					waiting_for_bytes_ = waiting_for_bytes_ - (BufferCurrentSize_-BufPos_);
					good = SendToClient(&Buffer_[BufPos_],
										BufferCurrentSize_-BufPos_);
					BufPos_ = BufferCurrentSize_;
				} else {
					good = SendToClient(&Buffer_[BufPos_], waiting_for_bytes_);
					BufPos_ += waiting_for_bytes_;
					waiting_for_bytes_ = 0;
				}
			} else {
				BufPos_++;
				msg_len -= 1;
				if (BufferCurrentSize_-BufPos_ < msg_len) {
					good =
						SendToClient(&Buffer_[BufPos_], BufferCurrentSize_-BufPos_);
					waiting_for_bytes_ = msg_len - (BufferCurrentSize_-BufPos_);
					BufPos_ = BufferCurrentSize_;
				} else {
					waiting_for_bytes_ = 0;
					good = SendToClient(&Buffer_[BufPos_], BufferCurrentSize_-BufPos_);
					BufPos_ = BufferCurrentSize_;
				}
			}
		} catch (const Poco::Exception &E) {
			Logger_.log(E);
			return false;
		} catch (const std::exception &E) {
			RTTYS_server()->LogStdException(E, "Cannot send data to UI Client");
			return false;
		}
		return good;
	}

	bool RTTYS_EndPoint::SendToClient(const u_char *Buf, int len) {
		WSSocket_->sendFrame(
			Buf, len, Poco::Net::WebSocket::FRAME_FLAG_FIN | Poco::Net::WebSocket::FRAME_OP_BINARY);
		return true;
	}

	bool RTTYS_EndPoint::SendToClient(const std::string &s) {
		WSSocket_->sendFrame(s.c_str(), s.length());
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeWinsize([[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger_, "Asking for msgTypeWinsize");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeCmd([[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger_, "Asking for msgTypeCmd");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeHeartbeat([[maybe_unused]] std::size_t msg_len) {
		if (DeviceSocket_ != nullptr) {
			try {
				u_char MsgBuf[RTTY_HDR_SIZE + 16]{0};
				BufPos_ += msg_len;
				MsgBuf[0] = msgTypeHeartbeat;
				MsgBuf[1] = 0;
				MsgBuf[2] = 0;
				auto Sent = send_ssl_bytes(MsgBuf, RTTY_HDR_SIZE);
				return Sent == RTTY_HDR_SIZE;
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
				return false;
			} catch (const std::exception &E) {
				RTTYS_server()->LogStdException(E, "Cannot send heartbeat");
				return false;
			}
		}
		return false;
	}

	bool RTTYS_EndPoint::do_msgTypeFile([[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger_, "Asking for msgTypeFile");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeHttp([[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger_, "Asking for msgTypeHttp");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeAck([[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger_, "Asking for msgTypeAck");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeMax([[maybe_unused]] std::size_t msg_len) {
		poco_debug(Logger_, "Asking for msgTypeMax");
		return true;
	}


	RTTYS_EndPoint::RTTYS_EndPoint(const std::string &Id, const std::string &Token,
								   const std::string &SerialNumber, const std::string &UserName,
								   bool mTLS,
								   Poco::Logger &Logger)
		: Id_(Id), Token_(Token), SerialNumber_(SerialNumber), UserName_(UserName),
	  	Logger_(Logger), mTLS_(mTLS) {
		Created_ = std::chrono::high_resolution_clock::now();
		// DeviceInBuf_ = std::make_shared<Poco::FIFOBuffer>(RTTY_DEVICE_BUFSIZE);
	}

	RTTYS_EndPoint::RTTYS_EndPoint(Poco::Net::StreamSocket &Socket, std::uint64_t tid,
								   Poco::Logger &Logger) :
		Logger_(Logger)
	{
		DeviceSocket_ = std::make_unique<Poco::Net::StreamSocket>(Socket);
		// DeviceInBuf_ = std::make_shared<Poco::FIFOBuffer>(RTTY_DEVICE_BUFSIZE);
		TID_ = tid;
	}

	RTTYS_EndPoint::~RTTYS_EndPoint() {
	}

} // namespace OpenWifi