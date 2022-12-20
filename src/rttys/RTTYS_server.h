//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include <shared_mutex>
#include <string>

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Timer.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/WebSocket.h"

#include "framework/SubSystemServer.h"
#include "framework/utils.h"
#include <fmt/format.h>

using namespace std::chrono_literals;

namespace OpenWifi {

	constexpr uint RTTY_DEVICE_TOKEN_LENGTH=32;
	constexpr std::size_t RTTY_DEVICE_BUFSIZE=64000;
	constexpr std::size_t RTTY_SESSION_ID_LENGTH=32;
	constexpr std::size_t RTTY_HDR_SIZE=3;

	class RTTYS_server;

	class RTTYS_server * RTTYS_server();

	class RTTYS_EndPoint {
	  public:
		RTTYS_EndPoint(const std::string &Id, const std::string &Token, const std::string &SerialNumber, const std::string &UserName );
		~RTTYS_EndPoint();

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

			bool Login();
			bool Logout();

		[[nodiscard]] inline bool TooOld()  {
			std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
			if(ClientDisconnected_!=std::chrono::time_point<std::chrono::high_resolution_clock>{0s} && (now-ClientDisconnected_)>5s) {
				if(DeviceDisconnected_==std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
					DeviceDisconnected_ = std::chrono::high_resolution_clock::now();
				}
				return true;
			}
			if(DeviceDisconnected_!=std::chrono::time_point<std::chrono::high_resolution_clock>{0s} && (now-DeviceDisconnected_)>5s) {
				if(ClientDisconnected_==std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
					ClientDisconnected_ = std::chrono::high_resolution_clock::now();
				}
				return true;
			}

			return false;
		}

		[[nodiscard]] inline const std::string & UserName() const { return UserName_; }
		[[nodiscard]] inline const std::string & SerialNumber() const { return SerialNumber_; }

		[[nodiscard]] inline auto TimeDeviceConnected() {
			if(DeviceDisconnected_==std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
				DeviceDisconnected_ = std::chrono::high_resolution_clock::now();
			}
			return std::chrono::duration<double>{DeviceDisconnected_ - DeviceConnected_}.count(); }

		[[nodiscard]] inline auto TimeClientConnected() {
			if(ClientDisconnected_==std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
				ClientDisconnected_ = std::chrono::high_resolution_clock::now();
			}
			return std::chrono::duration<double>{ClientDisconnected_ - ClientConnected_}.count();
		}

		[[nodiscard]] bool do_msgTypeRegister(Poco::Net::Socket &Socket, Poco::Buffer<char> &B, [[maybe_unused]] std::size_t msg_len);
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
		bool SendToClient(const u_char *buf, int len);
		bool SendToClient(const std::string &S);
		[[nodiscard]] bool WindowSize(int cols, int rows);
		[[nodiscard]] bool KeyStrokes(const u_char *buf, size_t len);

		inline Poco::Logger & Logger() { return Logger_; }
		Poco::Net::SocketAddress					device_address_;
		std::string 								Id_;
		std::string 								Token_;
		std::string 								SerialNumber_;
		std::string 								UserName_;
		std::unique_ptr<Poco::Net::StreamSocket>	DeviceSocket_;
		std::unique_ptr<Poco::Net::WebSocket>		WSSocket_;
		Poco::Logger								&Logger_;
		std::unique_ptr<Poco::FIFOBuffer> 			DeviceInBuf_;
		char 				          				session_id_[RTTY_SESSION_ID_LENGTH+1]{0};
		std::uint64_t 								session_length_=1;
		std::size_t      			  				waiting_for_bytes_{0};
		u_char 						  				last_command_=0;
		unsigned char 								small_buf_[64+RTTY_SESSION_ID_LENGTH];
		std::uint64_t 								TID_=0;
		bool										old_rtty_=false;
		bool 										completed_=false;

		std::chrono::time_point<std::chrono::high_resolution_clock>
			Created_{0s},DeviceDisconnected_{0s},
			ClientDisconnected_{0s},DeviceConnected_{0s} ,ClientConnected_{0s};
	};

	class RTTYS_server : public SubSystemServer
	{
	  public:
		static auto instance() {
			static auto instance = new RTTYS_server;
			return instance;
		}

		int Start() final;
		void Stop() final;

		inline auto UIAssets() { return RTTY_UIAssets_; }

		bool CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber );

