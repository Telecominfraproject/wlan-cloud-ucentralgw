//
// Created by stephane bourque on 2021-11-23.
//

#include "RTTYS_device.h"
#include "rttys/RTTYS_server.h"

namespace OpenWifi {

	RTTY_Device_ConnectionHandler::RTTY_Device_ConnectionHandler(Poco::Net::StreamSocket& socket,
															 Poco::Net::SocketReactor & reactor):
		socket_(socket),
		reactor_(reactor)
	{
		reactor_.addEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ReadableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketReadable));
		reactor_.addEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketShutdown));
	}

	RTTY_Device_ConnectionHandler::~RTTY_Device_ConnectionHandler()
	{
		reactor_.removeEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ReadableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketReadable));
		reactor_.removeEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::WritableNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketWritable));
		reactor_.removeEventHandler(socket_, Poco::NObserver<RTTY_Device_ConnectionHandler, Poco::Net::ShutdownNotification>(*this, &RTTY_Device_ConnectionHandler::onSocketShutdown));

		if(!id_.empty())
			RTTYS_server()->DeRegister(id_,this);
	}

	std::string RTTY_Device_ConnectionHandler::SafeCopy( const u_char * buf, int MaxSize, int & NewPos) {
		std::string     S;
		while(NewPos<MaxSize && buf[NewPos]!=0) {
			S += buf[NewPos++];
		}

		if(buf[NewPos]==0)
			NewPos++;

		return S;
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
		auto total_len = 0 ;
		auto msg_len = 0 ;
		u_char outBuf[ 8192 ];
		outBuf[0] = Type;
		if(sid_.empty()) {
			outBuf[3] = 0;
			std::memcpy(&outBuf[4], Buf, BufLen);
			msg_len = 1 + BufLen ;
			total_len = 4 + BufLen;
		} else {
			std::memcpy(&outBuf[3], sid_.c_str(), sid_.size());
			std::memcpy(&outBuf[3 + sid_.size()], Buf, BufLen);
			msg_len = sid_.size() + BufLen ;
			total_len = 3 + sid_.size() + BufLen;
		}
		outBuf[1] = (msg_len >> 8);
		outBuf[2] = (msg_len & 0x00ff);
		PrintBuf(outBuf,total_len);
		return socket_.sendBytes(outBuf,total_len) == total_len;
	}

	int RTTY_Device_ConnectionHandler::SendMessage(RTTY_MSG_TYPE Type, std::string &S ) {
		u_char outBuf[ 8192 ];
		auto len = S.size() + sid_.size();
		auto msg_len = 0 ;
		auto total_len=0;
		outBuf[0] = Type;
		if(sid_.empty()) {
			outBuf[3] = 0 ;
			std::strcpy((char*)&outBuf[4],S.c_str());
			msg_len = 1 + S.size();
			total_len = 3 + 1 + S.size() + 1;
		} else {
			std::memcpy(&outBuf[3], sid_.c_str(), sid_.size());
			std::strncpy((char*)&outBuf[3+sid_.size()],S.c_str(),S.size());
			total_len = 3 + sid_.size() + S.size();
			msg_len = sid_.size() + S.size() ;
		}
		outBuf[1] = (msg_len >> 8);
		outBuf[2] = (msg_len & 0x00ff);
		PrintBuf(outBuf,total_len);
		return socket_.sendBytes(outBuf,total_len) == total_len;
	}

	int RTTY_Device_ConnectionHandler::SendMessage(RTTY_MSG_TYPE Type) {
		u_char outBuf[ 8192 ];
		auto total_len = 0 ;
		auto msg_len = 0;
		outBuf[0] = Type;
		if(sid_.empty()) {
			outBuf[3] = 0 ;
			total_len = 3 + 1 ;
			msg_len = 1 ;
		} else {
			std::memcpy(&outBuf[3], sid_.c_str(), sid_.size());
			total_len = 3 + sid_.size() ;
			msg_len = sid_.size();
		}
		outBuf[1] = (msg_len >> 8);
		outBuf[2] = (msg_len & 0x00ff);
		return socket_.sendBytes(outBuf,total_len) == 3;
	}

	void RTTY_Device_ConnectionHandler::SendToClient(const u_char *Buf, int len) {
		auto Client = RTTYS_server()->GetClient(id_);
		if(Client!= nullptr)
			Client->SendData(Buf,len);
	}

	void RTTY_Device_ConnectionHandler::SendToDevice(const u_char *buf, int len) {
		if(buf[0]==0) {
			u_char outBuf[8192];
			auto total_len = 0 ;
			auto msg_len = 0;
			outBuf[0] = msgTypeTermData;
			if(sid_.empty()) {
				outBuf[3] = 0 ;
				memcpy( &outBuf[4] , &buf[1], len-1);
				msg_len = 1 + len ;
				total_len = 3 + 1 + len - 1 ;
			} else {
				std::memcpy(&outBuf[3], sid_.c_str(), sid_.size());
				memcpy(&outBuf[3 + sid_.size()], buf, len );
				total_len = 3 + sid_.size() + len ;
				msg_len = sid_.size() + len  ;
			}
			outBuf[1] = (msg_len >> 8);
			outBuf[2] = (msg_len & 0x00ff);
			socket_.sendBytes(outBuf, total_len );
			std::cout << "Sending to device: " << total_len << std::endl;
			PrintBuf(outBuf, total_len);
		}
	}

	void RTTY_Device_ConnectionHandler::WindowSize(int cols, int rows) {
		u_char	outBuf[64];
		outBuf[0] = msgTypeWinsize;
		auto total_len = 0 ;
		auto msg_len = 0;
		if(sid_.empty()) {

		} else {
			outBuf[1] = 0 ;
			outBuf[2] = 32 + 4;
			memcpy(&outBuf[3],sid_.c_str(),32);
			outBuf[35] = cols >> 8 ;
			outBuf[36] = cols & 0x00ff;
			outBuf[37] = rows >> 8;
			outBuf[38] = rows & 0x00ff;
			PrintBuf(outBuf,39);
			socket_.sendBytes(outBuf,39);
		}

	}

	bool RTTY_Device_ConnectionHandler::InitializeConnection( std::string & sid ) {
		sid_ = sid = MicroService::instance().CreateHash(id_).substr(0,32);
		u_char outBuf[64];
		outBuf[0] = msgTypeLogin;
		outBuf[1] = 0;
		outBuf[2] = 33;
		strncpy((char*)&outBuf[3],sid.c_str(),32);
		outBuf[35] = 0 ;
		std::cout << "Initialize device SID" << std::endl;
		PrintBuf(outBuf,35);
		socket_.sendBytes(outBuf,35 );
		return true;
	}

	void RTTY_Device_ConnectionHandler::onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf)
	{
		try
		{
			int len = socket_.receiveBytes(&inBuf_[0],BUF_SIZE);
			if (len > 0)
			{
				RTTY_MSG_TYPE   msg;
				if(inBuf_[0]>=(u_char)msgTypeMax) {
					delete this;
				}
				msg = (RTTY_MSG_TYPE) inBuf_[0];
				int MsgLen = (int) inBuf_[1] * 256 + (int) inBuf_[2];

				switch(msg) {
					case msgTypeRegister: {
						PrintBuf(&inBuf_[0],len);
						proto_ = inBuf_[0];
						int pos=3;
						id_ = SafeCopy(&inBuf_[0],MsgLen,pos);
						desc_ = SafeCopy(&inBuf_[0],MsgLen,pos);
						token_ = SafeCopy(&inBuf_[0],MsgLen,pos);
						std::cout << "msgTypeRegister: id: " << id_ << "  desc: " << desc_ << "  token: " << token_ << std::endl;
						std::string OK{"OK"};
						RTTYS_server()->Register(id_,this);
						SendMessage(msgTypeRegister, OK);
					}
					break;

					case msgTypeLogin: {
						std::cout << "msgTypeLogin: len" << MsgLen << std::endl;
						sid_code_ = inBuf_[3];
					}
					break;

					case msgTypeLogout: {
						std::cout << "msgTypeLogout" << std::endl;

					}
					break;

					case msgTypeTermData: {
						std::cout << "msgTypeTermData: len" << MsgLen << std::endl;
						PrintBuf(&inBuf_[0],len);
						SendToClient(&inBuf_[3],MsgLen);
					}
					break;

					case msgTypeWinsize: {
						std::cout << "msgTypeWinsize" << std::endl;

					}
					break;

					case msgTypeCmd: {
						std::cout << "msgTypeCmd" << std::endl;

					}
					break;

					case msgTypeHeartbeat: {
						std::cout << "msgTypeHeartbeat: " << MsgLen << " bytes" << std::endl;
						PrintBuf(&inBuf_[0], len);
						u_char MsgBuf[32]{0};
						MsgBuf[0] = msgTypeHeartbeat;
/*						MsgBuf[1] = 0 ;
						MsgBuf[2] = 16;
						auto T = std::time(nullptr);
						MsgBuf[3] = T >> 24 ;
						MsgBuf[4] = (T & 0x00ff0000) >> 16;
						MsgBuf[5] = (T & 0x0000ff00) >> 8;
						MsgBuf[6] = (T & 0x000000ff);
*/
						socket_.sendBytes(MsgBuf,3);
					}
					break;

					case msgTypeFile: {
						std::cout << "msgTypeFile" << std::endl;

					}
					break;

					case msgTypeHttp: {
						std::cout << "msgTypeHttp" << std::endl;

					}
					break;

					case msgTypeAck: {
						std::cout << "msgTypeAck" << std::endl;

					}
					break;

					case msgTypeMax: {
						std::cout << "msgTypeMax" << std::endl;
					}
					break;
					}
				}
				else
				{
					std::cout << "Device " << id_ << " no data." << std::endl;
					delete this;
				}
		}
		catch (Poco::Exception& exc)
		{
			delete this;
		}
	}

	void RTTY_Device_ConnectionHandler::onSocketWritable(const Poco::AutoPtr<Poco::Net::WritableNotification>& pNf)
	{
		try
		{
		}
		catch (Poco::Exception& exc)
		{
			delete this;
		}
	}

	void RTTY_Device_ConnectionHandler::onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf)
	{
		std::cout << "Device " << id_ << " closing socket." << std::endl;
		delete this;
	}
}