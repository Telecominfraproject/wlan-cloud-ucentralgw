//
// Created by stephane bourque on 2021-11-23.
//

#include "rttys/RTTYS_server.h"
#include "rttys/RTTYS_WebServer.h"

#include "framework/MicroServiceFuncs.h"
#include "fmt/format.h"

#include "nlohmann/json.hpp"
#include "Poco/Net/SecureStreamSocketImpl.h"

#include "Poco/Net/SocketNotification.h"
#include "Poco/NObserver.h"

#define DBGLINE		{	std::cout << __LINE__ << std::endl; }

namespace OpenWifi {

	int RTTYS_server::Start() {

		Internal_ = MicroServiceConfigGetBool("rtty.internal",false);
		if(Internal_) {
			int DSport = (int) MicroServiceConfigGetInt("rtty.port", 5912);
			int CSport = (int) MicroServiceConfigGetInt("rtty.viewport", 5913);
			RTTY_UIAssets_ = MicroServiceConfigPath("rtty.assets", "$OWGW_ROOT/rtty_ui");
			MaxConcurrentSessions_ = MicroServiceConfigGetInt("rtty.maxsessions",0);

			const auto & CertFileName = MicroServiceConfigPath("openwifi.restapi.host.0.cert","");
			const auto & KeyFileName = MicroServiceConfigPath("openwifi.restapi.host.0.key","");
			const auto & RootCa = MicroServiceConfigPath("openwifi.restapi.host.0.rootca","");

			NoSecurity_ = MicroServiceNoAPISecurity();

			if(NoSecurity_) {
				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, DSport);
				DeviceSocket_ = std::make_unique<Poco::Net::ServerSocket>(SockAddr, 64);
				Reactor_.addEventHandler(*DeviceSocket_ ,
											   Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>
											   (*this, &RTTYS_server::onDeviceAccept));
			} else {
				auto DeviceSecureContext = Poco::AutoPtr<Poco::Net::Context>( new Poco::Net::Context(Poco::Net::Context::SERVER_USE,
																  KeyFileName, CertFileName, "",
																  Poco::Net::Context::VERIFY_RELAXED));
				Poco::Crypto::X509Certificate DeviceRoot(RootCa);
				DeviceSecureContext->addCertificateAuthority(DeviceRoot);
				DeviceSecureContext->disableStatelessSessionResumption();
				DeviceSecureContext->enableSessionCache();
				DeviceSecureContext->setSessionCacheSize(0);
				DeviceSecureContext->setSessionTimeout(10);
				DeviceSecureContext->enableExtendedCertificateVerification(true);
				SSL_CTX *SSLCtxDevice = DeviceSecureContext->sslContext();
				SSL_CTX_dane_enable(SSLCtxDevice);

				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, DSport);

				SecureDeviceSocket_ = std::make_unique<Poco::Net::SecureServerSocket>(SockAddr, 64, DeviceSecureContext);
				Reactor_.addEventHandler(*SecureDeviceSocket_ ,
					Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>
					(*this, &RTTYS_server::onDeviceAccept));
			}

			ReactorThread_.start(Reactor_);
			Utils::SetThreadName(ReactorThread_,"rt:devreactor");

			auto WebServerHttpParams = new Poco::Net::HTTPServerParams;
			WebServerHttpParams->setMaxThreads(50);
			WebServerHttpParams->setMaxQueued(200);
			WebServerHttpParams->setKeepAlive(true);
			WebServerHttpParams->setName("rt:dispatch");

