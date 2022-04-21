//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"
#include "rttys/RTTYS_ClientConnection.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/StreamSocket.h"

namespace OpenWifi {

	RTTY_Device_ConnectionHandler::RTTY_Device_ConnectionHandler(const Poco::Net::StreamSocket & socket) :
		Poco::Net::TCPServerConnection(socket),
		Logger_(RTTYS_server()->Logger()) {
		conn_id_ = global_device_connection_id++;
	}

	RTTY_Device_ConnectionHandler::~RTTY_Device_ConnectionHandler() {
		running_ = false;
		while(!loop_done_) {
			Poco::Thread::sleep(10);
		}
		RTTYS_server()->DeRegister(id_, this);
	}

	void RTTY_Device_ConnectionHandler::AddCommand(u_char C) {
		std::lock_guard		G(M_);
		std::cout << conn_id_ << ": Adding command " << (int)C << std::endl;
		commands_.push_back(C);
	}


	void RTTY_Device_ConnectionHandler::run() {
		running_ = true ;
		auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(socket().impl());
		while (true) {
			auto V = SS->completeHandshake();
			if (V == 1)
				break;
		}
		socket().setKeepAlive(true);
		socket().setNoDelay(true);
		socket().setReceiveBufferSize(64000);

		Poco::Timespan timeOut(10,0);

		while(running_) {
			{
				std::lock_guard		G(M_);
				std::cout << conn_id_ << ": Looking for commands" << std::endl;
				if(!commands_.empty()) {
					std::cout << conn_id_ << ": Commands: " << commands_.size() << std::endl;
					for(const auto &i:commands_) {
						std::cout << "Command: " << (int)i << std::endl;
						if(i==msgTypeLogin) {
							Login();
						}
						else if(i==msgTypeLogout) {
							Logout();
						}
					}
					commands_.clear();
				}
			}

			if(socket().poll(timeOut,Poco::Net::Socket::SELECT_READ) == false) {

			} else {
				std::lock_guard		G(M_);

				std::cout << "Getting bytes..." << std::endl;
				int received = socket().receiveBytes(inBuf_);

				if(received<0) {

				}

				std::cout << "Received " << received << " bytes." << std::endl;
				while (!inBuf_.isEmpty() && running_) {
					std::cout << conn_id_ << ": processing buffer" << std::endl;
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
							std::cout << conn_id_ << ": Unknown command: " << (int)last_command_
									  << std::endl;
					}
				}
			}
		}
		std::cout << conn_id_ << ": Loop done" << std::endl;
		loop_done_=true;
	}

	void RTTY_Device_ConnectionHandler::Stop() {
		running_ = true;
	}

	void RTTY_Device_ConnectionHandler::SendToClient(const u_char *Buf, int len) {
		auto Client = RTTYS_server()->GetClient(id_);
		if(Client!= nullptr)
			Client->SendData(Buf,len);
	}

	void RTTY_Device_ConnectionHandler::SendToClient(const std::string &S) {
		auto Client = RTTYS_server()->GetClient(id_);
		if(Client!= nullptr)
			Client->SendData(S);
	}

	void RTTY_Device_ConnectionHandler::KeyStrokes(const u_char *buf, size_t len) {
		std::lock_guard		G(M_);
		u_char outBuf[16]{0};

		if(len>(sizeof(outBuf)-5))
			return;

		auto total_len = 3 + 1 + len-1;
		outBuf[0] = msgTypeTermData;
		outBuf[1] = 0 ;
		outBuf[2] = len +1-1;
		outBuf[3] = sid_;
		memcpy( &outBuf[4], &buf[1], len-1);
		socket().sendBytes(outBuf, total_len);
	}

	void RTTY_Device_ConnectionHandler::WindowSize(int cols, int rows) {
		std::lock_guard		G(M_);
		u_char	outBuf[32]{0};
		outBuf[0] = msgTypeWinsize;
		outBuf[1] = 0 ;
		outBuf[2] = 4 + 1 ;
		outBuf[3] = sid_;
		outBuf[4] = cols >> 8 ;
		outBuf[5] = cols & 0x00ff;
		outBuf[6] = rows >> 8;
		outBuf[7] = rows & 0x00ff;
		socket().sendBytes(outBuf,8);
	}

	bool RTTY_Device_ConnectionHandler::Login() {
		std::lock_guard		G(M_);
		std::cout << __LINE__ << std::endl;
		u_char outBuf[8]{0};
		std::cout << __LINE__ << std::endl;
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		outBuf[2] = 0;
		std::cout << __LINE__ << std::endl;
		try {
			socket().sendBytes(outBuf, 3);
		} catch (const Poco::IOException &E) {
			std::cout << "1  " << E.what() << " " << E.name() << " "<< E.className() << " "<< E.message() << std::endl;
		} catch (const Poco::Exception &E) {
			std::cout << "2  " << E.what() << " " << E.name() << std::endl;
		}
		std::cout << __LINE__ << std::endl;
		Logger().debug(fmt::format("{}: Device {} login", conn_id_, id_));
		std::cout << __LINE__ << std::endl;
		return true;
	}

	bool RTTY_Device_ConnectionHandler::Logout() {
		std::lock_guard		G(M_);
		u_char outBuf[4]{0};
		outBuf[0] = msgTypeLogout;
		outBuf[1] = 0;
		outBuf[2] = 1;
		outBuf[3] = sid_;
		Logger().debug(fmt::format("{}: Device {} logout", conn_id_, id_));
		socket().sendBytes(outBuf,4 );
		return true;
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
		std::cout << conn_id_ << ": Device Registration ID:" << id_ << " DESC:" << desc_ << " TOK:"
				  << token_ << std::endl;
		if (RTTYS_server()->Register(id_, token_, this)) {
			serial_ = RTTYS_server()->SerialNumber(id_);
			Logger().debug(fmt::format("{}: Registration for SerialNumber: {}, Description: {}",
									   conn_id_, serial_, desc_));
			u_char OutBuf[8];
			OutBuf[0] = msgTypeRegister;
			OutBuf[1] = 0;
			OutBuf[2] = 4;
			OutBuf[3] = 0;
			OutBuf[4] = 'O';
			OutBuf[5] = 'K';
			OutBuf[6] = 0;
			socket().sendBytes(OutBuf, 7);
		} else {
			std::cout << conn_id_ << ": not allowed to register" << std::endl;
			running_ = false;
		}
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeLogin([[maybe_unused]] std::size_t msg_len) {
		std::cout << conn_id_ << ": Device Login..." << std::endl;
		Logger().debug(fmt::format(
			"{}: Device created session for SerialNumber: {}, session: {}", conn_id_, serial_, id_));
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
		std::cout << conn_id_ << ": Device msgTypeLogout" << std::endl;
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeTermData(std::size_t msg_len) {
		if(waiting_for_bytes_!=0) {
			std::cout << conn_id_ << ": S2:" << inBuf_.used() << std::endl;
			auto to_read = std::min(inBuf_.used(),waiting_for_bytes_);
			inBuf_.read(&scratch_[0], to_read);
			SendToClient((u_char *)&scratch_[0], (int) to_read);
			if(to_read<waiting_for_bytes_)
				waiting_for_bytes_ -= to_read;
			else
				waiting_for_bytes_ = 0 ;
		} else {
			if(inBuf_.used()<msg_len) {
				std::cout << conn_id_ << ": S1:" << msg_len << std::endl;
				auto read_count = inBuf_.read(&scratch_[0], inBuf_.used());
				SendToClient((u_char *)&scratch_[0], read_count);
				waiting_for_bytes_ = msg_len - read_count;
			} else {
				std::cout << conn_id_ << ": S0:" << msg_len << std::endl;
				inBuf_.read(&scratch_[0], msg_len);
				SendToClient((u_char *)&scratch_[0], (int)msg_len);
				waiting_for_bytes_=0;
			}
		}
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeWinsize([[maybe_unused]] std::size_t msg_len) {
		std::cout << conn_id_ << ": Device msgTypeWinsize" << std::endl;
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeCmd([[maybe_unused]] std::size_t msg_len) {
		std::cout << conn_id_ << ": Device msgTypeCmd" << std::endl;
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeHeartbeat([[maybe_unused]] std::size_t msg_len) {
		std::cout << conn_id_ << ": Device msgTypeHeartbeat: " << std::endl;
		u_char MsgBuf[3]{0};
		MsgBuf[0] = msgTypeHeartbeat;
		socket().sendBytes(MsgBuf, 3);
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeFile([[maybe_unused]] std::size_t msg_len) {
		std::cout << conn_id_ << ": Device msgTypeFile" << std::endl;
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeHttp([[maybe_unused]] std::size_t msg_len) {
		std::cout << conn_id_ << ": Device msgTypeHttp" << std::endl;
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeAck([[maybe_unused]] std::size_t msg_len) {
		std::cout << conn_id_ << ": Device msgTypeAck" << std::endl;
	}

	void RTTY_Device_ConnectionHandler::do_msgTypeMax([[maybe_unused]] std::size_t msg_len) {
		std::cout << conn_id_ << ": Device msgTypeMax" << std::endl;
	}
}