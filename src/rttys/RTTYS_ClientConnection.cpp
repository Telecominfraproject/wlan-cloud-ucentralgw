//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_ClientConnection.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_server.h"

#include "fmt/format.h"

#include "nlohmann/json.hpp"

namespace OpenWifi {

	RTTYS_ClientConnection::RTTYS_ClientConnection(
		Poco::Net::HTTPServerRequest &request,
		Poco::Net::HTTPServerResponse &response,
		Poco::Net::SocketReactor &reactor,
		const std::string &Id)
		:
	  		Reactor_(reactor),
		  	Id_(Id),
	  		Logger_(Poco::Logger::get(fmt::format("RTTY-client({})",Id_)))
		{
			Valid_ = true;
			WS_ = std::make_unique<Poco::Net::WebSocket>(request,response);
			WS_->setBlocking(false);
			WS_->setNoDelay(true);
			WS_->setKeepAlive(true);
			Registered_ = true;
			Reactor_.addEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
						  *this, &RTTYS_ClientConnection::onSocketReadable));
			Reactor_.addEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
						  *this, &RTTYS_ClientConnection::onSocketShutdown));
			Logger_.information("Starting connection");
		}

	RTTYS_ClientConnection::~RTTYS_ClientConnection() {
		std::cout << "Deleting RTTY CkientConnection" << std::endl;
		poco_information(Logger_,
				   fmt::format("Client {} session ending", Id_)
				   );
		DeRegister();
	}

	void RTTYS_ClientConnection::DeRegister() {
		Valid_ = false;
		if(Registered_) {
			Registered_ = false;
			Reactor_.removeEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
						  *this, &RTTYS_ClientConnection::onSocketReadable));
			Reactor_.removeEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
						  *this, &RTTYS_ClientConnection::onSocketShutdown));
		}
	}

	void RTTYS_ClientConnection::EndConnection() {
		DeRegister();
		RTTYS_server()->NotifyClientDisconnect(Id_);
	}

	void RTTYS_ClientConnection::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		bool MustDisconnect = false;

		{
			if(!Valid_ || !Registered_)
				return;
			std::lock_guard G(Mutex_);
			try {
				int flags;
				unsigned char FrameBuffer[1024];
				auto ReceivedBytes = WS_->receiveFrame(FrameBuffer, sizeof(FrameBuffer), flags);
				auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;
				switch (Op) {
				case Poco::Net::WebSocket::FRAME_OP_PING: {
					WS_->sendFrame("", 0,
								   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
									   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
				} break;
				case Poco::Net::WebSocket::FRAME_OP_PONG: {
				} break;
				case Poco::Net::WebSocket::FRAME_OP_TEXT: {
					if (ReceivedBytes == 0) {
						poco_trace(Logger_,"Client closing connection.");
						MustDisconnect = true;
					} else {
						if(Device_!= nullptr) {
							std::string Frame((const char *)FrameBuffer, ReceivedBytes);
							try {
								auto Doc = nlohmann::json::parse(Frame);
								if (Doc.contains("type")) {
									auto Type = Doc["type"];
									if (Type == "winsize") {
										auto cols = Doc["cols"];
										auto rows = Doc["rows"];
										if (!Device_->WindowSize(cols, rows)) {
											Logger_.information("Winsize shutdown.");
											MustDisconnect = true;
										}
									}
								}
							} catch (...) {
								// just ignore parse errors
								Logger_.information("Frame text exception shutdown.");
								MustDisconnect = true;
							}
						}
					}
				} break;
				case Poco::Net::WebSocket::FRAME_OP_BINARY: {
					if (ReceivedBytes == 0) {
						poco_trace(Logger_,"Client closing connection.");
						MustDisconnect = true;
					} else {
						poco_trace(Logger_, fmt::format("Sending {} key strokes to device.", ReceivedBytes));
						if(Device_!= nullptr) {
							if (!Device_->KeyStrokes(FrameBuffer, ReceivedBytes)) {
								poco_trace(Logger_,"Cannot send keys to device. Close connection.");
								MustDisconnect = true;
							}
						}
					}
				} break;
				case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
					poco_trace(Logger_,"Frame close shutdown.");
					MustDisconnect = true;
				} break;

				default: {
				}
				}
			} catch (...) {
				poco_error(Logger_,"Frame readable shutdown.");
				MustDisconnect = true;
			}
		}

		if(MustDisconnect) {
			EndConnection();
		}
	}

	void RTTYS_ClientConnection::SendData( const u_char *Buf, size_t len ) {
		if(!Valid_ || !Registered_)
			return;
		try {
			WS_->sendFrame(Buf, len,
						   Poco::Net::WebSocket::FRAME_FLAG_FIN |
							   Poco::Net::WebSocket::FRAME_OP_BINARY);
			return;
		} catch (...) {
			Logger_.information("SendData shutdown.");
		}
		EndConnection();
	}

	void RTTYS_ClientConnection::SendData( const std::string &s) {
		if(!Valid_ || !Registered_)
			return;
		try {
			WS_->sendFrame(s.c_str(), s.length());
			return;
		} catch (...) {
			Logger_.information("SendData shutdown.");
		}
		EndConnection();
	}

	void RTTYS_ClientConnection::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		Logger_.information("Socket shutdown.");
		EndConnection();
	}

}