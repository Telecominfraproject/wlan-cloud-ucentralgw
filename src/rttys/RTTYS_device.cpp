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
			if (MicroService::instance().NoAPISecurity()) {
				poco_information(Logger(),"Unsecured connection.");
			} else {
				auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(socket_.impl());
				while (true) {
					auto V = SS->completeHandshake();
					if (V == 1)
						break;
				}
				if ((SS->secure())) {
					poco_information(Logger(), "Secure connection.");
				}
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
			Guard G(M_);
			EndConnection();
		}
	}

	RTTYS_Device_ConnectionHandler::~RTTYS_Device_ConnectionHandler() {
		if(valid_) {
			Guard G(M_);
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
					RTTYS_server()->NotifyDeviceDisconnect(Id_,this);
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

		Guard G(M_);

		try {
			auto received_bytes = socket_.receiveBytes(inBuf_);
			if (received_bytes == 0) {
				poco_information(Logger(),
								 fmt::format("{}: Connection being closed - 0 bytes received."));
				return EndConnection();
			}

			while (inBuf_.isReadable() && good) {
				uint32_t msg_len = 0;
				if (waiting_for_bytes_ != 0) {

				} else {
					if (inBuf_.used() >= 3) {
						auto *head = (unsigned char *)inBuf_.begin();
						last_command_ = head[0];
						msg_len = head[1] * 256 + head[2];
						inBuf_.drain(3);
					} else {
						good = false;
						if (!good)
							std::cout << "do_msgTypeTermData:5     " << inBuf_.used() << std::endl;
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
								 fmt::format("{}: Unknown command {}. Closing connection.", Id_,
											 (int)last_command_));
					good = false;
				}
				}
			}
		} catch (const Poco::Exception &E) {
			good = false;
			std::cout << "poco::exception in device" << E.what() << " " << E.message() << std::endl;
		} catch (const std::exception &E) {
			good = false;
			std::cout << "std::exception in device" << E.what() << std::endl;
		}

		if(!good) {
			poco_warning(Logger(),
						 fmt::format("{}: Closing connection. Some message did not succeed. CMD={}", Id_,
									 (int)last_command_));
			return EndConnection();
		}
	}

	void RTTYS_Device_ConnectionHandler::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		Guard G(M_);
		poco_information(Logger(),fmt::format("{}: Connection being closed - socket shutdown."));
		EndConnection();
	}

	bool RTTYS_Device_ConnectionHandler::SendToClient(const u_char *Buf, int Len) {
		return RTTYS_server()->SendToClient(Id_, Buf, Len);
	}

	bool RTTYS_Device_ConnectionHandler::SendToClient(const std::string &S) {
		return RTTYS_server()->SendToClient(Id_, S);
	}

	bool RTTYS_Device_ConnectionHandler::KeyStrokes(const u_char *buf, size_t len) {
		if(!valid_)
			return false;

		if(len<=(sizeof(small_buf_)-3)) {
			small_buf_[0] = msgTypeTermData;
			small_buf_[1] = (len & 0xff00) >> 8;
			small_buf_[2] =  (len & 0x00ff);
			memcpy(&small_buf_[3],buf,len);
			try {
				socket_.sendBytes(small_buf_,len+3);
				return true;
			} catch (...) {
				return false;
			}
		} else {
			auto Msg = std::make_unique<unsigned char []>(len + 3);
			Msg.get()[0] = msgTypeTermData;
			Msg.get()[1] = (len & 0xff00) >> 8;
			Msg.get()[2] = (len & 0x00ff);
			memcpy((void *)(Msg.get() + 3), buf, len);
			try {
				socket_.sendBytes(Msg.get(), len + 3);
				return true;
			} catch (...) {
				return false;
			}
		}
/*
		unsigned char Msg[64];
		Msg[0] = msgTypeTermData;
		Msg[1] = (len & 0xff00) >> 8;
		Msg[2] =  (len & 0x00ff);

		Poco::Net::SocketBufVec MsgParts{ 	Poco::Net::SocketBuf{ .iov_base=Msg, .iov_len=3},
											Poco::Net::SocketBuf{ .iov_base=(unsigned char *)buf, .iov_len=len}};
		try {
			socket_.sendBytes(MsgParts);
			return true;
		} catch (...) {

		}
		return false;
*/
	}

	bool RTTYS_Device_ConnectionHandler::WindowSize(int cols, int rows) {
		if(!valid_)
			return false;

		// Guard G(M_);

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
			socket_.sendBytes(outBuf, 8);
			return true;
		} catch (...) {

		}
		return false;
	}

	bool RTTYS_Device_ConnectionHandler::Login() {
		if(!valid_)
			return false;

		// Guard G(M_);
		u_char outBuf[3]{0};
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		outBuf[2] = 0;
		try {
			socket_.sendBytes(outBuf, 3);
		} catch (const Poco::IOException &E) {
			return false;
		} catch (const Poco::Exception &E) {
			return false;
		}
		poco_information(Logger(),fmt::format("{}: Device login", Id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::Logout() {
		if(!valid_)
			return false;

		Guard G(M_);

		u_char outBuf[4]{0};
		outBuf[0] = msgTypeLogout;
		outBuf[1] = 0;
		outBuf[2] = 1;
		outBuf[3] = sid_;
		poco_information(Logger(),fmt::format("{}: Logout", Id_));
		try {
			socket_.sendBytes(outBuf, 4);
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
			Id_ = ReadString();
			desc_ = ReadString();
			token_ = ReadString();

			poco_information(Logger(),
							 fmt::format("{}: Description:{} Device registration", Id_, desc_));
			RTTYS_server()->NotifyDeviceRegistration(Id_,token_,this);
			u_char OutBuf[8];
			OutBuf[0] = msgTypeRegister;
			OutBuf[1] = 0;
			OutBuf[2] = 4;
			OutBuf[3] = 0;
			OutBuf[4] = 'O';
			OutBuf[5] = 'K';
			OutBuf[6] = 0;
			if (socket_.sendBytes(OutBuf, 7) != 7) {
					poco_information(Logger(),
									 fmt::format("{}: Description:{} Could not send data to complete registration",
												 Id_, desc_));
					good = false;
			}
		} catch (...) {
			good = false;
		}
		return good;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeLogin([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for login", Id_));
		nlohmann::json doc;
		char Error;
		inBuf_.read(&Error, 1);
		inBuf_.read(&sid_, 1);
		doc["type"] = "login";
		doc["err"] = Error;
		const auto login_msg = to_string(doc);
		return SendToClient(login_msg);
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeLogout([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for logout", Id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeTermData(std::size_t msg_len) {
		bool good;
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
			if(inBuf_.used()<msg_len) {
				good = SendToClient((unsigned char *)inBuf_.begin(), inBuf_.used());
				if(!good) std::cout << "do_msgTypeTermData:3" << std::endl;
				waiting_for_bytes_ = msg_len - inBuf_.used();
				inBuf_.drain();
			} else {
				waiting_for_bytes_ = 0 ;
				good = SendToClient((unsigned char *)inBuf_.begin(), msg_len);
				if(!good) std::cout << "do_msgTypeTermData:4" << std::endl;
				inBuf_.drain(msg_len);
			}
		}
		return good;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeWinsize([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeWinsize", Id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeCmd([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeCmd", Id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeHeartbeat([[maybe_unused]] std::size_t msg_len) {
		if(!RTTYS_server()->ValidClient(Id_))
			return false;
		u_char MsgBuf[3]{0};
		MsgBuf[0] = msgTypeHeartbeat;
		return socket_.sendBytes(MsgBuf, 3)==3;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeFile([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeFile", Id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeHttp([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeHttp", Id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeAck([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeAck", Id_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeMax([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Asking for msgTypeMax", Id_));
		return true;
	}
}