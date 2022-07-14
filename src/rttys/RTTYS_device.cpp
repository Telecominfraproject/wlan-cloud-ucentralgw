//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"
#include "rttys/RTTYS_ClientConnection.h"
#include "Poco/Net/SecureStreamSocketImpl.h"
#include "Poco/Net/StreamSocket.h"

namespace OpenWifi {

	RTTYS_Device_ConnectionHandler::RTTYS_Device_ConnectionHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor & reactor):
			 	socket_(socket),
			 	reactor_(reactor),
				Logger_(Poco::Logger::get(fmt::format("RTTY-device({})",socket_.peerAddress().toString())))
	{
		std::thread T([=]() { CompleteConnection(); });
		T.detach();
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
			EndConnection();
		}
	}

	void RTTYS_Device_ConnectionHandler::EndConnection() {
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

				if (registered_) {
					RTTYS_server()->NotifyDeviceDisconnect(Id_,this);
				} else {
					RTTYS_server()->NotifyDeviceFailure(Id_,this);
				}
				poco_information(Logger(), "Connection done.");
				socket_.close();
			}
		} catch (...) {

		}
	}

	void RTTYS_Device_ConnectionHandler::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		bool good = true;

		Guard G(M_);

		try {
			auto received_bytes = socket_.receiveBytes(inBuf_);
			if(received_bytes==0) {
				// std::cout << "No data received" << std::endl;
				return EndConnection();
			}

			// std::cout << "Received: " << received_bytes << std::endl;
			while (inBuf_.isReadable() && good) {
				std::size_t msg_len;
				u_char header[3]{0};
				inBuf_.read((char *)&header[0], 3);
				last_command_ = header[0];
				msg_len = header[1] * 256 + header[2];

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
					default:
						poco_warning(Logger(),fmt::format("{}: Unknown command {}", Id_,
													 (int)last_command_));
				}
			}
		} catch (...) {
			good = false;
		}

		if(!good)
			return EndConnection();
	}

	void RTTYS_Device_ConnectionHandler::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
		Guard G(M_);
		EndConnection();
	}

	bool RTTYS_Device_ConnectionHandler::SendToClient(const u_char *Buf, int Len) {
		return RTTYS_server()->SendToClient(Id_, Buf, Len);
	}

	bool RTTYS_Device_ConnectionHandler::SendToClient(const std::string &S) {
		return RTTYS_server()->SendToClient(Id_, S);
	}

	bool RTTYS_Device_ConnectionHandler::KeyStrokes(const u_char *buf, size_t len) {
		Guard G(M_);

		if(!valid_)
			return false;

		if(len>(RTTY_DEVICE_BUFSIZE-5))
			return false;

		auto total_len = 3 + 1 + len-1;
		scratch_[0] = msgTypeTermData;
		scratch_[1] = (len & 0xff00) >> 8 ;
		scratch_[2] = (len & 0x00ff) ;
		scratch_[3] = sid_;
		memcpy( &scratch_[4], &buf[1], len-1);
		try {
			socket_.sendBytes((const void *)&scratch_[0], total_len);
			return true;
		} catch (...) {
			return false;
		}
	}

	bool RTTYS_Device_ConnectionHandler::WindowSize(int cols, int rows) {
		Guard G(M_);

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

		Guard G(M_);
		u_char outBuf[3]{0};
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		outBuf[2] = 0;
		try {
			socket_.sendBytes(outBuf, 3);
		} catch (const Poco::IOException &E) {
			// std::cout << "1  " << E.what() << " " << E.name() << " "<< E.className() << " "<< E.message() << std::endl;
			return false;
		} catch (const Poco::Exception &E) {
			// std::cout << "2  " << E.what() << " " << E.name() << std::endl;
			return false;
		}
		received_login_from_websocket_ = true;
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
									 fmt::format("{}: Serial:{} Description:{} Could not send data to complete registration",
												 Id_, serial_, desc_));
					good = false;
			} else {
				registered_ = true;
			}
		} catch (...) {
			good = false;
		}
		return good;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeLogin([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Serial:{} Asking for login", Id_, serial_));
		nlohmann::json doc;
		char Error;
		inBuf_.read(&Error, 1);
		inBuf_.read(&sid_, 1);
		doc["type"] = "login";
		doc["err"] = Error;
		const auto login_msg = to_string(doc);
		web_socket_active_ = true ;
		return SendToClient(login_msg);
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeLogout([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Serial:{} Asking for logout", Id_, serial_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeTermData(std::size_t msg_len) {
		bool good = false;
		if(waiting_for_bytes_!=0) {
			auto to_read = std::min(inBuf_.used(),waiting_for_bytes_);
			inBuf_.read(&scratch_[0], to_read);
			good = SendToClient((u_char *)&scratch_[0], (int) to_read);
			if(to_read<waiting_for_bytes_)
				waiting_for_bytes_ -= to_read;
			else
				waiting_for_bytes_ = 0 ;
		} else {
			if(inBuf_.used()<msg_len) {
				auto read_count = inBuf_.read(&scratch_[0], inBuf_.used());
				good = SendToClient((u_char *)&scratch_[0], read_count);
				waiting_for_bytes_ = msg_len - read_count;
			} else {
				inBuf_.read(&scratch_[0], msg_len);
				good = SendToClient((u_char *)&scratch_[0], (int)msg_len);
				waiting_for_bytes_=0;
			}
		}
		return good;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeWinsize([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Serial:{} Asking for msgTypeWinsize", Id_, serial_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeCmd([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Serial:{} Asking for msgTypeCmd", Id_, serial_));
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
		poco_information(Logger(),fmt::format("{}: Serial:{} Asking for msgTypeFile", Id_, serial_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeHttp([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Serial:{} Asking for msgTypeHttp", Id_, serial_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeAck([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Serial:{} Asking for msgTypeAck", Id_, serial_));
		return true;
	}

	bool RTTYS_Device_ConnectionHandler::do_msgTypeMax([[maybe_unused]] std::size_t msg_len) {
		poco_information(Logger(),fmt::format("{}: Serial:{} Asking for msgTypeMax", Id_, serial_));
		return true;
	}
}