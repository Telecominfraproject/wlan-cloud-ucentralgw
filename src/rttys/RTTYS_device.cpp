//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/StreamSocket.h"

namespace OpenWifi {

	RTTYS_Device_ConnectionHandler::RTTYS_Device_ConnectionHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor & reactor):
			 	socket_(socket),
			 	reactor_(reactor),
				Logger_(Poco::Logger::get(fmt::format("RTTY-device({})",socket_.peerAddress().toString())))
	{
//		std::thread T([=]() { CompleteConnection(); });
//		T.detach();
		CompleteConnection();
	}

	void RTTYS_Device_ConnectionHandler::CompleteConnection() {
		try {
			valid_=true;
			device_address_ = socket_.peerAddress();

			auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(socket_.impl());
			while (true) {
				auto V = SS->completeHandshake();
				if (V == 1)
					break;
			}

			if ((SS->secure())) {
				poco_information(Logger(), "Secure connection.");
			}

			reactor_.addEventHandler(
				socket_,
				Poco::NObserver<RTTYS_Device_ConnectionHandler, Poco::Net::ReadableNotification>(
					*this, &RTTYS_Device_ConnectionHandler::onSocketReadable));
			reactor_.addEventHandler(
				socket_,
				Poco::NObserver<RTTYS_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(
					*this, &RTTYS_Device_ConnectionHandler::onSocketShutdown));
		} catch (...) {
			poco_warning(Logger(), "Device caused exception while completing connection.");
			std::unique_lock G(M_);
			EndConnection();
		}
	}

	RTTYS_Device_ConnectionHandler::~RTTYS_Device_ConnectionHandler() {
		if(valid_) {
			std::unique_lock G(M_);
			poco_warning(Logger(), "Device connection being deleted.");
			EndConnection(false);
		}
	}

	void RTTYS_Device_ConnectionHandler::EndConnection(bool SendNotification) {
		try {
			if(valid_) {
				valid_ = false;
				reactor_.removeEventHandler(
					socket_,
					Poco::NObserver<RTTYS_Device_ConnectionHandler, Poco::Net::ReadableNotification>(
						*this, &RTTYS_Device_ConnectionHandler::onSocketReadable));
				reactor_.removeEventHandler(
					socket_,
					Poco::NObserver<RTTYS_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(
						*this, &RTTYS_Device_ConnectionHandler::onSocketShutdown));
				if(SendNotification)
					RTTYS_server()->NotifyDeviceDisconnect(id_,this);
				poco_information(Logger(), "Connection done.");
				socket_.close();
			}
		} catch (...) {

		}
	}

	[[maybe_unused]] static void dump(unsigned char *p,uint l) {
		for(uint i=0;i<l;i++) {
			std::cout << std::hex << (uint) p[i] << " ";
			if(i % 16 == 0)
				std::cout << std::endl;
		}
		std::cout << std::dec << std::endl ;
	}

	void RTTYS_Device_ConnectionHandler::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		bool good = true;

		std::unique_lock G(M_);
		try {
			auto received_bytes = socket_.receiveBytes(inBuf_);
			if (received_bytes == 0) {
				poco_information(Logger(), fmt::format("{}: Device Closing connection - 0 bytes received.",id_));
				return EndConnection();
			}

			while (inBuf_.isReadable() && good) {
				uint32_t msg_len = 0;
				if (waiting_for_bytes_ != 0) {

				} else {
					if (inBuf_.used() >= RTTY_HDR_SIZE) {
						auto *head = (unsigned char *)inBuf_.begin();
						last_command_ = head[0];
						msg_len = head[1] * 256 + head[2];
						inBuf_.drain(RTTY_HDR_SIZE);
					} else {
						good = false;
						if (!good)
							std::cout << "do_msgTypeTermData:5     " << inBuf_.used() << std::endl;
						continue;
					}
				}

				switch (last_command_) {
					case msgTypeRegister: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeRegister(msg_len);
					} break;
					case msgTypeLogin: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeLogin(msg_len);
					} break;
					case msgTypeLogout: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeLogout(msg_len);
					} break;
					case msgTypeTermData: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeTermData(msg_len);
					} break;
					case msgTypeWinsize: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeWinsize(msg_len);
					} break;
					case msgTypeCmd: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeCmd(msg_len);
					} break;
					case msgTypeHeartbeat: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeHeartbeat(msg_len);
					} break;
					case msgTypeFile: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeFile(msg_len);
					} break;
					case msgTypeHttp: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeHttp(msg_len);
					} break;
					case msgTypeAck: {
						std::cout << __func__ << ":" << __LINE__ << std::endl;
						good = do_msgTypeAck(msg_len);
					} break;
					case msgTypeMax: {
						good = do_msgTypeMax(msg_len);
					} break;
					default: {
						poco_warning(Logger(),
									 fmt::format("{}: Unknown command {} from device. GW closing connection.", id_,
												 (int)last_command_));
						good = false;
					}
				}
			}
		} catch (const Poco::Exception &E) {
			good = false;
			Logger().log(E,__FILE__,__LINE__);
			poco_warning(Logger(),fmt::format("{}: Exception. GW closing connection.", id_));
		} catch (const std::exception &E) {
			poco_warning(Logger(),fmt::format("{}: Exception. GW closing connection.", id_));
			good = false;
		}

		if(!good) {
			return EndConnection();
		}
	}

	void RTTYS_Device_ConnectionHandler::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		std::unique_lock G(M_);
		poco_information(Logger(),fmt::format("{}: Connection being closed - socket shutdown.",id_));
		EndConnection();
	}

	bool RTTYS_Device_ConnectionHandler::SendToClient(const u_char *Buf, int Len) {
		u_char bb[64000]{0};
		if(short_session_id_) {
			bb[0] = session_id_[0];
			memcpy(&bb[1],Buf,Len);
		} else {
			bb[0] = 0;
			memcpy(&bb[1],Buf,Len);
		}
		std::cout << ">>>" << bb << std::endl;
		return RTTYS_server()->SendToClient(id_, Buf, Len );
	}

	bool RTTYS_Device_ConnectionHandler::SendToClient(const std::string &S) {
		return RTTYS_server()->SendToClient(id_,S);
	}

	bool RTTYS_Device_ConnectionHandler::KeyStrokes(const u_char *buf, size_t len) {
		std::cout << "Trying to send keystrokes." << std::endl;
		if(!valid_)
			return false;

		std::cout << "Sending: " << len << " keys." << std::endl;
		if(len<=(sizeof(small_buf_)-RTTY_HDR_SIZE-session_length_)) {
			small_buf_[0] = msgTypeTermData;
			small_buf_[1] = ((len-1+session_length_) & 0xff00) >> 8;
			small_buf_[2] = ((len-1+session_length_) & 0x00ff);
			memcpy(&small_buf_[RTTY_HDR_SIZE],session_id_,session_length_);
			memcpy(&small_buf_[RTTY_HDR_SIZE+session_length_], &buf[1], len-1);
			try {
				auto Sent = socket_.sendBytes(small_buf_, RTTY_HDR_SIZE + session_length_ + len - 1);
				std::cout << "KeyStrokes: Sent (smallbuf): " << Sent << std::endl;
				return true;
			} catch (...) {
				return false;
			}
		} else {
			auto Msg = std::make_unique<unsigned char []>(len + RTTY_HDR_SIZE + session_length_);
			Msg.get()[0] = msgTypeTermData;
			Msg.get()[1] = ((len+session_length_) & 0xff00) >> 8;
			Msg.get()[2] = ((len+session_length_) & 0x00ff);
			memcpy((Msg.get()+RTTY_HDR_SIZE),session_id_,session_length_);
			memcpy((Msg.get()+RTTY_HDR_SIZE+session_length_), &buf[1], len-1);
			try {
				auto Sent = socket_.sendBytes(Msg.get(), RTTY_HDR_SIZE + session_length_ + len - 1);
				std::cout << "KeyStrokes: Sent (big buf): " << Sent << std::endl;
				return true;
			} catch (...) {
				return false;
			}
		}
	}

	bool RTTYS_Device_ConnectionHandler::WindowSize(int cols, int rows) {
		if(!valid_)
			return false;

		u_char	outBuf[8+RTTY_SESSION_ID_LENGTH]{0};
		outBuf[0] = msgTypeWinsize;
		outBuf[1] = 0 ;
		outBuf[2] = 4 + session_length_ ;
		memcpy(&outBuf[RTTY_HDR_SIZE],session_id_,session_length_);
		outBuf[RTTY_HDR_SIZE+0+session_length_] = cols >> 8 ;
		outBuf[RTTY_HDR_SIZE+1+session_length_] = cols & 0x00ff;
		outBuf[RTTY_HDR_SIZE+2+session_length_] = rows >> 8;
		outBuf[RTTY_HDR_SIZE+3+session_length_] = rows & 0x00ff;
		try {
			auto Sent = socket_.sendBytes(outBuf, 7 + session_length_ );
			std::cout << "Send WindowSize: " << Sent << std::endl;
			return true;
		} catch (...) {

		}
		return false;
	}

	bool RTTYS_Device_ConnectionHandler::Login() {
		if(!valid_)
			return false;

		u_char outBuf[RTTY_HDR_SIZE+RTTY_SESSION_ID_LENGTH]{0};
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		if(short_session_id_) {
			outBuf[2] = 0;
		} else {
			outBuf[2] = RTTY_SESSION_ID_LENGTH;
			std::strncpy(session_id_,Utils::ComputeHash(id_,token_).substr(0,RTTY_SESSION_ID_LENGTH/2).c_str(),RTTY_SESSION_ID_LENGTH);
			std::cout << "Created session: " << session_id_ << std::endl;
			memcpy(&outBuf[RTTY_HDR_SIZE],session_id_,RTTY_SESSION_ID_LENGTH);
		}
		try {
			auto Sent = socket_.sendBytes( outBuf, RTTY_HDR_SIZE + (short_session_id_ ? 0 : RTTY_SESSION_ID_LENGTH));
			std::cout << "Send Login: " << Sent << std::endl;
		} catch (const Poco::IOException &E) {
			return false;
		} catch (const Poco::Exception &E) {
			return false;
		}
		poco_information(Logger(),fmt::format("{}: Device login", id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::Logout() {
		if(!valid_)
			return false;

		u_char outBuf[4+RTTY_SESSION_ID_LENGTH]{0};
		outBuf[0] = msgTypeLogout;
		outBuf[1] = 0;
		outBuf[2] = session_length_;
		memcpy(&outBuf[3],session_id_,session_length_);
		poco_information(Logger(),fmt::format("{}: Logout", id_));
		try {
			socket_.sendBytes(outBuf, RTTY_HDR_SIZE + session_length_);
			return true;
		} catch (...) {

		}
		return false;
	}

	std::string RTTYS_Device_ConnectionHandler::ReadString() {
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

	bool RTTYS_Device_ConnectionHandler::do_msgTypeRegister([[maybe_unused]] std::size_t msg_len) {
		bool good = true;
		try {
			//	establish if this is an old rtty or a new one.
			short_session_id_ = (inBuf_[0] != 0x03);		//	rtty_proto_ver for full session ID inclusion
			std::cout << "Version: " << (int) (inBuf_[0]) << std::endl;
			if(short_session_id_) {
				session_length_ = 1;
			} else {
				inBuf_.drain(1); //	remove protocol if used.
				session_length_ = RTTY_SESSION_ID_LENGTH;
			}

			id_ = ReadString();
			desc_ = ReadString();
			token_ = ReadString();

			std::cout << "do_msgTypeRegister - token: " << token_ << std::endl;
			std::cout << "do_msgTypeRegister - id: " << id_ << std::endl;

			poco_information(Logger(),
							 fmt::format("{}: Description:{} Device registration", id_, desc_));
			RTTYS_server()->NotifyDeviceRegistration(id_,token_,this);
			u_char OutBuf[8];
			OutBuf[0] = msgTypeRegister;
			OutBuf[1] = 0;		//	Data length
			OutBuf[2] = 4;		//
			OutBuf[3] = 0;		//	Error
			OutBuf[4] = 'O';
			OutBuf[5] = 'K';
			OutBuf[6] = 0;
			if (socket_.sendBytes(OutBuf, 7) != 7) {
					poco_information(Logger(),
									 fmt::format("{}: Description:{} Could not send data to complete registration",
												 id_, desc_));
					good = false;
			}
		} catch (...) {
			good = false;
		}
		return good;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeLogin([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for login", id_));
		nlohmann::json doc;
		char Error;
		std::cout << "do_msgTypeLogin: " << msg_len << std::endl;
		if(short_session_id_) {
			inBuf_.read(&Error, 1);
			inBuf_.read(&session_id_[0], session_length_);
		} else {
			char session[RTTY_SESSION_ID_LENGTH+1]{0};
			inBuf_.read(&session[0], session_length_);
			inBuf_.read(&Error, 1);
			std::cout << "Received session: " << session << "  error:" << (int) Error << std::endl;
		}
		if(short_session_id_)
			std::cout << "Session: " << (int) session_id_[0] << std::endl;
		else
			std::cout << "Session: " << session_id_ << std::endl;

		doc["type"] = "login";
		doc["err"] = Error;
		const auto login_msg = to_string(doc);
		return SendToClient(login_msg);
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeLogout([[maybe_unused]] std::size_t msg_len) {
		char session[RTTY_SESSION_ID_LENGTH];
		if(short_session_id_) {
			inBuf_.read(&session[0],1);
		} else {
			inBuf_.read(&session[0],RTTY_SESSION_ID_LENGTH);
		}
		poco_information(Logger(),fmt::format("{}: Logout", id_));
		return false;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeTermData(std::size_t msg_len) {
		bool good;
		std::cout << "do_msgTypeTermData: " << msg_len << std::endl;
		if(waiting_for_bytes_>0) {
			if(inBuf_.used()<waiting_for_bytes_) {
				waiting_for_bytes_ = waiting_for_bytes_ - inBuf_.used();
				good = SendToClient((unsigned char *)inBuf_.begin(), (int) inBuf_.used());
				if(!good) std::cout << "do_msgTypeTermData:1" << std::endl;
				inBuf_.drain();
			} else {
				good = SendToClient((unsigned char *)inBuf_.begin(), waiting_for_bytes_);
				if(!good) std::cout << "do_msgTypeTermData:2" << std::endl;
				inBuf_.drain(waiting_for_bytes_);
				waiting_for_bytes_ = 0 ;
			}
		} else {
			std::cout << "MSGLEN-1: " << msg_len << std::endl;
			if(short_session_id_) {
				inBuf_.drain(1);
				msg_len -= 1;
			} else {
				inBuf_.drain(RTTY_SESSION_ID_LENGTH);
				msg_len -= RTTY_SESSION_ID_LENGTH;
			}
			std::cout << "MSGLEN-2: " << msg_len << std::endl;
			if(inBuf_.used()<msg_len) {
				std::cout << "Sending (0)" << msg_len << " to client" << std::endl;
				good = SendToClient((unsigned char *)inBuf_.begin(), inBuf_.used());
				if(!good) std::cout << "do_msgTypeTermData:3" << std::endl;
				waiting_for_bytes_ = msg_len - inBuf_.used();
				inBuf_.drain();
			} else {
				waiting_for_bytes_ = 0 ;
				std::cout << "Sending (1)" << msg_len << " to client" << std::endl;
				good = SendToClient((unsigned char *)inBuf_.begin(), msg_len);
				if(!good) std::cout << "do_msgTypeTermData:4" << std::endl;
				inBuf_.drain(msg_len);
			}
		}
		return good;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeWinsize([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeWinsize", id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeCmd([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeCmd", id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeHeartbeat([[maybe_unused]] std::size_t msg_len) {
		u_char MsgBuf[19]{0};
		std::cout << "do_msgTypeHeartbeat: " << msg_len << std::endl;
		if(short_session_id_) {
			MsgBuf[0] = msgTypeHeartbeat;
			MsgBuf[1] = 0;
			MsgBuf[2] = 3;
			auto Sent = socket_.sendBytes(MsgBuf, 3);
			std::cout << "Sent:" << Sent << " l:" << std::endl;
			return Sent == 3;
		} else {
			inBuf_.drain(msg_len);
			MsgBuf[0] = msgTypeHeartbeat;
			MsgBuf[1] = 0;
			MsgBuf[2] = 16;
			unsigned long t = htonl(RTTYS_server()->Uptime());
			MsgBuf[3] = (t & 0xff000000) >> 24;
			MsgBuf[4] = (t & 0x00ff0000) >> 16;
			MsgBuf[5] = (t & 0x0000ff00) >> 8;
			MsgBuf[6] = (t & 0x000000ff);
			auto Sent = socket_.sendBytes(MsgBuf, 16 + 3);
			std::cout << "Sent:" << Sent << " l:" << 19 << std::endl;
			return Sent == 19;
		}
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeFile([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeFile", id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeHttp([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeHttp", id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeAck([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeAck", id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeMax([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeMax", id_));
		return true;
	}
}