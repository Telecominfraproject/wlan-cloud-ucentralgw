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
		  	Id_(Id)
		{
			Logger_.information(fmt::format("{}: Client starting connection, session: {}.",
											 Id_, RTTYS_server()->DeviceSessionID(Id_)));
			RTTYS_server()->ClientReactor().addEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
						  *this, &RTTYS_ClientConnection::onSocketReadable));
			RTTYS_server()->ClientReactor().addEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
						  *this, &RTTYS_ClientConnection::onSocketShutdown));

//			std::thread T([=]() { CompleteLogin(); });
//			T.detach();
		}

	void RTTYS_ClientConnection::CompleteLogin() {
		int tries = 0;
		completing_connection_ = true;
		try {
			Valid_ = true;
			while (!aborting_connection_ && tries < 30) {
				if (RTTYS_server()->Login(this->Id_)) {
					Logger_.information(fmt::format("{}: Client connected to device, session: {}.",
													 Id_, RTTYS_server()->DeviceSessionID(Id_)));
					Connected_ = true;
					completing_connection_ = false;
					return;
				}
				std::this_thread::sleep_for(1000ms);
				tries++;
				Logger_.information(fmt::format(
					"{}: Waiting for device to connect to start session. (try={})", Id_, tries));
			}
			Logger_.information(fmt::format("{}: Client could not connect to device, session: {}.",
											 Id_, RTTYS_server()->DeviceSessionID(Id_)));
		} catch (...) {
			completing_connection_ = false;
		}
		Guard G(Mutex_);
		EndConnection(false,G);
	}

	RTTYS_ClientConnection::~RTTYS_ClientConnection() {
		std::lock_guard	G(Mutex_);
		if(Valid_)
			EndConnection(false,G);
		delete WS_;
	}

	void RTTYS_ClientConnection::EndConnection(bool external, [[maybe_unused]] Guard &G ) {
		if(Valid_) {
			Valid_=false;
			if (Connected_) {
				Connected_=false;
				Logger_.information(fmt::format("{}: Client disconnecting.", Id_));
				RTTYS_server()->DeRegisterClient(Id_, this);
			}
			RTTYS_server()->ClientReactor().removeEventHandler(
				*WS_,
				Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
					*this, &RTTYS_ClientConnection::onSocketReadable));
			RTTYS_server()->ClientReactor().removeEventHandler(
				*WS_,
				Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
					*this, &RTTYS_ClientConnection::onSocketShutdown));
			WS_->shutdown();
			if(!external)
				RTTYS_server()->DisconnectNotice(Id_,false);
			Logger_.information(fmt::format("{}: Client disconnected.", Id_));
		}
	}

	void RTTYS_ClientConnection::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		std::lock_guard	G(Mutex_);
		bool MustDisconnect = false;
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
					Logger_.information(fmt::format("{}: Socket readable shutdown.", Id_));
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
									Logger_.information(
										fmt::format("{}: Winsize shutdown.", Id_));
									MustDisconnect = true;
								}
							}
						}
					} catch (...) {
						// just ignore parse errors
						Logger_.information(
							fmt::format("{}: Frame text exception shutdown.", Id_));
						MustDisconnect = true;
					}
				}
			} break;
			case Poco::Net::WebSocket::FRAME_OP_BINARY: {
				if (n == 0) {
					Logger_.information(fmt::format("{}: Frame binary size shutdown.", Id_));
					MustDisconnect = true;
				} else {
					poco_trace(Logger_, fmt::format("Sending {} key strokes to device.", n));
					if (!RTTYS_server()->SendKeyStrokes(Id_, Buffer_, n)) {
						Logger_.information(fmt::format("{}: Sendkeystrokes shutdown.", Id_));
						MustDisconnect = true;
					}
				}
			} break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
				Logger_.information(fmt::format("{}: Frame frame close shutdown.", Id_));
				MustDisconnect = true;
			} break;

			default: {
			}
			}
		} catch (...) {
			Logger_.information(fmt::format("{}: Frame readable shutdown.", Id_));
			MustDisconnect = true;
		}

		if(MustDisconnect)
			EndConnection(false,G);
	}

	void RTTYS_ClientConnection::SendData( const u_char *Buf, size_t len ) {
		std::lock_guard G(Mutex_);

		if(!Valid_)
			return;

		bool done = false;
		try {
			WS_->sendFrame(Buf, len,
						   Poco::Net::WebSocket::FRAME_FLAG_FIN |
							   Poco::Net::WebSocket::FRAME_OP_BINARY);
		} catch (...) {
			done = true;
			Logger_.information(fmt::format("{}: SendData shutdown.", Id_));
		}

		if(done)
			EndConnection(false,G);
	}

	void RTTYS_ClientConnection::SendData( const std::string &s , bool login) {
		std::lock_guard G(Mutex_);
		if(!Valid_)
			return;
		bool done = false;
		try {
			if (login) {
				RTTYS_server()->LoginDone(Id_);
			}
			WS_->sendFrame(s.c_str(), s.length());
		} catch (...) {
			done = true;
			Logger_.information(fmt::format("{}: Senddata shutdown.", Id_));
		}

		if(done)
			EndConnection(false,G);
	}

	void RTTYS_ClientConnection::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		Guard G(Mutex_);
		Logger_.information(fmt::format("{}: Socket shutdown.", Id_));
		EndConnection(false,G);
	}

}