//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_ClientConnection.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_server.h"

namespace OpenWifi {

	RTTYS_ClientConnection::RTTYS_ClientConnection(
		Poco::Net::HTTPServerRequest & Request,
		Poco::Net::HTTPServerResponse & Response,
		std::string &Id,
		Poco::Net::SocketReactor &Reactor, Poco::Logger &L)
		: 	Id_(std::move(Id)),
			SR_(Reactor),
			Logger_(L) {

		WS_ = new Poco::Net::WebSocket(Request, Response);
		RTTYS_server()->Register(Id_, this);
		Logger().information(fmt::format("{}: Client starting connection, session: {}.",
										 Id_, RTTYS_server()->DeviceSessionID(Id_)));
		SR_.addEventHandler(
			*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
					  *this, &RTTYS_ClientConnection::onSocketReadable));
		SR_.addEventHandler(
			*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
					  *this, &RTTYS_ClientConnection::onSocketShutdown));
		SR_.addEventHandler(
			*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ErrorNotification>(
					  *this, &RTTYS_ClientConnection::onSocketError));
		SR_.addEventHandler(
			*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::IdleNotification>(
					  *this, &RTTYS_ClientConnection::onIdle));

		auto DoLogin = [this]() -> void {
			int tries = 0;
			while (tries < 20) {
				if (RTTYS_server()->Login(this->Id_)) {
					Logger().information(
						fmt::format("{}: Client connected to device, session: {}.", Id_,
									RTTYS_server()->DeviceSessionID(Id_)));
					this->Connected_ = true;
					return;
				}
				std::this_thread::sleep_for(2000ms);
				tries++;
				Logger().information(fmt::format(
					"{}: Waiting for device to connect to start session. (try={})", Id_,
					tries));
			}
			Logger().information(
				fmt::format("{}: Client could not connect to device, session: {}.", Id_,
							RTTYS_server()->DeviceSessionID(Id_)));
			delete this;
		};

		std::thread CompleteConnection(DoLogin);
		CompleteConnection.detach();
	}

	RTTYS_ClientConnection::~RTTYS_ClientConnection() {
		Logger().information(fmt::format("{}: Client disconnecting.", Id_));
		RTTYS_server()->DeRegister(Id_, this);
		if(Connected_) {
			std::cout << "Removing handlers for WS-RTTY" << std::endl;
			SR_.removeEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ReadableNotification>(
						 *this, &RTTYS_ClientConnection::onSocketReadable));
			SR_.removeEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ShutdownNotification>(
						 *this, &RTTYS_ClientConnection::onSocketShutdown));
			SR_.removeEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::ErrorNotification>(
						  *this, &RTTYS_ClientConnection::onSocketError));
			SR_.removeEventHandler(
				*WS_, Poco::NObserver<RTTYS_ClientConnection, Poco::Net::IdleNotification>(
						  *this, &RTTYS_ClientConnection::onIdle));
		}
		delete WS_;
		Logger().information(fmt::format("{}: Client disconnected.", Id_));
	}

	void RTTYS_ClientConnection::Close() {
		CloseConnection_ = true;
	}

	void RTTYS_ClientConnection::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {

		int flags;
		auto n = WS_->receiveFrame(Buffer_, sizeof(Buffer_), flags);
		auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;
		switch(Op) {
			case Poco::Net::WebSocket::FRAME_OP_PING: {
					WS_->sendFrame("", 0,(int)Poco::Net::WebSocket::FRAME_OP_PONG | (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
				}
				break;
			case Poco::Net::WebSocket::FRAME_OP_PONG: {
				}
				break;
			case Poco::Net::WebSocket::FRAME_OP_TEXT: {
					if (n == 0)
						return delete this;
					std::string s((char*)Buffer_, n);
					try {
						auto Doc = nlohmann::json::parse(s);
						if (Doc.contains("type")) {
							auto Type = Doc["type"];
							if (Type == "winsize") {
								auto cols = Doc["cols"];
								auto rows = Doc["rows"];
								if(!RTTYS_server()->WindowSize(Id_,cols, rows)) {
									return delete this;
								}
							}
						}
					} catch (...) {
						// just ignore parse errors
					}
				}
				break;
			case Poco::Net::WebSocket::FRAME_OP_BINARY: {
					if (n == 0)
						return delete this;
					if(!RTTYS_server()->SendKeyStrokes(Id_,Buffer_,n)) {
						return delete this;
					}
				}
				break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
					return delete this;
				}
				break;

			default:
			{

			}
		}
	}

	void RTTYS_ClientConnection::SendData( const u_char *Buf, size_t len ) {
		WS_->sendFrame(Buf, len, Poco::Net::WebSocket::FRAME_FLAG_FIN | Poco::Net::WebSocket::FRAME_OP_BINARY);
	}

	void RTTYS_ClientConnection::SendData( const std::string &s , bool login) {
		if(login) {
			RTTYS_server()->LoginDone(Id_);
		}
		WS_->sendFrame( s.c_str(), s.length());
	}

	void RTTYS_ClientConnection::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
//		RTTYS_server()->Close(Id_);
		delete this;
	}

	void RTTYS_ClientConnection::onSocketError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
//		RTTYS_server()->Close(Id_);
		delete this;
	}

	void RTTYS_ClientConnection::onIdle([[maybe_unused]] const Poco::AutoPtr<Poco::Net::IdleNotification> &pNf) {
		std::cout << "onIdle..." << std::endl;
		if(CloseConnection_)
			delete this;
	}

}