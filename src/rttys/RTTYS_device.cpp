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
			RTTYS_server()->instance()->DeRegister(id_,this);
	}

	std::string RTTY_Device_ConnectionHandler::SafeCopy( const u_char * buf, int MaxSize, int & NewPos) {
		std::string     S;
		std::cout << __LINE__ << std::endl;

		while(NewPos<MaxSize && buf[NewPos]!=0) {
			// std::cout << __LINE__ << std::endl;
			S += buf[NewPos++];
		}

		if(buf[NewPos]==0)
			NewPos++;

		return S;
	}

	void RTTY_Device_ConnectionHandler::PrintBuf(const u_char * buf, int size) {

		while(size) {
			std::cout << std::hex << (int) *buf++ << " ";
			size--;
		}
		std::cout << std::endl;
	}

	int RTTY_Device_ConnectionHandler::SendMessage( Poco::Net::StreamSocket & Socket, int Type, const u_char * Buf, int len) {
		u_char outBuf[ 8192 ];
		outBuf[0] = Type;
		std::memcpy(&outBuf[3], Buf, len);
		outBuf[1] = (len >> 8);
		outBuf[2] = (len & 0x00ff);
		return Socket.sendBytes(&outBuf[0],len+3) == len+3;
	}

	int RTTY_Device_ConnectionHandler::SendMessage( Poco::Net::StreamSocket & Socket, int Type, std::string &S ) {
		u_char outBuf[ 8192 ];
		outBuf[0] = Type;
		auto len = S.size();
		std::memcpy(&outBuf[3], S.c_str(), len);
		len++;
		outBuf[1] = (len >> 8);
		outBuf[2] = (len & 0x00ff);
		outBuf[len+3] = 0 ;
		return Socket.sendBytes(&outBuf[0],len+3) == len+3;
	}

	int RTTY_Device_ConnectionHandler::SendMessage( Poco::Net::StreamSocket & Socket, int Type) {
		u_char outBuf[ 8192 ];
		outBuf[0] = Type;
		outBuf[1] = 0;
		outBuf[2] = 0;
		return Socket.sendBytes(&outBuf[0],3) == 3;
	}

	void RTTY_Device_ConnectionHandler::SendToClient(const u_char *Buf, int len) {
		auto Client = RTTYS_server()->instance()->GetClient(id_);
		if(Client!= nullptr)
			Client->SendData(Buf,len);
	}

	void RTTY_Device_ConnectionHandler::SendToDevice(const u_char *buf, int len) {
		socket_.sendBytes(buf, len);
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
					PrintBuf(&inBuf_[1],len);
					proto_ = inBuf_[0];
					int pos=3;
					id_ = SafeCopy(&inBuf_[0],MsgLen,pos);
					desc_ = SafeCopy(&inBuf_[0],MsgLen,pos);
					token_ = SafeCopy(&inBuf_[0],MsgLen,pos);
					std::cout << "msgTypeRegister: id: " << id_ << "  desc: " << desc_ << "  token: " << token_ << std::endl;
					u_char  outBuf[7];
					outBuf[0] = 0;
					outBuf[1] = 'O' ;
					outBuf[2] = 'K' ;
					outBuf[3] = 0;
					SendMessage(socket_, msgTypeRegister, &outBuf[0], 4);
					RTTYS_server()->instance()->Register(id_,this);
				}
				break;

				case msgTypeLogin: {
					std::cout << "msgTypeLogin" << std::endl;
					if(MsgLen<33) {
						std::cout << "Illegal login..." << std::endl;
					}
					memcpy(&sid_[0],&inBuf_[1],32);
					sid_[32] = 0 ;
					sid_code_ = inBuf_[32];
				}
				break;

				case msgTypeLogout: {
					std::cout << "msgTypeLogout" << std::endl;

				}
				break;

				case msgTypeTermData: {
					if(MsgLen<32) {
						std::cout << " device - bad data msg len" << std::endl;
						return;
					}
					std::cout << "msgTypeTermData" << std::endl;
					memcpy(&sid_[0],&inBuf_[1],32);
					sid_[32] = 0 ;
					SendToClient(&inBuf_[32+3],MsgLen-35);
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
					std::cout << "msgTypeHeartbeat" << std::endl;
					SendMessage(socket_,msgTypeHeartbeat);
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
		delete this;
	}
}