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

namespace OpenWifi {

#define RTTY_DEVICE_BUFSIZE	64000

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
  private:
	Poco::Logger & Logger() { return Logger_; }
	Poco::Net::StreamSocket       socket_;
	Poco::Net::SocketReactor&     reactor_;
	std::string                   id_;
	std::string                   token_;
	std::string                   desc_;
	std::string 				  serial_;
	int 				          sid_=0;
	Poco::Logger					&Logger_;
	std::array<unsigned char,RTTY_DEVICE_BUFSIZE>	inBuf{0};
};


}
