//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"
#include "Poco/Net/SecureStreamSocketImpl.h"

namespace OpenWifi {

	RTTY_Device_ConnectionHandler::RTTY_Device_ConnectionHandler(Poco::Net::StreamSocket& socket,
															 Poco::Net::SocketReactor & reactor):
		socket_(socket),
		reactor_(reactor),
		Logger_(RTTYS_server()->Logger())
	{
		conn_id_ = global_device_connection_id++;
		try {
			std::cout << conn_id_ << ": Completing connection setup" << std::endl;
			auto SS = dynamic_cast<Poco::Net::SecureStreamSocketImpl *>(socket_.impl());
			while (true) {
				auto V = SS->completeHandshake();
				if (V == 1)
					break;
			}
			socket_.setKeepAlive(true);
			socket_.setNoDelay(true);
			socket_.setReceiveBufferSize(64000);
			reactor_.addEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ReadableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketReadable));
			reactor_.addEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketShutdown));
		} catch (...) {
			std::cout << conn_id_ << ": Exception during connection setup" << std::endl;
		}
	}

	RTTY_Device_ConnectionHandler::~RTTY_Device_ConnectionHandler()
	{
		socket_.close();
		reactor_.removeEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ReadableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketReadable));
		reactor_.removeEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketShutdown));
		if(!id_.empty()) {
			std::cout << conn_id_ << ": Device de-registering during connection" << std::endl;
			RTTYS_server()->DeRegister(id_, this);
			RTTYS_server()->Close(id_);
		} else {
			std::cout << conn_id_ << ": Device could not de-register" << std::endl;
		}
	}

	void RTTY_Device_ConnectionHandler::PrintBuf(const u_char * buf, int size) {

		std::cout << "======================================" << std::endl;
		while(size) {
			std::cout << std::hex << (int) *buf++ << " ";
			size--;
		}
		std::cout << std::endl;
		std::cout << "======================================" << std::endl;
	}

	int RTTY_Device_ConnectionHandler::SendMessage(RTTY_MSG_TYPE Type, const u_char * Buf, int BufLen) {
		u_char outBuf[ 256 ]{0};
		auto msg_len = BufLen + 1 ;
		auto total_len = msg_len + 3 ;
		outBuf[0] = Type;
		outBuf[1] = (msg_len >> 8);
		outBuf[2] = (msg_len & 0x00ff);
		outBuf[3] = sid_;
		std::memcpy(&outBuf[4], Buf, BufLen);
		// PrintBuf(outBuf,total_len);
		return socket_.sendBytes(outBuf,total_len) == total_len;
	}

	int RTTY_Device_ConnectionHandler::SendMessage(RTTY_MSG_TYPE Type, std::string &S ) {
		u_char outBuf[ 256 ]{0};
		int msg_len = S.size()+1 ;
		int total_len= msg_len+3;
		outBuf[0] = Type;
		outBuf[1] = (msg_len >> 8);
		outBuf[2] = (msg_len & 0x00ff);
		outBuf[3] = sid_;
		std::strcpy((char*)&outBuf[4],S.c_str());
		// PrintBuf(outBuf,total_len);
		return socket_.sendBytes(outBuf,total_len) == total_len;
	}

	int RTTY_Device_ConnectionHandler::SendMessage(RTTY_MSG_TYPE Type) {
		u_char outBuf[ 256 ]{0};
		auto msg_len = 0 ;
		auto total_len= msg_len+3+1;
		outBuf[0] = Type;
		outBuf[1] = (msg_len >> 8);
		outBuf[2] = (msg_len & 0x00ff);
		outBuf[3] = sid_;
		return socket_.sendBytes(outBuf,total_len) == total_len;
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
		u_char outBuf[16]{0};

		if(len>(sizeof(outBuf)-5))
			return;

		auto total_len = 3 + 1 + len-1;
		outBuf[0] = msgTypeTermData;
		outBuf[1] = 0 ;
		outBuf[2] = len +1-1;
		outBuf[3] = sid_;
		memcpy( &outBuf[4], &buf[1], len-1);
		socket_.sendBytes(outBuf, total_len);
	}

	void RTTY_Device_ConnectionHandler::WindowSize(int cols, int rows) {
		u_char	outBuf[32]{0};
		outBuf[0] = msgTypeWinsize;
		outBuf[1] = 0 ;
		outBuf[2] = 4 + 1 ;
		outBuf[3] = sid_;
		outBuf[4] = cols >> 8 ;
		outBuf[5] = cols & 0x00ff;
		outBuf[6] = rows >> 8;
		outBuf[7] = rows & 0x00ff;
		socket_.sendBytes(outBuf,8);
	}

	bool RTTY_Device_ConnectionHandler::Login() {
		u_char outBuf[3]{0};
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		outBuf[2] = 0;
		socket_.sendBytes(outBuf,3 );
		Logger().debug(fmt::format("Device {} login", id_));
		return true;
	}

	bool RTTY_Device_ConnectionHandler::Logout() {
		u_char outBuf[4]{0};
		outBuf[0] = msgTypeLogout;
		outBuf[1] = 0;
		outBuf[2] = 1;
		outBuf[3] = sid_;
		Logger().debug(fmt::format("Device {} logout", id_));
		socket_.sendBytes(outBuf,4 );
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
		socket_.receiveBytes(inBuf_);
		id_ = ReadString();
		desc_ = ReadString();
		token_ = ReadString();
		std::cout << conn_id_ << ": Device Registration ID:" << id_ << " DESC:" << desc_ << " TOK:"
				  << token_ << std::endl;
		if (RTTYS_server()->ValidEndPoint(id_, token_)) {
			if (RTTYS_server()->Register(id_, token_, this)) {
				serial_ = RTTYS_server()->SerialNumber(id_);
				Logger().debug(fmt::format("{}: Registration for SerialNumber: {}, Description: {}",
										   conn_id_, serial_, desc_));
			} else {
				return;
			}
			u_char OutBuf[12];
			OutBuf[0] = msgTypeRegister;
			OutBuf[1] = 0;
			OutBuf[2] = 4;
			OutBuf[3] = 0;
			OutBuf[4] = 'O';
			OutBuf[5] = 'K';
			OutBuf[6] = 0;
			socket_.sendBytes(OutBuf, 7);
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
		socket_.sendBytes(MsgBuf, 3);
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

	void RTTY_Device_ConnectionHandler::onSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf)
	{
		try
		{
			int received = socket_.receiveBytes(inBuf_);
			while(!inBuf_.isEmpty()) {
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
				case msgTypeRegister:
					return do_msgTypeRegister(msg_len);
				case msgTypeLogin:
					return do_msgTypeLogin(msg_len);
				case msgTypeLogout:
					return do_msgTypeLogout(msg_len);
				case msgTypeTermData:
					return do_msgTypeTermData(msg_len);
				case msgTypeWinsize:
					return do_msgTypeWinsize(msg_len);
				case msgTypeCmd:
					return do_msgTypeCmd(msg_len);
				case msgTypeHeartbeat:
					return do_msgTypeHeartbeat(msg_len);
				case msgTypeFile:
					return do_msgTypeFile(msg_len);
				case msgTypeHttp:
					return do_msgTypeHttp(msg_len);
				case msgTypeAck:
					return do_msgTypeAck(msg_len);
				case msgTypeMax:
					return do_msgTypeMax(msg_len);
				default:
					std::cout << conn_id_ << ": Unknown command: " << (int)last_command_ << std::endl;
				}
			}
		}
		catch (const Poco::Exception & E)
		{
			std::cout << conn_id_ << ": EXC: " << E.what() << std::endl;
			Logger().debug(fmt::format("{} :DeRegistration: {} exception, session {}.", conn_id_, serial_, id_));
			Logger().log(E);
			return delete this;
		}
		catch (...) {
			std::cout << conn_id_ << ": Fatal exception" << std::endl;
			return delete this;
		}
	}

	void RTTY_Device_ConnectionHandler::onSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf)
	{
		std::cout << conn_id_ << ": Device " << id_ << " closing socket." << std::endl;
		Logger().debug(fmt::format("{}: Socket closed by device {}.", conn_id_, serial_));
		delete this;
	}
}