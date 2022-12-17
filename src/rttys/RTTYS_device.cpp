//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/StreamSocket.h"

#include "nlohmann/json.hpp"
#include "fmt/format.h"

void dump(const u_char *b, uint s) {
	static const char hex[] = "0123456789abcdef";

	int l=0;
	std::cout << std::endl;
	while(s) {
		std::string SS;
		SS += (hex[(*b & 0xf0) >> 4]);
		SS += (hex[(*b & 0x0f)]);
		std::cout << SS << " ";
		l++;
		if((l % 16) == 0)
			std::cout << std::endl;
		b++;
		--s;
	}
	std::cout << std::endl;
}


#define SOCKET_DEBUG(X,Y,Z)	{ std::cout << __func__ << ":" << __LINE__ << std::endl; (Z)=socket_.sendBytes(X,Y); dump(X,Y); }

namespace OpenWifi {

	RTTYS_Device_ConnectionHandler::RTTYS_Device_ConnectionHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor & reactor, std::uint64_t TID):
			 	socket_(socket),
			 	reactor_(reactor),
				Logger_(Poco::Logger::get(fmt::format("RTTY-device({})",socket_.peerAddress().toString()))),
				TID_(TID),
				inBuf_(RTTY_DEVICE_BUFSIZE)
	{
		dev_id_ = ++dev_;
		std::cout << "Device construction: " << dev_id_ << std::endl;
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

			socket_.setBlocking(false);
			socket_.setKeepAlive(true);
			socket_.setNoDelay(true);
			socket_.setReceiveTimeout(Poco::Timespan(60*60,0));

			RegisteredWithReactor_=true;
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
			EndConnection(5);
		}
	}

	RTTYS_Device_ConnectionHandler::~RTTYS_Device_ConnectionHandler() {
		poco_information(Logger_,
			fmt::format("Device {} session ending", id_)
		);
		std::cout << "Device destruction-a: " << dev_id_ << std::endl;
		EndConnection(0);
		std::cout << "Device destruction-b: " << dev_id_ << std::endl;
	}

	void RTTYS_Device_ConnectionHandler::EndConnection(int v) {
		std::cout << "Device EndConnection: " << dev_id_ << "    v:" << v << std::endl;
		if(valid_) {
			std::lock_guard Lock(Mutex_);
			valid_ = false;
			if (RegisteredWithReactor_) {
				RegisteredWithReactor_ = false;
				reactor_.removeEventHandler(
					socket_, Poco::NObserver<RTTYS_Device_ConnectionHandler,
											 Poco::Net::ReadableNotification>(
								 *this, &RTTYS_Device_ConnectionHandler::onSocketReadable));
				reactor_.removeEventHandler(
					socket_, Poco::NObserver<RTTYS_Device_ConnectionHandler,
											 Poco::Net::ShutdownNotification>(
								 *this, &RTTYS_Device_ConnectionHandler::onSocketShutdown));
			}
			deviceIsRegistered_ = false;
			WSClient_.reset();
			if (connected_) {
				connected_ = false;
				std::cout << "Device EndConnection Disconnect A: " << dev_id_ << "    v:" << v
						  << std::endl;
				RTTYS_server()->NotifyDeviceDisconnect(id_);
				std::cout << "Device EndConnection Disconnect B: " << dev_id_ << "    v:" << v
						  << std::endl;
			}
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

		if(!valid_ || !RegisteredWithReactor_)
			return;

		try {

			std::lock_guard		Guard(Mutex_);

			if(!RegisteredWithReactor_)
				return;

			auto received_bytes = socket_.receiveBytes(inBuf_);
			if (received_bytes == 0) {
				poco_information(Logger(), fmt::format("{}: Device Closing connection - 0 bytes received.",id_));
				return EndConnection(1);
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
						continue;
					}
				}

				switch (last_command_) {
					case msgTypeRegister: {
						good = do_msgTypeRegister(msg_len);
					} break;
					case msgTypeLogin: {
						good = do_msgTypeLogin(msg_len);
					} break;
					case msgTypeLogout: {
						good = do_msgTypeLogout(msg_len);
					} break;
					case msgTypeTermData: {
						good = do_msgTypeTermData(msg_len);
					} break;
					case msgTypeWinsize: {
						good = do_msgTypeWinsize(msg_len);
					} break;
					case msgTypeCmd: {
						good = do_msgTypeCmd(msg_len);
					} break;
					case msgTypeHeartbeat: {
						good = do_msgTypeHeartbeat(msg_len);
					} break;
					case msgTypeFile: {
						good = do_msgTypeFile(msg_len);
					} break;
					case msgTypeHttp: {
						good = do_msgTypeHttp(msg_len);
					} break;
					case msgTypeAck: {
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
			poco_warning(Logger(),fmt::format("{}: Exception: {} GW closing connection.", id_, E.what()));
		} catch (const std::exception &E) {
			poco_warning(Logger(),fmt::format("{}: std::exception: {}. GW closing connection.", id_, E.what()));
			good = false;
		}

		if(!good) {
			return EndConnection(2);
		}
	}

	void RTTYS_Device_ConnectionHandler::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		poco_information(Logger(),fmt::format("{}: Connection being closed - socket shutdown.",id_));
		EndConnection(3);
	}

	bool RTTYS_Device_ConnectionHandler::SendToClient(const u_char *Buf, int Len) {
		std::lock_guard		Guard(Mutex_);
		if(WSClient_!= nullptr) {
			u_char bb[64000]{0};
			if (old_rtty_) {
				bb[0] = session_id_[0];
				memcpy(&bb[1], Buf, Len);
			} else {
				bb[0] = 0;
				memcpy(&bb[1], Buf, Len);
			}
			WSClient_->SendData(bb, Len + 1);
			return true;
//			return RTTYS_server()->SendToClient(id_, bb, Len + 1);
		}
		std::cout << "No WS Client set" << std::endl;
		return false;
		// return RTTYS_server()->SendToClient(id_, Buf, Len );
	}

	bool RTTYS_Device_ConnectionHandler::SendToClient(const std::string &S) {
		std::lock_guard		Guard(Mutex_);
		if(!valid_ || WSClient_== nullptr)
			return false;
		WSClient_->SendData(S);
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::KeyStrokes(const u_char *buf, size_t len) {
		std::lock_guard		Guard(Mutex_);

		if(!valid_)
			return false;

		if(len<=(sizeof(small_buf_)-RTTY_HDR_SIZE-session_length_)) {
			small_buf_[0] = msgTypeTermData;
			small_buf_[1] = ((len-1+session_length_) & 0xff00) >> 8;
			small_buf_[2] = ((len-1+session_length_) & 0x00ff);
			memcpy(&small_buf_[RTTY_HDR_SIZE],session_id_,session_length_);
			memcpy(&small_buf_[RTTY_HDR_SIZE+session_length_], &buf[1], len-1);
			try {
				auto Sent = socket_.sendBytes(small_buf_, RTTY_HDR_SIZE + session_length_ + len - 1);
				return (Sent==(int)(RTTY_HDR_SIZE + session_length_ + len - 1));
			} catch (...) {
				return false;
			}
		} else {
			auto Msg = std::make_unique<unsigned char []>(len + RTTY_HDR_SIZE + session_length_);
			Msg.get()[0] = msgTypeTermData;
			Msg.get()[1] = ((len-1+session_length_) & 0xff00) >> 8;
			Msg.get()[2] = ((len-1+session_length_) & 0x00ff);
			memcpy((Msg.get()+RTTY_HDR_SIZE),session_id_,session_length_);
			memcpy((Msg.get()+RTTY_HDR_SIZE+session_length_), &buf[1], len-1);
			try {
				auto Sent = socket_.sendBytes(Msg.get(), RTTY_HDR_SIZE + session_length_ + len - 1);
				return (Sent==(int)( RTTY_HDR_SIZE + session_length_ + len - 1));
			} catch (...) {
				return false;
			}
		}
	}

	bool RTTYS_Device_ConnectionHandler::WindowSize(int cols, int rows) {
		std::lock_guard		Guard(Mutex_);

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
			auto Sent = socket_.sendBytes(outBuf, RTTY_HDR_SIZE + 4 + session_length_ );
			return (Sent==(int)(RTTY_HDR_SIZE + 4 + session_length_));
		} catch (...) {

		}
		return false;
	}

	bool RTTYS_Device_ConnectionHandler::Login() {

		std::lock_guard		Guard(Mutex_);

		if(!valid_)
			return false;

		u_char outBuf[RTTY_HDR_SIZE+RTTY_SESSION_ID_LENGTH]{0};
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		if(old_rtty_) {
			outBuf[2] = 0;
		} else {
			outBuf[2] = RTTY_SESSION_ID_LENGTH;
			std::strncpy(session_id_,Utils::ComputeHash(id_,token_).substr(0,RTTY_SESSION_ID_LENGTH/2).c_str(),RTTY_SESSION_ID_LENGTH);
			memcpy(&outBuf[RTTY_HDR_SIZE],session_id_,RTTY_SESSION_ID_LENGTH);
		}
		try {
			poco_information(Logger(),fmt::format("{}: Device login", id_));
			auto Sent = socket_.sendBytes( outBuf, RTTY_HDR_SIZE + (old_rtty_ ? 0 : RTTY_SESSION_ID_LENGTH));
			return Sent==(int)(RTTY_HDR_SIZE + (old_rtty_ ? 0 : RTTY_SESSION_ID_LENGTH));
		} catch (const Poco::IOException &E) {
			return false;
		} catch (const Poco::Exception &E) {
			return false;
		}
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
			auto Sent = socket_.sendBytes(outBuf, RTTY_HDR_SIZE + session_length_);
			return Sent==(int)(RTTY_HDR_SIZE+session_length_);
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
			old_rtty_ = (inBuf_[0] != 0x03);		//	rtty_proto_ver for full session ID inclusion
			if(old_rtty_) {
				session_length_ = 1;
			} else {
				inBuf_.drain(1); //	remove protocol if used.
				session_length_ = RTTY_SESSION_ID_LENGTH;
			}

			id_ = ReadString();
			desc_ = ReadString();
			token_ = ReadString();

			if(id_.size()!=RTTY_DEVICE_TOKEN_LENGTH || token_.size()!=RTTY_DEVICE_TOKEN_LENGTH || desc_.empty()) {
				return false;
			}

			poco_information(Logger(),
							 fmt::format("{}: Description:{} Device registration", id_, desc_));
			if(!RTTYS_server()->NotifyDeviceRegistration(id_,token_,TID_)) {
				return false;
			}
			connected_=true;
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
			deviceIsRegistered_ = true;
		} catch (...) {
			good = false;
		}
		return good;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeLogin([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for login", id_));

		nlohmann::json doc;
		char Error;
		if(old_rtty_) {
			inBuf_.read(&Error, 1);
			inBuf_.read(&session_id_[0], session_length_);
		} else {
			char session[RTTY_SESSION_ID_LENGTH+1]{0};
			inBuf_.read(&session[0], session_length_);
			inBuf_.read(&Error, 1);
		}
		doc["type"] = "login";
		doc["err"] = Error;
		const auto login_msg = to_string(doc);
		return SendToClient(login_msg);
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeLogout([[maybe_unused]] std::size_t msg_len) {
		char session[RTTY_SESSION_ID_LENGTH];
		if(old_rtty_) {
			inBuf_.read(&session[0],1);
		} else {
			inBuf_.read(&session[0],RTTY_SESSION_ID_LENGTH);
		}
		poco_information(Logger(),fmt::format("{}: Logout", id_));
		return false;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeTermData(std::size_t msg_len) {
		bool good;
		if(waiting_for_bytes_>0) {
			if(inBuf_.used()<waiting_for_bytes_) {
				waiting_for_bytes_ = waiting_for_bytes_ - inBuf_.used();
				good = SendToClient((unsigned char *)inBuf_.begin(), (int) inBuf_.used());
				inBuf_.drain();
			} else {
				good = SendToClient((unsigned char *)inBuf_.begin(), waiting_for_bytes_);
				inBuf_.drain(waiting_for_bytes_);
				waiting_for_bytes_ = 0 ;
			}
		} else {
			if(old_rtty_) {
				inBuf_.drain(1);
				msg_len -= 1;
			} else {
				inBuf_.drain(RTTY_SESSION_ID_LENGTH);
				msg_len -= RTTY_SESSION_ID_LENGTH;
			}
			if(inBuf_.used()<msg_len) {
				good = SendToClient((unsigned char *)inBuf_.begin(), inBuf_.used());
				waiting_for_bytes_ = msg_len - inBuf_.used();
				inBuf_.drain();
			} else {
				waiting_for_bytes_ = 0 ;
				good = SendToClient((unsigned char *)inBuf_.begin(), msg_len);
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
		u_char MsgBuf[RTTY_HDR_SIZE + 16]{0};
		if(msg_len)
			inBuf_.drain(msg_len);
		MsgBuf[0] = msgTypeHeartbeat;
		MsgBuf[1] = 0;
		MsgBuf[2] = 0;
		auto Sent = socket_.sendBytes(MsgBuf, RTTY_HDR_SIZE);
		return Sent == RTTY_HDR_SIZE;
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