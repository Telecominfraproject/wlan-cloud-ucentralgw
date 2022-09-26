//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_ClientConnection.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_server.h"

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
			Logger_.information("Starting connection");
			Valid_ = true;
			WS_ = std::make_unique<Poco::Net::WebSocket>(request,response);
			WS_->setBlocking(false);
			WS_->setNoDelay(true);
			WS_->setKeepAlive(true);
			Reactor_.addEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
						  *this, &RTTYS_ClientConnection::onSocketReadable));
			Reactor_.addEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
						  *this, &RTTYS_ClientConnection::onSocketShutdown));
		}

	RTTYS_ClientConnection::~RTTYS_ClientConnection() {
		if(Valid_) {
			std::unique_lock G(Mutex_);
			EndConnection(false);
		}
	}

	void RTTYS_ClientConnection::EndConnection(bool SendNotification) {
		if(Valid_) {
			Valid_=false;
			try {
				Reactor_.removeEventHandler(
					*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
							  *this, &RTTYS_ClientConnection::onSocketReadable));
				Reactor_.removeEventHandler(
					*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
							  *this, &RTTYS_ClientConnection::onSocketShutdown));
				if (SendNotification)
					RTTYS_server()->NotifyClientDisconnect(Id_, this);
			} catch(...) {

			}
			Logger_.information("Disconnected.");

		}
	}

	void RTTYS_ClientConnection::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		bool MustDisconnect = false;

		{
			std::shared_lock G(Mutex_);
			try {
				int flags;
				Poco::Buffer<char> IncomingFrame(0);
				auto n = WS_->receiveFrame(IncomingFrame, flags);
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
					if (n == 0) {
						Logger_.information("Socket readable shutdown.");
						MustDisconnect = true;
					} else {
						std::string s((char *)IncomingFrame.begin(), IncomingFrame.size());
						try {
							auto Doc = nlohmann::json::parse(s);
							if (Doc.contains("type")) {
								auto Type = Doc["type"];
								if (Type == "winsize") {
									auto cols = Doc["cols"];
									auto rows = Doc["rows"];
									if (!RTTYS_server()->WindowSize(Id_, cols, rows)) {
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
				} break;
				case Poco::Net::WebSocket::FRAME_OP_BINARY: {
					if (n == 0) {
						Logger_.information("Frame binary size shutdown.");
						MustDisconnect = true;
					} else {
						poco_trace(Logger_, fmt::format("Sending {} key strokes to device.", n));
						if (!RTTYS_server()->SendKeyStrokes(
								Id_, (const unsigned char *)IncomingFrame.begin(),
								IncomingFrame.size())) {
							Logger_.information("Sendkeystrokes shutdown.");
							MustDisconnect = true;
						}
					}
				} break;
				case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
					Logger_.information("Frame frame close shutdown.");
					MustDisconnect = true;
				} break;

				default: {
				}
				}
			} catch (...) {
				Logger_.information("Frame readable shutdown.");
				MustDisconnect = true;
			}
		}

		if(MustDisconnect) {
			std::unique_lock	G(Mutex_);
			EndConnection();
		}
	}

	void RTTYS_ClientConnection::SendData( const u_char *Buf, size_t len ) {
		if(!Valid_)
			return;
		try {
			WS_->sendFrame(Buf, len,
						   Poco::Net::WebSocket::FRAME_FLAG_FIN |
							   Poco::Net::WebSocket::FRAME_OP_BINARY);
			return;
		} catch (...) {
			Logger_.information("SendData shutdown.");
		}
		std::unique_lock G(Mutex_);
		EndConnection();
	}

	void RTTYS_ClientConnection::SendData( const std::string &s) {
		if(!Valid_)
			return;
		try {
			WS_->sendFrame(s.c_str(), s.length());
			return;
		} catch (...) {
			Logger_.information("SendData shutdown.");
		}
		std::unique_lock G(Mutex_);
		EndConnection();
	}

	void RTTYS_ClientConnection::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		Logger_.information("Socket shutdown.");
		std::unique_lock G(Mutex_);
		EndConnection();
	}

}