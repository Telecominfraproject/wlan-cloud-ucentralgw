//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include <array>
#include "framework/MicroService.h"
#include "Poco/FIFOBuffer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"

namespace OpenWifi {

	inline static const std::size_t RTTY_DEVICE_BUFSIZE=64000;

	inline static std::atomic_uint64_t global_device_connection_id = 1;

	class RTTY_Device_ConnectionHandler{
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

		explicit RTTY_Device_ConnectionHandler(Poco::Net::StreamSocket& socket, Poco::Net::SocketReactor& reactor);

		~RTTY_Device_ConnectionHandler();

		bool Login();
		bool Logout();

		[[nodiscard]] bool SendToClient(const u_char *buf, int len);
		[[nodiscard]] bool SendToClient(const std::string &S);
		[[nodiscard]] bool WindowSize(int cols, int rows);
		[[nodiscard]] bool KeyStrokes(const u_char *buf, size_t len);
		std::string ReadString();
		inline auto SessionID() const { return conn_id_; }

		void onSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void onSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);

		bool Connected() const { return received_login_from_websocket_; }
		using My_mutex_type = std::mutex;
		using Guard = std::lock_guard<My_mutex_type>;

		void EndConnection(Guard &);
		inline void EndConnection() {
			Guard G(M_);
			EndConnection(G);
		}

		inline bool Valid() {
			std::lock_guard	G(M_);
			return valid_;
		}

	  private:
		Poco::Net::StreamSocket   		socket_;
		Poco::Net::SocketReactor		&reactor_;

		mutable bool 					valid_=false;
		Poco::Net::SocketAddress		device_address_;
		My_mutex_type 		  			M_;
		std::string                   	Id_;
		std::string                   	token_;
		std::string                   	desc_;
		std::string 				  	serial_;
		char 				          	sid_=0;
		mutable bool 					registered_=false;
		mutable bool					web_socket_active_=false;

		Poco::FIFOBuffer 				inBuf_;
		std::array<char,RTTY_DEVICE_BUFSIZE>	scratch_{0};
		std::size_t      			  	waiting_for_bytes_{0};
		u_char 						  	last_command_=0;
		uint64_t 					  	conn_id_=0;
		mutable std::atomic_bool		received_login_from_websocket_=false;

		void CompleteConnection();

		Poco::Logger & Logger();

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
