//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include <array>

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketNotifier.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/NObserver.h"
#include "Poco/FIFOBuffer.h"

namespace OpenWifi {

inline static const std::size_t RTTY_DEVICE_BUFSIZE=64000;

class RTTY_Device_ConnectionHandler {

  public:
	enum RTTY_MSG_TYPE {
		msgTypeRegister = 0,
		msgTypeLogin,
		msgTypeLogout,
		msgTypeTermData,
		msgTypeWinsize,
		msgTypeCmd,
		msgTypeHeartbeat,
		msgTypeFile,
		msgTypeHttp,
		msgTypeAck,
		msgTypeMax };

	RTTY_Device_ConnectionHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor & reactor);
	~RTTY_Device_ConnectionHandler();

	void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
	void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);

	std::string SafeCopy( const u_char * buf, int MaxSize, int & NewPos);
	void PrintBuf(const u_char * buf, int size);
	int SendMessage( RTTY_MSG_TYPE Type, const u_char * Buf, int len);
	int SendMessage( RTTY_MSG_TYPE Type, std::string &S );
	int SendMessage( RTTY_MSG_TYPE Type);

	bool Login();
	bool Logout();

	void SendToClient(const u_char *buf, int len);
	void SendToClient(const std::string &S);
	void WindowSize(int cols, int rows);
	void KeyStrokes(const u_char *buf, size_t len);
	std::string ReadString();

  private:
	Poco::Logger & Logger() { return Logger_; }
	Poco::Net::StreamSocket       socket_;
	Poco::Net::SocketReactor&     reactor_;
	std::string                   id_;
	std::string                   token_;
	std::string                   desc_;
	std::string 				  serial_;
	char 				          sid_=0;
	Poco::Logger				  &Logger_;
	Poco::FIFOBuffer  			  inBuf_{64000};
	std::array<char,32000>		  scratch_{0};
	std::size_t      			  waiting_for_bytes_{0};
	u_char 						  last_command_=0;

	void do_msgTypeRegister(std::size_t msg_len);
	void do_msgTypeLogin(std::size_t msg_len);
	void do_msgTypeLogout(std::size_t msg_len);
	void do_msgTypeTermData(std::size_t msg_len);
	void do_msgTypeWinsize(std::size_t msg_len);
	void do_msgTypeCmd(std::size_t msg_len);
	void do_msgTypeHeartbeat(std::size_t msg_len);
	void do_msgTypeFile(std::size_t msg_len);
	void do_msgTypeHttp(std::size_t msg_len);
	void do_msgTypeAck(std::size_t msg_len);
	void do_msgTypeMax(std::size_t msg_len);
};


}