		bool ValidId(const std::string &Id);

		void onTimer(Poco::Timer & timer);

		inline bool UseInternal() const {
			return Internal_;
		}

		std::string ReadString(unsigned char *buf, int len, int &pos);
		void onDeviceAccept(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);

		void onDeviceSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void onDeviceSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
		void onDeviceSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);

		void onClientSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
		void onClientSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
		void onClientSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);

		void RemoveConnectingDeviceEventHandlers(Poco::Net::StreamSocket &Socket);
		void RemoveClientEventHandlers(Poco::Net::StreamSocket &Socket);
		void RemoveDeviceEventHandlers(Poco::Net::StreamSocket &Socket);

		void AddConnectingDeviceEventHandlers(Poco::Net::StreamSocket &Socket);
		void AddClientEventHandlers(Poco::Net::StreamSocket &Socket, std::shared_ptr<RTTYS_EndPoint> EndPoint);
		void AddDeviceEventHandlers(Poco::Net::StreamSocket &Socket, std::shared_ptr<RTTYS_EndPoint> EndPoint);

		void onConnectingDeviceData(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void onConnectingDeviceShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);
		void onConnectingDeviceError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf);

		bool do_msgTypeRegister(Poco::Net::StreamSocket &Socket, unsigned char *Buffer, int Len);
		bool do_msgTypeHeartbeat(Poco::Net::StreamSocket &Socket);

		void CloseDevice(std::shared_ptr<RTTYS_EndPoint> Device);
		void CloseClient(std::shared_ptr<RTTYS_EndPoint> Client);
		void CloseConnection(std::shared_ptr<RTTYS_EndPoint> Connection);


		inline auto Uptime() const { return Utils::Now() - Started_; }

		void CreateWSClient(
				Poco::Net::HTTPServerRequest &request,
				Poco::Net::HTTPServerResponse &response,
				const std::string &Id);

		Poco::Net::SocketReactor & Reactor() { return Reactor_; }

		void SendData( std::shared_ptr<RTTYS_EndPoint> & Connection, const u_char *Buf, size_t len );
		void SendData( std::shared_ptr<RTTYS_EndPoint> & Connection, const std::string &s) ;

		inline std::shared_ptr<RTTYS_EndPoint> FindConnection(const std::string &Id,const std::string &Token) {
			std::shared_ptr<RTTYS_EndPoint>	Res;
			auto EndPoint = EndPoints_.find(Id);
			if(EndPoint!=end(EndPoints_) && EndPoint->second->Token_==Token) {
				Res = EndPoint->second;
			}
			return Res;
		}

	  private:
		Poco::Net::SocketReactor					Reactor_;
		Poco::Thread								ReactorThread_;
		std::string 								RTTY_UIAssets_;
		bool			 							Internal_ = false;
		bool 										NoSecurity_=false;
		volatile bool 								Running_ = false;

		std::unique_ptr<Poco::Net::HTTPServer>		WebServer_;

		std::map<std::string,std::shared_ptr<RTTYS_EndPoint>> 		EndPoints_;			//	id, endpoint
		std::map<int, std::shared_ptr<RTTYS_EndPoint> >	Connections_;
		std::map<int, std::pair<Poco::Net::StreamSocket,std::chrono::time_point<std::chrono::high_resolution_clock>>>		ConnectingDevices_;

		Poco::Timer                     					Timer_;
		std::unique_ptr<Poco::TimerCallback<RTTYS_server>>  GCCallBack_;
		std::mutex									ServerMutex_;

		std::atomic_uint64_t 						TotalEndPoints_=0;
		std::chrono::duration<double,std::milli>	TotalConnectedDeviceTime_{0ms},
													TotalConnectedClientTime_{0ms};

		std::atomic_uint64_t						Started_=Utils::Now();
		std::atomic_uint64_t						MaxConcurrentSessions_=0;
		std::unique_ptr<Poco::Net::ServerSocket>	DeviceSocket_;
		std::unique_ptr<Poco::Net::SecureServerSocket>	SecureDeviceSocket_;

		static inline std::uint64_t 				CurrentTID_=0;

		explicit RTTYS_server() noexcept:
		SubSystemServer("RTTY_Server", "RTTY-SVR", "rtty.server")
			{
			}
	};

	inline class RTTYS_server * RTTYS_server() { return RTTYS_server::instance(); }

} // namespace OpenWifi
