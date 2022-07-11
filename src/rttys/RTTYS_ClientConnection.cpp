//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_ClientConnection.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_server.h"

namespace OpenWifi {

	RTTYS_ClientConnection::RTTYS_ClientConnection(Poco::Net::WebSocket *WS,
			   	const std::string &Id)
		:
	  		WS_(WS),
		  	Id_(Id),
	  		Logger_(Poco::Logger::get(fmt::format("RTTY-client({})",Id_)))
		{
			Logger_.information("Starting connection");
			Valid_ = true;
			RTTYS_server()->ClientReactor().addEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
						  *this, &RTTYS_ClientConnection::onSocketReadable));
			RTTYS_server()->ClientReactor().addEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
						  *this, &RTTYS_ClientConnection::onSocketShutdown));
			logging_in_ = true;
			/*
			std::thread T([=]() { CompleteStartup(); });
			T.detach();
	*/
		}

	bool RTTYS_ClientConnection::CompleteStartup() {
		int tries = 0;
		try {
			std::lock_guard	G(Mutex_);
			while (!abort_connection_ && tries < 30) {
				if (RTTYS_server()->Login(this->Id_)) {
					Logger_.information("Connected to device");
					Connected_ = true;
					logging_in_ = false;
					return true;
				}
				std::this_thread::sleep_for(1000ms);
				std::this_thread::yield();
				tries++;
				Logger_.information(fmt::format(
					"Waiting for device to connect to start session. (try={})", tries));
			}
			Logger_.information("Could not connect to device");
		} catch (...) {
		}
		Valid_ = false;
		logging_in_ = false;
		RTTYS_server()->ClientReactor().addEventHandler(
			*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
					  *this, &RTTYS_ClientConnection::onSocketReadable));
		RTTYS_server()->ClientReactor().addEventHandler(
			*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
					  *this, &RTTYS_ClientConnection::onSocketShutdown));
		return false;
	}

	RTTYS_ClientConnection::~RTTYS_ClientConnection() {
		while(logging_in_) {
			std::this_thread::sleep_for(100ms);
			std::this_thread::yield();
		}
		std::lock_guard	G(Mutex_);
		if(Valid_)
			EndConnection(false,G);
	}

	void RTTYS_ClientConnection::EndConnection(bool external, [[maybe_unused]] Guard &G ) {
		if(Valid_) {
			Valid_=false;
			RTTYS_server()->ClientReactor().removeEventHandler(
				*WS_,
				Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
					*this, &RTTYS_ClientConnection::onSocketReadable));
			RTTYS_server()->ClientReactor().removeEventHandler(
				*WS_,
				Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
					*this, &RTTYS_ClientConnection::onSocketShutdown));
			WS_->shutdown();
			if (Connected_) {
				Connected_=false;
				Logger_.information("Disconnecting.");
				RTTYS_server()->DeRegisterClient(Id_, this);
			}
			if(!external)
				RTTYS_server()->DisconnectNotice(Id_,false);
			Logger_.information("Disconnected.");
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
					Logger_.information("Socket readable shutdown.");
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
					if (!RTTYS_server()->SendKeyStrokes(Id_, Buffer_, n)) {
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
			Logger_.information("SendData shutdown.");
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
			Logger_.information("Senddata shutdown.");
		}

		if(done)
			EndConnection(false,G);
	}

	void RTTYS_ClientConnection::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		abort_connection_ = true;
		Guard G(Mutex_);
		Logger_.information("Socket shutdown.");
		EndConnection(false,G);
	}

}