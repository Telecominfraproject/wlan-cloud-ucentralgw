//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTY_ClientConnection.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_server.h"

namespace OpenWifi {

	RTTY_ClientConnection::RTTY_ClientConnection(Poco::Net::WebSocket &WS, std::string &Id,
						  Poco::Net::SocketReactor &Reactor)
						  : WS_(WS), Id_(std::move(Id)), SR_(Reactor) {
		SR_.addEventHandler(WS_,
							Poco::NObserver<RTTY_ClientConnection, Poco::Net::ReadableNotification>(
								*this, &RTTY_ClientConnection::onSocketReadable));
		SR_.addEventHandler(WS_,
							Poco::NObserver<RTTY_ClientConnection, Poco::Net::ShutdownNotification>(
								*this, &RTTY_ClientConnection::onSocketShutdown));
		std::cout << "We have a web socket...for " << Id_ << std::endl;
		RTTYS_server()->Register(Id_,this);
		RTTYS_server()->InitializeDevice(Id_, Sid_);
	}

	RTTY_ClientConnection::~RTTY_ClientConnection() {
		SR_.removeEventHandler(
			WS_, Poco::NObserver<RTTY_ClientConnection, Poco::Net::ReadableNotification>(
				*this, &RTTY_ClientConnection::onSocketReadable));
		SR_.removeEventHandler(
			WS_, Poco::NObserver<RTTY_ClientConnection, Poco::Net::ShutdownNotification>(
				*this, &RTTY_ClientConnection::onSocketShutdown));
		std::cout << "Closing client connection" << std::endl;
		RTTYS_server()->DeRegister(Id_,this);
	}

	void RTTY_ClientConnection::onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		char Buffer[8192];
		int flags;
		auto n = WS_.receiveFrame(Buffer, sizeof(Buffer), flags);

		auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

		switch(Op) {
			case Poco::Net::WebSocket::FRAME_OP_PING: {
					WS_.sendFrame("", 0,(int)Poco::Net::WebSocket::FRAME_OP_PONG | (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
				}
				break;
			case Poco::Net::WebSocket::FRAME_OP_PONG: {
				}
				break;
			case Poco::Net::WebSocket::FRAME_OP_TEXT: {
					std::cout << "Web Socket Received TEXT" << n << std::endl;
					if (n == 0)
						delete this;
					auto Device = RTTYS_server()->GetDevice(Id_);
					if(Device==nullptr) {
						std::cout << "Cannot send data to device: " << Id_ << std::endl;
						return;
					}
					Device->SendToDevice((u_char *)&Buffer[0],n);
				}
				break;
			case Poco::Net::WebSocket::FRAME_OP_BINARY: {
					std::cout << "Web Socket Received BINARY" << n << std::endl;
					if (n == 0)
						delete this;
					auto Device = RTTYS_server()->GetDevice(Id_);
					if(Device==nullptr) {
						std::cout << "Cannot send data to device: " << Id_ << std::endl;
						return;
					}
					Device->SendToDevice((u_char *)&Buffer[0],n);
				}
				break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
				}
				break;

			default:
			{

			}
		}
	}

	void RTTY_ClientConnection::SendData( const u_char *Buf, int len ) {
		WS_.sendFrame(Buf, len, Poco::Net::WebSocket::FRAME_FLAG_FIN | Poco::Net::WebSocket::FRAME_OP_BINARY);
	}

	void RTTY_ClientConnection::SendData( const std::string &s ) {
		WS_.sendFrame( s.c_str(), s.length());
	}
	void RTTY_ClientConnection::onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
		delete this;
	}

}