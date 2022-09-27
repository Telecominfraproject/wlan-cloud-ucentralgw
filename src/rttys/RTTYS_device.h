//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include <array>
#include <shared_mutex>
#include "framework/MicroService.h"
#include "Poco/FIFOBuffer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"

namespace OpenWifi {

	constexpr std::size_t RTTY_DEVICE_BUFSIZE=64000;
	constexpr std::size_t RTTY_SESSION_ID_LENGTH=32;
	constexpr std::size_t RTTY_HDR_SIZE=3;

	class RTTYS_Device_ConnectionHandler{
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

		explicit RTTYS_Device_ConnectionHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor);
		~RTTYS_Device_ConnectionHandler();

		bool Login();
		bool Logout();

		[[nodiscard]] bool SendToClient(const u_char *buf, int len);
		[[nodiscard]] bool SendToClient(const std::string &S);
		[[nodiscard]] bool WindowSize(int cols, int rows);
		[[nodiscard]] bool KeyStrokes(const u_char *buf, size_t len);
		std::string ReadString();
		// inline auto SessionID() const { return conn_id_; }

		void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);

		inline Poco::Logger	&Logger() { return Logger_; }
		inline bool Valid() { return valid_; }

	  private:
		Poco::Net::StreamSocket   		socket_;
		Poco::Net::SocketReactor		&reactor_;
		Poco::FIFOBuffer 				inBuf_{RTTY_DEVICE_BUFSIZE};
		Poco::Logger					&Logger_;

		std::atomic_bool				valid_=false;
		bool 							old_rtty_=true;
		Poco::Net::SocketAddress		device_address_;
		std::shared_mutex 		  		M_;
		std::string                   	id_;
		std::string                   	token_;
		std::string                   	desc_;
		char 				          	session_id_[RTTY_SESSION_ID_LENGTH+1]{0};
		std::uint64_t 					session_length_=1;
		std::size_t      			  	waiting_for_bytes_{0};
		u_char 						  	last_command_=0;
		unsigned char 					small_buf_[64+RTTY_SESSION_ID_LENGTH];

		void EndConnection() ;
		void CompleteConnection();

		[[nodiscard]] bool do_msgTypeRegister(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeLogin(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeLogout(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeTermData(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeWinsize(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeCmd(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeHeartbeat(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeFile(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeHttp(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeAck(std::size_t msg_len);
		[[nodiscard]] bool do_msgTypeMax(std::size_t msg_len);
	};
}
