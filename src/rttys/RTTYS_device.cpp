//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"
#include "rttys/RTTYS_ClientConnection.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/StreamSocket.h"

namespace OpenWifi {

	inline Poco::Logger & RTTY_Device_ConnectionHandler::Logger() { return RTTYS_server()->Logger(); }

	RTTY_Device_ConnectionHandler::RTTY_Device_ConnectionHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor & reactor):
			 _socket(socket),
			 _reactor(reactor),
			inBuf_(RTTY_DEVICE_BUFSIZE){
		std::thread T([=]() { CompleteConnection(); });
		T.detach();
	}

	void RTTY_Device_ConnectionHandler::CompleteConnection() {
		if(MicroService::instance().NoAPISecurity()) {

		} else {
			auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(_socket.impl());
			while (true) {
				auto V = SS->completeHandshake();
				if (V == 1)
					break;
			}
			if((SS->secure()))
				std::cout << "Secure..." << std::endl;
		}
		device_address_ = _socket.peerAddress();
		Logger().information(fmt::format("{}: Starting.", device_address_.toString()));
		std::cout << "Peer: " << device_address_.toString() << std::endl;
		_reactor.addEventHandler(_socket, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ReadableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketReadable));
		_reactor.addEventHandler(_socket, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketShutdown));
	}


	RTTY_Device_ConnectionHandler::~RTTY_Device_ConnectionHandler() {
		_reactor.removeEventHandler(_socket, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ReadableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketReadable));
		_reactor.removeEventHandler(_socket, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::WritableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketWritable));
		_reactor.removeEventHandler(_socket, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketShutdown));

		Logger().information(fmt::format("{}: Deregistering.", device_address_.toString()));
		if(registered_)
			RTTYS_server()->DeRegisterDevice(id_, this);
		Logger().information(fmt::format("{}: Deregistered.", device_address_.toString()));
	}

	void RTTY_Device_ConnectionHandler::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		try {
			auto received_bytes = _socket.receiveBytes(inBuf_);
			if(received_bytes==0) {
				// std::cout << __LINE__ << std::endl;
				return;
				// delete this;
			}

			std::cout << "Received " << received_bytes << std::endl;
			while (!inBuf_.isEmpty()) {
				std::cout << __LINE__ << std::endl;
				std::size_t msg_len;
				if (waiting_for_bytes_ == 0) {
					u_char header[3]{0};
					inBuf_.read((char *)&header[0], 3);
					last_command_ = header[0];
					msg_len = header[1] * 256 + header[2];
				} else {
					msg_len = received_bytes;
				}

				std::cout << __LINE__ << std::endl;
				switch (last_command_) {
				case msgTypeRegister: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeRegister(msg_len);
				} break;
				case msgTypeLogin: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeLogin(msg_len);
				} break;
				case msgTypeLogout: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeLogout(msg_len);
				} break;
				case msgTypeTermData: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeTermData(msg_len);
				} break;
				case msgTypeWinsize: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeWinsize(msg_len);
				} break;
				case msgTypeCmd: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeCmd(msg_len);
				} break;
				case msgTypeHeartbeat: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeHeartbeat(msg_len);
				} break;
				case msgTypeFile: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeFile(msg_len);
				} break;
				case msgTypeHttp: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeHttp(msg_len);
				} break;
				case msgTypeAck: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeAck(msg_len);
				} break;
				case msgTypeMax: {
					std::cout << __LINE__ << std::endl;
					do_msgTypeMax(msg_len);
				} break;
				default:
					Logger().warning(fmt::format("{}: ID:{} Unknown command {}", conn_id_, id_,
												 (int)last_command_));
					continue;
				}
			}
		} catch (...) {

		}
	}

	void RTTY_Device_ConnectionHandler::onSocketWritable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::WritableNotification>& pNf) {

	}

	void RTTY_Device_ConnectionHandler::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		std::cout << __LINE__ << std::endl;
		delete this;
	}

	void RTTY_Device_ConnectionHandler::Stop() {
		running_ = false;
	}

	void RTTY_Device_ConnectionHandler::SendToClient(const u_char *Buf, int Len) {
		RTTYS_server()->SendToClient(id_, Buf, Len);
	}

	void RTTY_Device_ConnectionHandler::SendToClient(const std::string &S) {
		RTTYS_server()->SendToClient(id_, S);
	}

	bool RTTY_Device_ConnectionHandler::KeyStrokes(const u_char *buf, size_t len) {
		std::lock_guard		G(M_);

		if(len>(RTTY_DEVICE_BUFSIZE-5))
			return false;

		auto total_len = 3 + 1 + len-1;
		scratch_[0] = msgTypeTermData;
		scratch_[1] = (len & 0xff00) >> 8 ;
		scratch_[2] = (len & 0x00ff) ;
		scratch_[3] = sid_;
		memcpy( &scratch_[4], &buf[1], len-1);
		try {
			_socket.sendBytes((const void *)&scratch_[0], total_len);
			return true;
		} catch (...) {
			return false;
		}
	}

	bool RTTY_Device_ConnectionHandler::WindowSize(int cols, int rows) {
		std::lock_guard		G(M_);
		u_char	outBuf[8]{0};
		outBuf[0] = msgTypeWinsize;
		outBuf[1] = 0 ;
		outBuf[2] = 4 + 1 ;
		outBuf[3] = sid_;
		outBuf[4] = cols >> 8 ;
		outBuf[5] = cols & 0x00ff;
		outBuf[6] = rows >> 8;
		outBuf[7] = rows & 0x00ff;
		try {
			_socket.sendBytes(outBuf, 8);
			return true;
		} catch (...) {

		}
		return false;
	}

	bool RTTY_Device_ConnectionHandler::Login() {
		std::lock_guard		G(M_);
		std::cout << __LINE__ << std::endl;
		u_char outBuf[3]{0};
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		outBuf[2] = 0;
		try {
			std::cout << __LINE__ << std::endl;
			_socket.sendBytes(outBuf, 3);
			std::cout << __LINE__ << std::endl;
		} catch (const Poco::IOException &E) {
			// std::cout << "1  " << E.what() << " " << E.name() << " "<< E.className() << " "<< E.message() << std::endl;
			return false;
		} catch (const Poco::Exception &E) {
			// std::cout << "2  " << E.what() << " " << E.name() << std::endl;
			return false;
		}
		received_login_from_websocket_ = true;
		Logger().debug(fmt::format("{}: Device {} login", conn_id_, id_));
		return true;
	}

	bool RTTY_Device_ConnectionHandler::Logout() {
		std::lock_guard		G(M_);
		u_char outBuf[4]{0};
		outBuf[0] = msgTypeLogout;
		outBuf[1] = 0;
		outBuf[2] = 1;
		outBuf[3] = sid_;
		Logger().debug(fmt::format("{}: ID:{} Logout", conn_id_, id_));
		try {
			_socket.sendBytes(outBuf, 4);
			return true;
		} catch (...) {

		}
		return false;
	}

	std::string RTTY_Device_ConnectionHandler::ReadString() {
		std::string Res;

		while(inBuf_.used()) {
			char C;
			inBuf_.read(&C,1);
			if(C==0) {
				break;
			}
			Res += C;
		}

		return Res;
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeRegister([[maybe_unused]] std::size_t msg_len) {
		std::cout << __LINE__ << std::endl;
		id_ = ReadString();
		std::cout << __LINE__ << std::endl;
		desc_ = ReadString();
		std::cout << __LINE__ << std::endl;
		token_ = ReadString();
		std::cout << __LINE__ << std::endl;
		serial_ = RTTYS_server()->SerialNumber(id_);
		std::cout << __LINE__ << std::endl;

		Logger().debug(fmt::format("{}: ID:{} Serial:{} Description:{} Device registration", conn_id_, id_, serial_, desc_));
		if (RTTYS_server()->Register(id_, token_, this)) {
			u_char OutBuf[8];
			OutBuf[0] = msgTypeRegister;
			OutBuf[1] = 0;
			OutBuf[2] = 4;
			OutBuf[3] = 0;
			OutBuf[4] = 'O';
			OutBuf[5] = 'K';
			OutBuf[6] = 0;
			if(_socket.sendBytes(OutBuf, 7) !=7) {
				Logger().debug(fmt::format("{}: ID:{} Serial:{} Description:{} Could not complete registration", conn_id_, id_, serial_, desc_));
			}
			registered_ = true;
		} else {
			Logger().debug(fmt::format("{}: ID:{} Serial:{} Description:{} Could not complete registration", conn_id_, id_, serial_, desc_));
		}
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeLogin([[maybe_unused]] std::size_t msg_len) {
		Logger().debug(fmt::format("{}: ID:{} Serial:{} Asking for login", conn_id_, id_, serial_));
		nlohmann::json doc;
		char Error;
		inBuf_.read(&Error, 1);
		inBuf_.read(&sid_, 1);
		doc["type"] = "login";
		doc["err"] = Error;
		const auto login_msg = to_string(doc);
		SendToClient(login_msg);
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeLogout([[maybe_unused]] std::size_t msg_len) {
		Logger().debug(fmt::format("{}: ID:{} Serial:{} Asking for logout", conn_id_, id_, serial_));
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeTermData(std::size_t msg_len) {
		if(waiting_for_bytes_!=0) {
			auto to_read = std::min(inBuf_.used(),waiting_for_bytes_);
			inBuf_.read(&scratch_[0], to_read);
			SendToClient((u_char *)&scratch_[0], (int) to_read);
			if(to_read<waiting_for_bytes_)
				waiting_for_bytes_ -= to_read;
			else
				waiting_for_bytes_ = 0 ;
		} else {
			if(inBuf_.used()<msg_len) {
				auto read_count = inBuf_.read(&scratch_[0], inBuf_.used());
				SendToClient((u_char *)&scratch_[0], read_count);
				waiting_for_bytes_ = msg_len - read_count;
			} else {
				inBuf_.read(&scratch_[0], msg_len);
				SendToClient((u_char *)&scratch_[0], (int)msg_len);
				waiting_for_bytes_=0;
			}
		}
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeWinsize([[maybe_unused]] std::size_t msg_len) {
		Logger().debug(fmt::format("{}: ID:{} Serial:{} Asking for msgTypeWinsize", conn_id_, id_, serial_));
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeCmd([[maybe_unused]] std::size_t msg_len) {
		Logger().debug(fmt::format("{}: ID:{} Serial:{} Asking for msgTypeCmd", conn_id_, id_, serial_));
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeHeartbeat([[maybe_unused]] std::size_t msg_len) {
		u_char MsgBuf[3]{0};
		MsgBuf[0] = msgTypeHeartbeat;
		_socket.sendBytes(MsgBuf, 3);
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeFile([[maybe_unused]] std::size_t msg_len) {
		Logger().debug(fmt::format("{}: ID:{} Serial:{} Asking for msgTypeFile", conn_id_, id_, serial_));
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeHttp([[maybe_unused]] std::size_t msg_len) {
		Logger().debug(fmt::format("{}: ID:{} Serial:{} Asking for msgTypeHttp", conn_id_, id_, serial_));
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeAck([[maybe_unused]] std::size_t msg_len) {
		Logger().debug(fmt::format("{}: ID:{} Serial:{} Asking for msgTypeAck", conn_id_, id_, serial_));
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeMax([[maybe_unused]] std::size_t msg_len) {
		Logger().debug(fmt::format("{}: ID:{} Serial:{} Asking for msgTypeMax", conn_id_, id_, serial_));
	}
}