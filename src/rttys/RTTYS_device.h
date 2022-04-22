//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include <array>
#include "framework/MicroService.h"
#include "Poco/FIFOBuffer.h"

namespace OpenWifi {

inline static const std::size_t RTTY_DEVICE_BUFSIZE=64000;

inline static std::atomic_uint64_t global_device_connection_id = 1;

class RTTY_Device_ConnectionHandler : public Poco::Net::TCPServerConnection {
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

	explicit RTTY_Device_ConnectionHandler(const Poco::Net::StreamSocket & socket) ;
	~RTTY_Device_ConnectionHandler();

	void run() final;
	bool Login();
	bool Logout();
	void Stop();

	void SendToClient(const u_char *buf, int len);
	void SendToClient(const std::string &S);
	bool WindowSize(int cols, int rows);
	bool KeyStrokes(const u_char *buf, size_t len);
	std::string ReadString();
	inline auto SessionID() const { return conn_id_; }
	void AddCommand(u_char C);

  private:
	std::atomic_bool 			  running_=false;
	std::atomic_bool 			  loop_done_=false;
	std::recursive_mutex		  M_;
	Poco::Logger				  &Logger_;
	std::string                   id_;
	std::string                   token_;
	std::string                   desc_;
	std::string 				  serial_;
	char 				          sid_=0;
	Poco::FIFOBuffer  			  inBuf_{64000};
	std::array<char,32000>		  scratch_{0};
	std::size_t      			  waiting_for_bytes_{0};
	u_char 						  last_command_=0;
	uint64_t 					  conn_id_=0;
	std::vector<u_char>			  commands_;

	inline Poco::Logger & Logger() { return Logger_; }

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

	bool ProcessCommands();
};


}
