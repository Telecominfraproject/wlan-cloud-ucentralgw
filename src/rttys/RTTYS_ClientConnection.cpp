//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_ClientConnection.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_server.h"

namespace OpenWifi {

	RTTYS_ClientConnection::RTTYS_ClientConnection(Poco::Net::WebSocket *WS,
	   			Poco::Logger &L,
			   	std::string &Id)
		:
	  		WS_(WS),
			Logger_(L),
		  	Id_(std::move(Id))
		{

		//WS_ = new Poco::Net::WebSocket(Request, Response);
		RTTYS_server()->Register(Id_, this);
		Logger().information(fmt::format("{}: Client starting connection, session: {}.",
										 Id_, RTTYS_server()->DeviceSessionID(Id_)));
		RTTYS_server()->ClientReactor().addEventHandler(
			*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
					  *this, &RTTYS_ClientConnection::onSocketReadable));
		RTTYS_server()->ClientReactor().addEventHandler(
			*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
					  *this, &RTTYS_ClientConnection::onSocketShutdown));

		std::thread T([=]() { CompleteLogin(); });
		T.detach();
	}

	void RTTYS_ClientConnection::CompleteLogin() {
		int tries = 0;
		completing_connection_ = true;
		try {
			while (!aborting_connection_ && tries < 20) {
				if (RTTYS_server()->Login(this->Id_)) {
					Logger().information(fmt::format("{}: Client connected to device, session: {}.",
													 Id_, RTTYS_server()->DeviceSessionID(Id_)));
					this->Connected_ = true;
					completing_connection_ = false;
					return;
				}
				std::this_thread::sleep_for(2000ms);
				tries++;
				Logger().information(fmt::format(
					"{}: Waiting for device to connect to start session. (try={})", Id_, tries));
			}
			Logger().information(fmt::format("{}: Client could not connect to device, session: {}.",
											 Id_, RTTYS_server()->DeviceSessionID(Id_)));
			delete this;
		} catch (...) {
			completing_connection_ = false;
			delete this;
		}
	}

	RTTYS_ClientConnection::~RTTYS_ClientConnection() {
		// std::lock_guard	G(Mutex_);
		try {
			aborting_connection_ = true;
			if(completing_connection_) {
				aborting_connection_ = true;
				while(completing_connection_)
					continue;
			}
			Logger().information(fmt::format("{}: Client disconnecting.", Id_));
			RTTYS_server()->DeRegister(Id_, this);
			if (Connected_) {
				RTTYS_server()->ClientReactor().removeEventHandler(
					*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
							 *this, &RTTYS_ClientConnection::onSocketReadable));
				RTTYS_server()->ClientReactor().removeEventHandler(
					*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
							 *this, &RTTYS_ClientConnection::onSocketShutdown));
			}
			Logger().information(fmt::format("{}: Client disconnected.", Id_));
			delete WS_;
		} catch (...) {
			std::cout << "Exception when closing RTTY WebSocket" << std::endl;
			Logger().information(fmt::format("{}: Client disconnected.", Id_));
		}
	}

	void RTTYS_ClientConnection::Close() {
		{
			std::lock_guard G(Mutex_);
			CloseConnection_ = true;
		}
		delete this;
	}

	void RTTYS_ClientConnection::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		bool MustDisconnect = false;
		{
			std::lock_guard	G(Mutex_);
			try {
				int flags;
				auto n = WS_->receiveFrame(Buffer_, sizeof(Buffer_), flags);
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
						Logger().information(fmt::format("{}: Socket readable shutdown.", Id_));
						MustDisconnect = true;
					} else {
						std::string s((char *)Buffer_, n);
						try {
							auto Doc = nlohmann::json::parse(s);
							if (Doc.contains("type")) {
								auto Type = Doc["type"];
								if (Type == "winsize") {
									auto cols = Doc["cols"];
									auto rows = Doc["rows"];
									if (!RTTYS_server()->WindowSize(Id_, cols, rows)) {
										Logger().information(
											fmt::format("{}: Winsize shutdown.", Id_));
										MustDisconnect = true;
									}
								}
							}
						} catch (...) {
							// just ignore parse errors
							Logger().information(
								fmt::format("{}: Frame text exception shutdown.", Id_));
							MustDisconnect = true;
						}
					}
				} break;
				case Poco::Net::WebSocket::FRAME_OP_BINARY: {
					if (n == 0) {
						Logger().information(fmt::format("{}: Frame binary size shutdown.", Id_));
						MustDisconnect = true;
					} else {
						poco_trace(Logger(), fmt::format("Sending {} key strokes to device.", n));
						if (!RTTYS_server()->SendKeyStrokes(Id_, Buffer_, n)) {
							Logger().information(fmt::format("{}: Sendkeystrokes shutdown.", Id_));
							MustDisconnect = true;
						}
					}
				} break;
				case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
					Logger().information(fmt::format("{}: Frame frame close shutdown.", Id_));
					MustDisconnect = true;
				} break;

				default: {
				}
				}
			} catch (...) {
				Logger().information(fmt::format("{}: Frame readable shutdown.", Id_));
				MustDisconnect = true;
			}
		}

		if(MustDisconnect)
			delete this;
	}

	void RTTYS_ClientConnection::SendData( const u_char *Buf, size_t len ) {
		bool done = false;
		{
			std::lock_guard G(Mutex_);

			try {
				WS_->sendFrame(Buf, len,
							   Poco::Net::WebSocket::FRAME_FLAG_FIN |
								   Poco::Net::WebSocket::FRAME_OP_BINARY);
			} catch (...) {
				done = true;
				Logger().information(fmt::format("{}: SendData shutdown.", Id_));
			}
		}
		if(done)
			return delete this;
	}

	void RTTYS_ClientConnection::SendData( const std::string &s , bool login) {
		bool done = false;
		{
			try {
				std::lock_guard G(Mutex_);
				if (login) {
					RTTYS_server()->LoginDone(Id_);
				}
				WS_->sendFrame(s.c_str(), s.length());
			} catch (...) {
				Logger().information(fmt::format("{}: Senddata shutdown.", Id_));
				return delete this;
			}
		}
		if(done)
			return delete this;
	}

	void RTTYS_ClientConnection::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
//		RTTYS_server()->Close(Id_);
		Logger().information(fmt::format("{}: Socket shutdown.", Id_));
		delete this;
	}

}