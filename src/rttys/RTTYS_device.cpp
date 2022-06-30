//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"
#include "rttys/RTTYS_ClientConnection.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/StreamSocket.h"

#include <pthread.h>

namespace OpenWifi {

/*	RTTY_Device_ConnectionHandler::RTTY_Device_ConnectionHandler(const Poco::Net::StreamSocket & socket) :
		Poco::Net::TCPServerConnection(socket),
		Logger_(RTTYS_server()->Logger()) {
		device_address_ = socket.address().toString();
		Logger().information(fmt::format("{}: Started.", device_address_));
		conn_id_ = global_device_connection_id++;
	}
*/
	RTTY_Device_ConnectionHandler::RTTY_Device_ConnectionHandler(const Poco::Net::StreamSocket & socket) :
		Poco::Net::TCPServerConnection(socket) {
	}

	inline Poco::Logger & RTTY_Device_ConnectionHandler::Logger() { return RTTYS_server()->Logger(); }

/*	RTTY_Device_ConnectionHandler::~RTTY_Device_ConnectionHandler() {
		Logger().information(fmt::format("{}: Completing.", device_address_));
		running_ = false;
		RTTYS_server()->DeRegisterDevice(id_, this);
		socket().close();
		Logger().information(fmt::format("{}: Completed.", device_address_));
		~Poco::Net::TCPServerConnection();
	}
*/
	void RTTY_Device_ConnectionHandler::AddCommand(u_char C) {
		std::lock_guard		G(M_);
		// std::cout << conn_id_ << ": Adding command " << (int)C << std::endl;
		commands_.push_back(C);
	}

	bool RTTY_Device_ConnectionHandler::ProcessCommands() {
		std::lock_guard		G(M_);
		if(!commands_.empty()) {
			// std::cout << conn_id_ << ": Commands: " << commands_.size() << std::endl;
			for(const auto &i:commands_) {
//				std::cout << "Command: " << (int)i << std::endl;
				if(i==msgTypeLogin) {
// 					std::cout << "Doing login..." << std::endl;
					Login();
				}
				else if(i==msgTypeLogout) {
// 					std::cout << "Doing logout..." << std::endl;
					Logout();
				}
			}
			commands_.clear();
		}
		return true;
	}

#define _GNU_SOURCE

	void RTTY_Device_ConnectionHandler::run() {
		running_ = true ;

		OpenWifi::Utils::SetThreadName("RTTY-Device");

		device_address_ = socket().address().toString();
		Logger().information(fmt::format("{}: Started.", device_address_));
		conn_id_ = global_device_connection_id++;

		Poco::Timespan pollTimeOut(1,0);
		Poco::Timespan pollError(0,1);
		Poco::Timespan recvTimeOut(120,0);

		socket().setKeepAlive(true);
		socket().setNoDelay(true);
		socket().setReceiveBufferSize(64000);
		socket().setLinger(false,0);
		socket().setSendBufferSize(64000);
		socket().setReceiveTimeout(recvTimeOut);

		int reason=0;
		while(running_) {

			if (!socket().poll(pollTimeOut, Poco::Net::Socket::SELECT_READ | Poco::Net::Socket::SELECT_ERROR)) {
				continue;
			}

			int received = socket().receiveBytes(inBuf_);
			if(received<0) {
				running_ = false;
				reason=3;
				continue;
			}

			if(received==0) {
				continue;
			}

			while (!inBuf_.isEmpty() && running_) {
				// std::cout << conn_id_ << ": processing buffer" << std::endl;
				std::size_t msg_len;
				if (waiting_for_bytes_ == 0) {
					u_char header[3]{0};
					inBuf_.read((char *)&header[0], 3);
					last_command_ = header[0];
					msg_len = header[1] * 256 + header[2];
				} else {
					msg_len = received;
				}

				switch (last_command_) {
					case msgTypeRegister: {
						do_msgTypeRegister(msg_len);
					} break;
					case msgTypeLogin: {
						do_msgTypeLogin(msg_len);
					} break;
					case msgTypeLogout: {
						do_msgTypeLogout(msg_len);
					} break;
					case msgTypeTermData: {
						do_msgTypeTermData(msg_len);
					} break;
					case msgTypeWinsize: {
						do_msgTypeWinsize(msg_len);
					} break;
					case msgTypeCmd: {
						do_msgTypeCmd(msg_len);
					} break;
					case msgTypeHeartbeat: {
						do_msgTypeHeartbeat(msg_len);
					} break;
					case msgTypeFile: {
						do_msgTypeFile(msg_len);
					} break;
					case msgTypeHttp: {
						do_msgTypeHttp(msg_len);
					} break;
					case msgTypeAck: {
						do_msgTypeAck(msg_len);
					} break;
					case msgTypeMax: {
						do_msgTypeMax(msg_len);
					} break;
					default:
						Logger().warning(fmt::format("{}: ID:{} Unknown command {}", conn_id_, id_, (int)last_command_));
						running_ = false;
						continue;
					}
			}
		}
		Logger().information(fmt::format("{}: ID:{} Exiting. Reason:{}", conn_id_, id_, reason));
		Logger().information(fmt::format("{}: Completing.", device_address_));
		running_ = false;
		RTTYS_server()->DeRegisterDevice(id_, this);
		socket().close();
		Logger().information(fmt::format("{}: Completed.", device_address_));
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
		u_char outBuf[16]{0};

		if(len>(sizeof(outBuf)-5))
			return false;

		auto total_len = 3 + 1 + len-1;
		outBuf[0] = msgTypeTermData;
		outBuf[1] = 0 ;
		outBuf[2] = len +1-1;
		outBuf[3] = sid_;
		memcpy( &outBuf[4], &buf[1], len-1);
		try {
			socket().sendBytes(outBuf, total_len);
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
			socket().sendBytes(outBuf, 8);
			return true;
		} catch (...) {

		}
		return false;
	}

	bool RTTY_Device_ConnectionHandler::Login() {
		std::lock_guard		G(M_);
		u_char outBuf[3]{0};
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		outBuf[2] = 0;
		try {
			socket().sendBytes(outBuf, 3);
		} catch (const Poco::IOException &E) {
			// std::cout << "1  " << E.what() << " " << E.name() << " "<< E.className() << " "<< E.message() << std::endl;
			return false;
		} catch (const Poco::Exception &E) {
			// std::cout << "2  " << E.what() << " " << E.name() << std::endl;
			return false;
		}
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
			socket().sendBytes(outBuf, 4);
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
		socket().receiveBytes(inBuf_);
		id_ = ReadString();
		desc_ = ReadString();
		token_ = ReadString();
		serial_ = RTTYS_server()->SerialNumber(id_);

		OpenWifi::Utils::SetThreadName(serial_.c_str());

		Poco::Thread::current()->setName(fmt::format("RTTY-device-thread-{}:{}:{}", conn_id_, id_, serial_));
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
			if(socket().sendBytes(OutBuf, 7) !=7) {
				Logger().debug(fmt::format("{}: ID:{} Serial:{} Description:{} Could not complete registration", conn_id_, id_, serial_, desc_));
				running_ = false;
			}
		} else {
			Logger().debug(fmt::format("{}: ID:{} Serial:{} Description:{} Could not complete registration", conn_id_, id_, serial_, desc_));
			running_ = false;
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
		socket().sendBytes(MsgBuf, 3);
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