			if(NoSecurity_) {
				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, CSport);
				Poco::Net::ServerSocket ClientSocket(SockAddr, 64);
				ClientSocket.setNoDelay(true);
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(new RTTYS_Client_RequestHandlerFactory(Logger()), ClientSocket, WebServerHttpParams);
			} else {
				auto WebClientSecureContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE, KeyFileName, CertFileName,
										   "", Poco::Net::Context::VERIFY_RELAXED);
				Poco::Crypto::X509Certificate WebRoot(RootCa);
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
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(new RTTYS_Client_RequestHandlerFactory(Logger()), ClientSocket, WebServerHttpParams);
			};
			WebServer_->start();
		}

		GCCallBack_ = std::make_unique<Poco::TimerCallback<RTTYS_server>>(*this, &RTTYS_server::onTimer);
		Timer_.setStartInterval(30 * 1000);  // first run in 30 seconds
		Timer_.setPeriodicInterval(5 * 1000);
		Timer_.start(*GCCallBack_, MicroServiceTimerPool() );
		Running_ = true;
		return 0;
	}

	void RTTYS_server::Stop() {
		Timer_.stop();
		Running_ = false;
		if(Internal_) {
			WebServer_->stopAll(true);
			WebServer_->stop();
			Reactor_.removeEventHandler(NoSecurity_ ? *DeviceSocket_ : *SecureDeviceSocket_, Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>
											  (*this, &RTTYS_server::onDeviceAccept));
			Reactor_.stop();
			ReactorThread_.join();
		}
	}

	void RTTYS_server::CloseDevice(std::shared_ptr<RTTYS_EndPoint> Device) {
		if(Device->DeviceSocket_!= nullptr) {
			Device->DeviceDisconnected_ = std::chrono::high_resolution_clock::now();
			TotalConnectedDeviceTime_ += Device->DeviceDisconnected_ - Device->DeviceConnected_;
			RemoveDeviceEventHandlers(*Device->DeviceSocket_);
			Device->DeviceSocket_.reset();
		} else {
			std::cout << "CloseDevice NULL" << std::endl;
		}
	}

	void RTTYS_server::CloseClient(std::shared_ptr<RTTYS_EndPoint> Client) {
		if(Client->WSSocket_!= nullptr) {
			Client->ClientDisconnected_ = std::chrono::high_resolution_clock::now();
			TotalConnectedClientTime_ += Client->ClientDisconnected_ - Client->ClientConnected_;
			RemoveClientEventHandlers(*Client->WSSocket_);
			Client->WSSocket_.reset();
		} else {
			std::cout << "CloseClient NULL" << std::endl;
		}
	}

	void RTTYS_server::onDeviceAccept(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		std::lock_guard	Guard(ServerMutex_);

		try {
			Poco::Net::SocketAddress Client;
			Poco::Net::StreamSocket NewSocket = pNf->socket().impl()->acceptConnection(Client);
			if(NewSocket.impl()->secure()) {
				auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(NewSocket.impl());
				while (true) {
					auto V = SS->completeHandshake();
					if (V == 1)
						break;
				}
			}
			AddConnectingDeviceEventHandlers(NewSocket);
		} catch (const Poco::Exception &E) {
			std::cout << "Exception onDeviceAccept: " << E.what() << std::endl;
			Logger().log(E);
		}
	}

	void RTTYS_server::RemoveConnectingDeviceEventHandlers(Poco::Net::StreamSocket &Socket) {
		if(ConnectingDevices_.find(Socket.impl()->sockfd())==ConnectingDevices_.end()) {
			std::cout << "Could not find connecting device socket" << std::endl;
		}
		ConnectingDevices_.erase(Socket.impl()->sockfd());
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
										*this, &RTTYS_server::onConnectingDeviceData));
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
										*this, &RTTYS_server::onConnectingDeviceShutdown));
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
										*this, &RTTYS_server::onConnectingDeviceError));
	}

	void RTTYS_server::RemoveClientEventHandlers(Poco::Net::StreamSocket &Socket) {
		if(Connections_.find(Socket.impl()->sockfd())==Connections_.end()) {
			std::cout << "Could not find client socket" << std::endl;
		}
		Connections_.erase(Socket.impl()->sockfd());
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
										*this, &RTTYS_server::onClientSocketReadable));
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
										*this, &RTTYS_server::onClientSocketShutdown));
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
										*this, &RTTYS_server::onClientSocketError));
	}

	void RTTYS_server::RemoveDeviceEventHandlers(Poco::Net::StreamSocket &Socket) {
		if(Connections_.find(Socket.impl()->sockfd())==Connections_.end()) {
			std::cout << "Could not find device socket" << std::endl;
		}
		Connections_.erase(Socket.impl()->sockfd());
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
										*this, &RTTYS_server::onDeviceSocketReadable));
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
										*this, &RTTYS_server::onDeviceSocketShutdown));
		Reactor_.removeEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
										*this, &RTTYS_server::onDeviceSocketError));
	}

	void RTTYS_server::AddConnectingDeviceEventHandlers(Poco::Net::StreamSocket &Socket) {
		if(ConnectingDevices_.find(Socket.impl()->sockfd())!=ConnectingDevices_.end()) {
			std::cout << "Connecting socket already exists" << std::endl;
		}
		ConnectingDevices_[ Socket.impl()->sockfd() ] = std::make_pair(Socket,std::chrono::high_resolution_clock::now());
		Reactor_.addEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
										*this, &RTTYS_server::onConnectingDeviceData));
		Reactor_.addEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
										*this, &RTTYS_server::onConnectingDeviceShutdown));
		Reactor_.addEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
										*this, &RTTYS_server::onConnectingDeviceError));
	}

	void RTTYS_server::AddClientEventHandlers(Poco::Net::StreamSocket &Socket, std::shared_ptr<RTTYS_EndPoint> EndPoint) {
		if(Connections_.find(Socket.impl()->sockfd())!=Connections_.end()) {
			std::cout << "Client socket already exists" << std::endl;
		}
		Connections_[ Socket.impl()->sockfd() ] = EndPoint;
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

	void RTTYS_server::AddDeviceEventHandlers(Poco::Net::StreamSocket &Socket, std::shared_ptr<RTTYS_EndPoint> EndPoint) {
		if(Connections_.find(Socket.impl()->sockfd())!=Connections_.end()) {
			std::cout << "Device socket already exists" << std::endl;
		}
		Connections_[Socket.impl()->sockfd()] = EndPoint;
		Reactor_.addEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>(
										*this, &RTTYS_server::onDeviceSocketReadable));
		Reactor_.addEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ShutdownNotification>(
										*this, &RTTYS_server::onDeviceSocketShutdown));
		Reactor_.addEventHandler(Socket,
									Poco::NObserver<RTTYS_server, Poco::Net::ErrorNotification>(
										*this, &RTTYS_server::onDeviceSocketError));
	}

	void RTTYS_server::onConnectingDeviceData(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		std::lock_guard	Guard(ServerMutex_);

		auto ConnectingDevice = ConnectingDevices_.end();

		try {
			ConnectingDevice = ConnectingDevices_.find(pNf->socket().impl()->sockfd());
			if (ConnectingDevice == end(ConnectingDevices_)) {
				std::cout << "Cannot find connecting socket." << std::endl;
				poco_warning(Logger(), "Cannot find connecting socket.");
				return;
			}

			//	We are waiting for this device to register, so we can only accept regitration and hertbeat
			unsigned char Buffer[1024];
			auto ReceivedBytes =
				ConnectingDevice->second.first.receiveBytes(Buffer, sizeof(Buffer));
			if (ReceivedBytes == 0) {
				std::cout << "Connecting socket closing." << std::endl;
				return RemoveConnectingDeviceEventHandlers(ConnectingDevice->second.first);
			}

			//	Process the command
			bool good = true;
			switch (Buffer[0]) {
			case RTTYS_EndPoint::msgTypeRegister: {
				good = do_msgTypeRegister(ConnectingDevice->second.first, Buffer, ReceivedBytes);
			} break;
			case RTTYS_EndPoint::msgTypeHeartbeat: {
				good = do_msgTypeHeartbeat(ConnectingDevice->second.first);
			} break;
			default: {
				poco_warning(Logger(), "Device violated protocol");
				good = false;
			}
			}

			if (!good) {
				return RemoveConnectingDeviceEventHandlers(ConnectingDevice->second.first);
			}
		} catch (const Poco::Exception &E) {
			std::cout << "Poco::Exception connecting socket: " << E.what() << std::endl;
			if(ConnectingDevice!=ConnectingDevices_.end()) {
				RemoveConnectingDeviceEventHandlers(ConnectingDevice->second.first);
			}

		} catch (...) {
			std::cout << "std::exception connecting socket." << std::endl;
			if(ConnectingDevice!=ConnectingDevices_.end()) {
				RemoveConnectingDeviceEventHandlers(ConnectingDevice->second.first);
			}
		}
	}

	bool RTTYS_server::do_msgTypeRegister(Poco::Net::StreamSocket &Socket, unsigned char *Buffer, int Len) {
		bool good = true;
		try {
			//	establish if this is an old rtty or a new one.
			bool old_rtty_ = (Buffer[0] != 0x03);		//	rtty_proto_ver for full session ID inclusion
			int pos=3;
			int session_length_=0;
			if(old_rtty_) {
				session_length_ = 1;
			} else {
				pos++;
				session_length_ = RTTY_SESSION_ID_LENGTH;
			}

			std::string id_ = ReadString(Buffer,Len,pos);
			std::string desc_ = ReadString(Buffer,Len,pos);
			std::string token_ = ReadString(Buffer,Len,pos);

			if(id_.size()!=RTTY_DEVICE_TOKEN_LENGTH || token_.size()!=RTTY_DEVICE_TOKEN_LENGTH || desc_.empty()) {
				return false;
			}

			poco_information(Logger(), fmt::format("Description:{} Device registration", desc_));
			//	find this device in our connectio end points...
			auto Connection = FindConnection(id_,token_);
			if(Connection==nullptr) {
				//	bad connection
				return false;
			}

			u_char OutBuf[8];
			OutBuf[0] = RTTYS_EndPoint::msgTypeRegister;
			OutBuf[1] = 0;		//	Data length
			OutBuf[2] = 4;		//
			OutBuf[3] = 0;		//	Error
			OutBuf[4] = 'O';
			OutBuf[5] = 'K';
			OutBuf[6] = 0;
			if (Socket.sendBytes(OutBuf, 7) != 7) {
				poco_information(Logger(),
								 fmt::format("{}: Description:{} Could not send data to complete registration",
											 id_, desc_));
				return false;
			}
			RemoveConnectingDeviceEventHandlers(Socket);
			Connection->DeviceSocket_ = std::make_unique<Poco::Net::StreamSocket>(Socket);
			Connection->DeviceConnected_ = std::chrono::high_resolution_clock::now();
			Connection->old_rtty_ = old_rtty_;
			Connection->session_length_ = session_length_;
			AddDeviceEventHandlers(*Connection->DeviceSocket_, Connection);

			if(Connection->WSSocket_!= nullptr) {
				Connection->Login();
			}

			return true;
		} catch (...) {
			good = false;
		}
		return good;
	}

	bool RTTYS_server::do_msgTypeHeartbeat(Poco::Net::StreamSocket &Socket) {
		u_char MsgBuf[RTTY_HDR_SIZE + 16]{0};
		MsgBuf[0] = RTTYS_EndPoint::msgTypeHeartbeat;
		MsgBuf[1] = 0;
		MsgBuf[2] = 0;
		auto Sent = Socket.sendBytes(MsgBuf, RTTY_HDR_SIZE);
		return Sent == RTTY_HDR_SIZE;
	}

	void RTTYS_server::onConnectingDeviceShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		std::lock_guard	Guard(ServerMutex_);
		auto ConnectingDevice = ConnectingDevices_.find(pNf->socket().impl()->sockfd());
		if(ConnectingDevice==end(ConnectingDevices_)) {
			poco_warning(Logger(), "Cannot find connecting socket.");
			return;
		}
		RemoveConnectingDeviceEventHandlers(ConnectingDevice->second.first);
	}

	void RTTYS_server::onConnectingDeviceError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
		std::lock_guard	Guard(ServerMutex_);
		auto ConnectingDevice = ConnectingDevices_.find(pNf->socket().impl()->sockfd());
		if(ConnectingDevice==end(ConnectingDevices_)) {
			poco_warning(Logger(), "Cannot find connecting socket.");
			return;
		}
		RemoveConnectingDeviceEventHandlers(ConnectingDevice->second.first);
	}

	void RTTYS_server::onDeviceSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
		std::lock_guard	Guard(ServerMutex_);

		auto Connection = Connections_.end();
		std::shared_ptr<RTTYS_EndPoint>		EndPoint;

		try {
			Connection = Connections_.find(pNf->socket().impl()->sockfd());
			if (Connection == end(Connections_)) {
				std::cout << "Unknown client socket" << std::endl;
				poco_warning(Logger(), fmt::format("Cannot find device socket: {}",
												   pNf->socket().impl()->sockfd()));
				return;
			}

			EndPoint = Connection->second;
			if (EndPoint== nullptr || EndPoint->DeviceSocket_ == nullptr) {
				std::cout << "Null device " << std::endl;
				return;
			}

			bool good = true;

			auto received_bytes = EndPoint->DeviceSocket_->receiveBytes(*EndPoint->DeviceInBuf_);
			if (received_bytes == 0) {
				good = false;
				poco_information(Logger(), "Device Closing connection - 0 bytes received.");
			} else {
				while (EndPoint->DeviceInBuf_->isReadable() && good) {
					uint32_t msg_len = 0;
					if (EndPoint->waiting_for_bytes_ != 0) {

					} else {
						if (EndPoint->DeviceInBuf_->used() >= RTTY_HDR_SIZE) {
							auto *head = (unsigned char *)EndPoint->DeviceInBuf_->begin();
							EndPoint->last_command_ = head[0];
							msg_len = head[1] * 256 + head[2];
							EndPoint->DeviceInBuf_->drain(RTTY_HDR_SIZE);
						} else {
							good = false;
							continue;
						}
					}

					switch (EndPoint->last_command_) {
					case RTTYS_EndPoint::msgTypeLogin: {
						good = EndPoint->do_msgTypeLogin(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeLogout: {
						good = EndPoint->do_msgTypeLogout(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeTermData: {
						good = EndPoint->do_msgTypeTermData(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeWinsize: {
						good = EndPoint->do_msgTypeWinsize(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeCmd: {
						good = EndPoint->do_msgTypeCmd(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeHeartbeat: {
						good = EndPoint->do_msgTypeHeartbeat(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeFile: {
						good = EndPoint->do_msgTypeFile(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeHttp: {
						good = EndPoint->do_msgTypeHttp(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeAck: {
						good = EndPoint->do_msgTypeAck(msg_len);
					} break;
					case RTTYS_EndPoint::msgTypeMax: {
						good = EndPoint->do_msgTypeMax(msg_len);
					} break;
					default: {
						poco_warning(Logger(),
									 fmt::format("Unknown command {}. GW closing connection.",
												 (int)EndPoint->last_command_));
						good = false;
					}
					}
				}
			}

			if (!good)
				CloseConnection(EndPoint);
		} catch (const Poco::Exception &E) {
			std::cout << "Device Poco E:" << E.what() << std::endl;
			Logger().log(E);
			if(Connection!=Connections_.end() && EndPoint!= nullptr) {
				CloseConnection(EndPoint);
			}
		} catch (...) {
			std::cout << "Device Std::E" << std::endl;
			if(Connection!=Connections_.end() && EndPoint!= nullptr) {
				CloseConnection(EndPoint);
			}
		}
	}

	void RTTYS_server::onDeviceSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		std::lock_guard	Guard(ServerMutex_);

		auto Device = Connections_.find(pNf->socket().impl()->sockfd());
		if( Device == end(Connections_)) {
			poco_warning(Logger(),fmt::format("Cannot find device socket: {}", pNf->socket().impl()->sockfd()));
			return;
		}
		CloseConnection(Device->second);
	}

	void RTTYS_server::onDeviceSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
		std::lock_guard	Guard(ServerMutex_);

		auto Device = Connections_.find(pNf->socket().impl()->sockfd());
		if( Device == end(Connections_)) {
			poco_warning(Logger(),fmt::format("Cannot find device socket: {}", pNf->socket().impl()->sockfd()));
			return;
		}
		CloseConnection(Device->second);
	}

	void RTTYS_server::CloseConnection(std::shared_ptr<RTTYS_EndPoint> Connection) {
		if(Connection!=nullptr) {
			CloseDevice(Connection);
			CloseClient(Connection);
			EndPoints_.erase(Connection->Id_);
		} else {
			std::cout << "Close connection error NULL" << std::endl;
		}
	}

	void RTTYS_server::onClientSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {

		std::lock_guard	Guard(ServerMutex_);

		auto Connection = Connections_.end();
		std::shared_ptr<RTTYS_EndPoint>		EndPoint;
		try {
			Connection = Connections_.find(pNf->socket().impl()->sockfd());
			if( Connection == end(Connections_)) {
				std::cout << "Unknown client socket" << std::endl;
				poco_warning(Logger(),fmt::format("Cannot find client socket: {}", pNf->socket().impl()->sockfd()));
				return;
			}

			EndPoint = Connection->second;
			if(EndPoint == nullptr ) {
				std::cout << "NULL EndPoint Client" << std::endl;
				return;
			}

			if(EndPoint->WSSocket_ == nullptr) {
				std::cout << "NULL WS Client" << std::endl;
				return;
			}

			int flags;
			unsigned char FrameBuffer[1024];

			auto ReceivedBytes = EndPoint->WSSocket_->receiveFrame(FrameBuffer, sizeof(FrameBuffer), flags);
			auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;
			switch (Op) {

			case Poco::Net::WebSocket::FRAME_OP_PING: {
				EndPoint->WSSocket_->sendFrame("", 0,
							   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
								   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
			} break;
			case Poco::Net::WebSocket::FRAME_OP_PONG: {
			} break;
			case Poco::Net::WebSocket::FRAME_OP_TEXT: {
				if (ReceivedBytes == 0) {
					poco_trace(Logger(),"Client closing connection.");
					return CloseConnection(EndPoint);
				} else {
					if(EndPoint->DeviceSocket_!=nullptr) {
						std::string Frame((const char *)FrameBuffer, ReceivedBytes);
						try {
							auto Doc = nlohmann::json::parse(Frame);
							if (Doc.contains("type")) {
								auto Type = Doc["type"];
								if (Type == "winsize") {
									auto cols = Doc["cols"];
									auto rows = Doc["rows"];
									if (!EndPoint->WindowSize(cols, rows)) {
										poco_information(Logger(),"Winsize shutdown.");
										return CloseConnection(EndPoint);
									}
								}
							}
						} catch (...) {
							// just ignore parse errors
							poco_information(Logger(),"Frame text exception shutdown.");
							return CloseConnection(EndPoint);
						}
					}
				}
			} break;
			case Poco::Net::WebSocket::FRAME_OP_BINARY: {
				if (ReceivedBytes == 0) {
					poco_trace(Logger(),"Client closing connection.");
					return CloseConnection(EndPoint);
				} else {
					poco_trace(Logger(), fmt::format("Sending {} key strokes to device.", ReceivedBytes));
					if(EndPoint->DeviceSocket_!= nullptr) {
						if (!EndPoint->KeyStrokes(FrameBuffer, ReceivedBytes)) {
							poco_trace(Logger(),"Cannot send keys to device. Close connection.");
							return CloseConnection(EndPoint);
						}
					}
				}
			} break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
				poco_trace(Logger(),"Frame close shutdown.");
				return CloseConnection(EndPoint);
			} break;

			default: {
			}
			}
		} catch (...) {
			poco_error(Logger(),"Frame readable shutdown.");
			std::cout << "Exception in WS Client read" << std::endl;
			if(Connection!=Connections_.end() && EndPoint!=nullptr) {
				CloseConnection(EndPoint);
			}
			return;
		}
	}

	void RTTYS_server::onClientSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		std::lock_guard	Guard(ServerMutex_);
		auto Client = Connections_.find(pNf->socket().impl()->sockfd());
		if( Client == end(Connections_)) {
			poco_warning(Logger(),fmt::format("Cannot find client socket: {}", pNf->socket().impl()->sockfd()));
			return;
		}
		return CloseConnection(Client->second);
	}

	void RTTYS_server::onClientSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
		std::lock_guard	Guard(ServerMutex_);
		auto Client = Connections_.find(pNf->socket().impl()->sockfd());
		if( Client == end(Connections_)) {
			poco_warning(Logger(),fmt::format("Cannot find client socket: {}", pNf->socket().impl()->sockfd()));
			return;
		}
		return CloseConnection(Client->second);
	}

	void RTTYS_server::SendData(std::shared_ptr<RTTYS_EndPoint> & Connection, const u_char *Buf, size_t len ) {
		if(Connection->WSSocket_!= nullptr) {
			try {
				Connection->WSSocket_->sendFrame(Buf, len,
							   Poco::Net::WebSocket::FRAME_FLAG_FIN |
								   Poco::Net::WebSocket::FRAME_OP_BINARY);
				return;
			} catch (...) {
				poco_information(Logger(), "SendData shutdown.");
			}
		}
		CloseConnection(Connection);
	}

	void RTTYS_server::SendData(std::shared_ptr<RTTYS_EndPoint> & Connection, const std::string &s) {
		if(Connection->WSSocket_!= nullptr) {
			try {
				Connection->WSSocket_->sendFrame(s.c_str(), s.length());
				return;
			} catch (...) {
				poco_information(Logger(), "SendData shutdown.");
			}
		}
		CloseConnection(Connection);
	}

	void RTTYS_server::CreateWSClient(
		Poco::Net::HTTPServerRequest &request,
		Poco::Net::HTTPServerResponse &response,
		const std::string &Id) {

		std::lock_guard		Guard(ServerMutex_);

		auto EndPoint = EndPoints_.find(Id);
		if(EndPoint==end(EndPoints_)) {
			std::cout << "Invalid WS session" << std::endl;
			poco_warning(Logger(),fmt::format("Session {} is invalid."));
			return;
		}

		//	OK Create and register this WS client
		try {
			EndPoint->second->WSSocket_ = std::make_unique<Poco::Net::WebSocket>(request,response);
			EndPoint->second->ClientConnected_ = std::chrono::high_resolution_clock::now();
			EndPoint->second->WSSocket_->setBlocking(false);
			EndPoint->second->WSSocket_->setNoDelay(true);
			EndPoint->second->WSSocket_->setKeepAlive(true);
			AddClientEventHandlers(*EndPoint->second->WSSocket_, EndPoint->second);
			if(EndPoint->second->DeviceSocket_!= nullptr && !EndPoint->second->completed_) {
				EndPoint->second->Login();
			}
		} catch (const Poco::Exception &E) {
			std::cout << "Could not create WS session: Poco::Exception: " << E.what() << std::endl;
		} catch (...) {
			std::cout << "Could not create WS session: std::Exception: " << std::endl;
		}
	}

	void RTTYS_server::onTimer([[maybe_unused]] Poco::Timer & timer) {
		poco_trace(Logger(),"Removing stale connections.");
		Utils::SetThreadName("rt:janitor");
		static auto LastStats = Utils::Now();

		std::cout << "OnTimer: Start" << std::endl;
		std::lock_guard		Guard(ServerMutex_);
		auto Now = std::chrono::high_resolution_clock::now();
		int D=0,C=0;
		for(auto EndPoint=EndPoints_.begin();EndPoint!=EndPoints_.end();) {
			if(EndPoint->second->WSSocket_!= nullptr) C++;
			if(EndPoint->second->DeviceSocket_!= nullptr) D++;
			if((Now - EndPoint->second->Created_)>2min && !EndPoint->second->completed_) {
				std::cout << "OnTimer: Start 1" << std::endl;
				CloseDevice(EndPoint->second);
				std::cout << "OnTimer: Start 2" << std::endl;
				CloseClient(EndPoint->second);
				std::cout << "OnTimer: Start 3" << std::endl;
				EndPoint = EndPoints_.erase(EndPoint);
				std::cout << "OnTimer: Start 4" << std::endl;
			} else {
				++EndPoint;
			}
		}

		std::cout << "OnTimer: Stats   D:" << D << "   C:" << C << std::endl;

		if(Utils::Now()-LastStats>(60*1)) {
			LastStats = Utils::Now();
			Logger().information(fmt::format("Statistics: Total connections:{} Current-connections:{} Avg-Device-Connection Time: {:.2f}ms Avg-Client-Connection Time: {:.2f}ms #Sockets: {}. Connecting devices: {}",
				TotalEndPoints_,
				EndPoints_.size(),
				TotalEndPoints_ ? TotalConnectedDeviceTime_.count() / (double)TotalEndPoints_ : 0.0,
				TotalEndPoints_ ? TotalConnectedClientTime_.count() / (double)TotalEndPoints_ : 0.0,
				Connections_.size(),
				ConnectingDevices_.size() ));
		}

		std::cout << "OnTimer: End" << std::endl;
	}

	bool RTTYS_server::CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber ) {
		std::lock_guard		Guard(ServerMutex_);
		if(MaxConcurrentSessions_!=0 && EndPoints_.size()==MaxConcurrentSessions_) {
			return false;
		}
		EndPoints_[Id] = std::make_unique<RTTYS_EndPoint>(Id,Token, SerialNumber, UserName );
		++TotalEndPoints_;
		return true;
	}

	bool RTTYS_server::ValidId(const std::string &Id) {
		std::lock_guard			Guard(ServerMutex_);
		return EndPoints_.find(Id) != EndPoints_.end();
	}

	bool RTTYS_EndPoint::KeyStrokes(const u_char *buf, size_t len) {

		if(DeviceSocket_!= nullptr) {
			if (len <= (sizeof(small_buf_) - RTTY_HDR_SIZE - session_length_)) {
				small_buf_[0] = msgTypeTermData;
				small_buf_[1] = ((len - 1 + session_length_) & 0xff00) >> 8;
				small_buf_[2] = ((len - 1 + session_length_) & 0x00ff);
				memcpy(&small_buf_[RTTY_HDR_SIZE], session_id_, session_length_);
				memcpy(&small_buf_[RTTY_HDR_SIZE + session_length_], &buf[1], len - 1);
				try {
					auto Sent = DeviceSocket_->sendBytes(small_buf_,
														 RTTY_HDR_SIZE + session_length_ + len - 1);
					return (Sent == (int)(RTTY_HDR_SIZE + session_length_ + len - 1));
				} catch (...) {
					return false;
				}
			} else {
				auto Msg = std::make_unique<unsigned char[]>(len + RTTY_HDR_SIZE + session_length_);
				Msg.get()[0] = msgTypeTermData;
				Msg.get()[1] = ((len - 1 + session_length_) & 0xff00) >> 8;
				Msg.get()[2] = ((len - 1 + session_length_) & 0x00ff);
				memcpy((Msg.get() + RTTY_HDR_SIZE), session_id_, session_length_);
				memcpy((Msg.get() + RTTY_HDR_SIZE + session_length_), &buf[1], len - 1);
				try {
					auto Sent = DeviceSocket_->sendBytes(Msg.get(), RTTY_HDR_SIZE +
																		 session_length_ + len - 1);
					return (Sent == (int)(RTTY_HDR_SIZE + session_length_ + len - 1));
				} catch (...) {
					return false;
				}
			}
		}
		return false;
	}

	bool RTTYS_EndPoint::WindowSize(int cols, int rows) {
		if(DeviceSocket_!= nullptr) {
			u_char outBuf[8 + RTTY_SESSION_ID_LENGTH]{0};
			outBuf[0] = msgTypeWinsize;
			outBuf[1] = 0;
			outBuf[2] = 4 + session_length_;
			memcpy(&outBuf[RTTY_HDR_SIZE], session_id_, session_length_);
			outBuf[RTTY_HDR_SIZE + 0 + session_length_] = cols >> 8;
			outBuf[RTTY_HDR_SIZE + 1 + session_length_] = cols & 0x00ff;
			outBuf[RTTY_HDR_SIZE + 2 + session_length_] = rows >> 8;
			outBuf[RTTY_HDR_SIZE + 3 + session_length_] = rows & 0x00ff;
			try {
				auto Sent = DeviceSocket_->sendBytes(outBuf, RTTY_HDR_SIZE + 4 + session_length_);
				return (Sent == (int)(RTTY_HDR_SIZE + 4 + session_length_));
			} catch (...) {
			}
		}
		return false;
	}

	bool RTTYS_EndPoint::Login() {
		if(DeviceSocket_!= nullptr) {
			u_char outBuf[RTTY_HDR_SIZE + RTTY_SESSION_ID_LENGTH]{0};
			outBuf[0] = msgTypeLogin;
			outBuf[1] = 0;
			if (old_rtty_) {
				outBuf[2] = 0;
			} else {
				outBuf[2] = RTTY_SESSION_ID_LENGTH;
				std::strncpy(
					session_id_,
					Utils::ComputeHash(Id_, Token_).substr(0, RTTY_SESSION_ID_LENGTH / 2).c_str(),
					RTTY_SESSION_ID_LENGTH);
				memcpy(&outBuf[RTTY_HDR_SIZE], session_id_, RTTY_SESSION_ID_LENGTH);
			}
			try {
				poco_information(Logger(), "Device login");
				auto Sent = DeviceSocket_->sendBytes(outBuf, RTTY_HDR_SIZE +
														  (old_rtty_ ? 0 : RTTY_SESSION_ID_LENGTH));
				completed_ = true ;
				return Sent == (int)(RTTY_HDR_SIZE + (old_rtty_ ? 0 : RTTY_SESSION_ID_LENGTH));
			} catch (const Poco::IOException &E) {
				return false;
			} catch (const Poco::Exception &E) {
				return false;
			}
		}
		return false;
	}

	bool RTTYS_EndPoint::Logout() {
		if(DeviceSocket_!= nullptr) {
			u_char outBuf[4 + RTTY_SESSION_ID_LENGTH]{0};
			outBuf[0] = msgTypeLogout;
			outBuf[1] = 0;
			outBuf[2] = session_length_;
			memcpy(&outBuf[3], session_id_, session_length_);
			poco_information(Logger(), "{}: Logout");
			try {
				auto Sent = DeviceSocket_->sendBytes(outBuf, RTTY_HDR_SIZE + session_length_);
				return Sent == (int)(RTTY_HDR_SIZE + session_length_);
			} catch (...) {
			}
		}
		return false;
	}

	std::string RTTYS_server::ReadString(unsigned char *buffer, int len, int &pos) {
		std::string Res;
		while(pos<len) {
			char C = (char)buffer[pos++];
			if(C==0) {
				break;
			}
			Res += C;
		}
		return Res;
	}

	bool RTTYS_EndPoint::do_msgTypeLogin([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),"Asking for login");
		if(WSSocket_!=nullptr) {
			nlohmann::json doc;
			char Error;
			if (old_rtty_) {
				DeviceInBuf_->read(&Error, 1);
				DeviceInBuf_->read(&session_id_[0], session_length_);
			} else {
				char session[RTTY_SESSION_ID_LENGTH + 1]{0};
				DeviceInBuf_->read(&session[0], session_length_);
				DeviceInBuf_->read(&Error, 1);
			}
			doc["type"] = "login";
			doc["err"] = Error;
			const auto login_msg = to_string(doc);
			return SendToClient(login_msg);
		}
		return false;
	}

	bool RTTYS_EndPoint::do_msgTypeLogout([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),"Logout");
		char session[RTTY_SESSION_ID_LENGTH];
		if(old_rtty_) {
			DeviceInBuf_->read(&session[0],1);
		} else {
			DeviceInBuf_->read(&session[0],RTTY_SESSION_ID_LENGTH);
		}
		return false;
	}

	bool RTTYS_EndPoint::do_msgTypeTermData(std::size_t msg_len) {
		bool good;
		if(waiting_for_bytes_>0) {
			if(DeviceInBuf_->used()<waiting_for_bytes_) {
				waiting_for_bytes_ = waiting_for_bytes_ - DeviceInBuf_->used();
				good = SendToClient((unsigned char *)DeviceInBuf_->begin(), (int) DeviceInBuf_->used());
				DeviceInBuf_->drain();
			} else {
				good = SendToClient((unsigned char *)DeviceInBuf_->begin(), waiting_for_bytes_);
				DeviceInBuf_->drain(waiting_for_bytes_);
				waiting_for_bytes_ = 0 ;
			}
		} else {
			if(old_rtty_) {
				DeviceInBuf_->drain(1);
				msg_len -= 1;
			} else {
				DeviceInBuf_->drain(RTTY_SESSION_ID_LENGTH);
				msg_len -= RTTY_SESSION_ID_LENGTH;
			}
			if(DeviceInBuf_->used()<msg_len) {
				good = SendToClient((unsigned char *)DeviceInBuf_->begin(), DeviceInBuf_->used());
				waiting_for_bytes_ = msg_len - DeviceInBuf_->used();
				DeviceInBuf_->drain();
			} else {
				waiting_for_bytes_ = 0 ;
				good = SendToClient((unsigned char *)DeviceInBuf_->begin(), msg_len);
				DeviceInBuf_->drain(msg_len);
			}
		}
		return good;
	}

	bool RTTYS_EndPoint::SendToClient( const u_char *Buf, int len ) {
		WSSocket_->sendFrame(Buf, len,
					   Poco::Net::WebSocket::FRAME_FLAG_FIN |
						   Poco::Net::WebSocket::FRAME_OP_BINARY);
		return true;
	}

	bool RTTYS_EndPoint::SendToClient( const std::string &s) {
		WSSocket_->sendFrame(s.c_str(), s.length());
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeWinsize([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),"Asking for msgTypeWinsize");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeCmd([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),"Asking for msgTypeCmd");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeHeartbeat([[maybe_unused]] std::size_t msg_len) {
		if(DeviceSocket_!= nullptr) {
			u_char MsgBuf[RTTY_HDR_SIZE + 16]{0};
			if (msg_len)
				DeviceInBuf_->drain(msg_len);
			MsgBuf[0] = msgTypeHeartbeat;
			MsgBuf[1] = 0;
			MsgBuf[2] = 0;
			auto Sent = DeviceSocket_->sendBytes(MsgBuf, RTTY_HDR_SIZE);
			return Sent == RTTY_HDR_SIZE;
		}
		return false;
	}

	bool RTTYS_EndPoint::do_msgTypeFile([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),"Asking for msgTypeFile");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeHttp([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),"Asking for msgTypeHttp");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeAck([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),"Asking for msgTypeAck");
		return true;
	}

	bool RTTYS_EndPoint::do_msgTypeMax([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),"Asking for msgTypeMax");
		return true;
	}

	RTTYS_EndPoint::RTTYS_EndPoint(const std::string &Id,const std::string &Token, const std::string &SerialNumber, const std::string &UserName ):
 		Id_(Id),
		Token_(Token),
		SerialNumber_(SerialNumber),
		UserName_(UserName),
		Logger_(RTTYS_server()->Logger())
	{
		DeviceInBuf_ = std::make_unique<Poco::FIFOBuffer>(RTTY_DEVICE_BUFSIZE);
		Created_ = std::chrono::high_resolution_clock::now();
	}

	RTTYS_EndPoint::~RTTYS_EndPoint() {
		poco_information(Logger(),fmt::format("{}: Connection ending.", SerialNumber_));

		if(DeviceSocket_!= nullptr)
			std::cout << "Delete device with valid socket" << std::endl;
		if(WSSocket_!= nullptr)
			std::cout << "Client with valid socket" << std::endl;

	}

}