//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include <shared_mutex>
#include <string>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/NotificationQueue.h"
#include "Poco/Timer.h"

#include "framework/SubSystemServer.h"
#include "framework/utils.h"
#include <fmt/format.h>

using namespace std::chrono_literals;

namespace OpenWifi {

	constexpr uint RTTY_DEVICE_TOKEN_LENGTH = 32;
	constexpr std::size_t RTTY_DEVICE_BUFSIZE = 256000;
	constexpr std::size_t RTTY_SESSION_ID_LENGTH = 32;
	constexpr std::size_t RTTY_HDR_SIZE = 3;
	constexpr std::size_t RTTY_RECEIVE_BUFFER = 64000;

	class RTTYS_server;

	class RTTYS_server *RTTYS_server();

	class RTTYS_EndPoint {
	  public:
		RTTYS_EndPoint(const std::string &Id, const std::string &Token,
					   const std::string &SerialNumber, const std::string &UserName,
					   bool mTLS);

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
			msgTypeMax
		};

		[[nodiscard]] inline bool TooOld() {
			std::chrono::time_point<std::chrono::high_resolution_clock> now =
				std::chrono::high_resolution_clock::now();
			if (ClientDisconnected_ !=
					std::chrono::time_point<std::chrono::high_resolution_clock>{0s} &&
				(now - ClientDisconnected_) > 5s) {
				if (DeviceDisconnected_ ==
					std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
					DeviceDisconnected_ = std::chrono::high_resolution_clock::now();
				}
				return true;
			}
			if (DeviceDisconnected_ !=
					std::chrono::time_point<std::chrono::high_resolution_clock>{0s} &&
				(now - DeviceDisconnected_) > 5s) {
				if (ClientDisconnected_ ==
					std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
					ClientDisconnected_ = std::chrono::high_resolution_clock::now();
				}
				return true;
			}

			return false;
		}

		[[nodiscard]] inline const std::string &UserName() const { return UserName_; }
		[[nodiscard]] inline const std::string &SerialNumber() const { return SerialNumber_; }

		[[nodiscard]] inline auto TimeDeviceConnected() {
			if (DeviceDisconnected_ ==
				std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
				DeviceDisconnected_ = std::chrono::high_resolution_clock::now();
			}
			return std::chrono::duration<double>{DeviceDisconnected_ - DeviceConnected_}.count();
		}

		[[nodiscard]] inline auto TimeClientConnected() {
			if (ClientDisconnected_ ==
				std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
				ClientDisconnected_ = std::chrono::high_resolution_clock::now();
			}
			return std::chrono::duration<double>{ClientDisconnected_ - ClientConnected_}.count();
		}

		[[nodiscard]] std::string ReadString();
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
		[[nodiscard]] bool do_msgTypeRegister(int fd);

		bool SendToClient(const u_char *buf, int len);
		bool SendToClient(const std::string &S);
		[[nodiscard]] bool WindowSize(int cols, int rows);
		[[nodiscard]] bool KeyStrokes(const u_char *buf, size_t len);

		Poco::Net::SocketAddress device_address_;
		std::string Id_;
		std::string Token_;
		std::string SerialNumber_;
		std::string UserName_;
		bool DeviceIsAttached_ = false;
		int 										Device_fd=0;
		std::uint64_t TID_ = 0;
		std::unique_ptr<Poco::Net::WebSocket> 		WSSocket_;
		unsigned char sid_=0;
		unsigned char small_buf_[64 + RTTY_SESSION_ID_LENGTH]{0};
		bool completed_ = false;
		bool mTLS_=false;
		Poco::Net::Socket							DeviceSocket_;
		std::chrono::time_point<std::chrono::high_resolution_clock> Created_{0s},
			DeviceDisconnected_{0s}, ClientDisconnected_{0s}, DeviceConnected_{0s},
			ClientConnected_{0s};
	};

	class RTTYS_server : public SubSystemServer {
	  public:
		static auto instance() {
			static auto instance = new RTTYS_server;
			return instance;
		}

		int Start() final;
		void Stop() final;

		inline auto UIAssets() { return RTTY_UIAssets_; }

		bool CreateEndPoint(const std::string &Id, const std::string &Token,
							const std::string &UserName, const std::string &SerialNumber,
							bool mTLS);

		bool ValidId(const std::string &Id);

		void onTimer(Poco::Timer &timer);

		inline bool UseInternal() const { return Internal_; }

		void onDeviceAccept(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);

		void onConnectedDeviceSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void onConnectedDeviceSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);
		void onConnectedDeviceSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf);

		void onClientSocketReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf);
		void onClientSocketShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf);
		void onClientSocketError(const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf);

		void RemoveClientEventHandlers(Poco::Net::StreamSocket &Socket);
		void RemoveConnectedDeviceEventHandlers(Poco::Net::StreamSocket &Socket);

		void RemoveDeviceEventHandlers(const Poco::Net::Socket &Socket);
		void AddDeviceEventHandlers(Poco::Net::Socket &Socket);

		void AddConnectedDeviceEventHandlers(std::shared_ptr<RTTYS_EndPoint> ep);
		void AddClientEventHandlers(Poco::Net::StreamSocket &Socket,
									std::shared_ptr<RTTYS_EndPoint> EndPoint);

		inline auto Uptime() const { return Utils::Now() - Started_; }

		void CreateWSClient(Poco::Net::HTTPServerRequest &request,
							Poco::Net::HTTPServerResponse &response, const std::string &Id);

		std::map<std::string, std::shared_ptr<RTTYS_EndPoint>>::iterator EndConnection(std::shared_ptr<RTTYS_EndPoint> Connection, std::uint64_t l);
		void EndConnection(const Poco::Net::Socket &Socket, std::uint32_t Line);

		Poco::Net::SocketReactor &Reactor() { return Reactor_; }

		void SendData(std::shared_ptr<RTTYS_EndPoint> &Connection, const u_char *Buf, size_t len);
		void SendData(std::shared_ptr<RTTYS_EndPoint> &Connection, const std::string &s);

		int SendBytes(int fd, const unsigned char *buffer, std::size_t len);
		int SendBytes(const Poco::Net::Socket &Socket, const unsigned char *buffer, std::size_t len);

		inline std::shared_ptr<RTTYS_EndPoint> FindRegisteredEndPoint(const std::string &Id,
															  const std::string &Token) {
			std::lock_guard		G(ServerMutex_);
			std::shared_ptr<RTTYS_EndPoint> Res;
			auto EndPoint = EndPoints_.find(Id);
			if (EndPoint != end(EndPoints_) && EndPoint->second->Token_ == Token) {
				Res = EndPoint->second;
			}
			return Res;
		}

		inline void RemoveRegisteredEndPoint(const std::string &Id,
																	  const std::string &Token) {
			std::lock_guard		G(ServerMutex_);
			auto EndPoint = EndPoints_.find(Id);
			if (EndPoint != end(EndPoints_) && EndPoint->second->Token_ == Token) {
				EndPoints_.erase(Id);
			}
		}

		void AddNewSocket(Poco::Net::Socket &Socket);
		void RemoveSocket(const Poco::Net::StreamSocket &Socket);
		void LogStdException(const std::exception &E, const std::string & msg);

		bool do_msgTypeRegister(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeLogin(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeTermData(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeLogout(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeWinsize(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeCmd(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeHeartbeat(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeFile(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeHttp(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeAck(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);
		bool do_msgTypeMax(const Poco::Net::Socket &Socket, unsigned char *Buffer, std::size_t  BufferCurrentSize, std::size_t  &BufferPos);

		bool WindowSize(std::shared_ptr<RTTYS_EndPoint> Conn, int cols, int rows);
		bool KeyStrokes(std::shared_ptr<RTTYS_EndPoint> Conn, const u_char *buf, size_t len);

		bool Login(const Poco::Net::Socket &Socket, std::shared_ptr<RTTYS_EndPoint> Conn);
		bool Logout(const Poco::Net::Socket &Socket, std::shared_ptr<RTTYS_EndPoint> Conn);

		std::string ReadString(unsigned char *Buffer, std::size_t BufferCurrentSize, std::size_t &BufferPos);
		bool SendToClient(Poco::Net::WebSocket &WebSocket, const u_char *Buf, int len);
		bool SendToClient(Poco::Net::WebSocket &WebSocket, const std::string &s);

		friend class RTTYS_EndPoint;

	  private:
		std::recursive_mutex		ServerMutex_;
		Poco::Net::SocketReactor 	Reactor_;
		Poco::Thread 				ReactorThread_;
		std::string 				RTTY_UIAssets_;
		bool 						Internal_ = false;
		bool 						NoSecurity_ = false;
		volatile bool 				Running_ = false;

		std::unique_ptr<Poco::Net::HTTPServer> 					WebServer_;
		std::map<std::string, std::shared_ptr<RTTYS_EndPoint>> 	EndPoints_; //	id, endpoint
		std::map<int, std::shared_ptr<RTTYS_EndPoint>> 			Connected_; //	id, endpoint
		std::map<int, std::shared_ptr<RTTYS_EndPoint>> 			Clients_;
		std::map<int, Poco::Net::Socket>						Sockets_;

		Poco::Timer Timer_;
		std::unique_ptr<Poco::TimerCallback<RTTYS_server>> GCCallBack_;

		std::atomic_uint64_t TotalEndPoints_ = 0;
		std::chrono::duration<double, std::milli> TotalConnectedDeviceTime_{0ms},
			TotalConnectedClientTime_{0ms};

		std::atomic_uint64_t Started_ = Utils::Now();
		std::atomic_uint64_t MaxConcurrentSessions_ = 0;
		std::unique_ptr<Poco::Net::ServerSocket> 		ServerDeviceSocket_;
		std::unique_ptr<Poco::Net::SecureServerSocket> 	SecureServerDeviceSocket_;
		bool enforce_mTLS_ = false;

		static inline std::uint64_t CurrentTID_ = 0;

		explicit RTTYS_server() noexcept
			: SubSystemServer("RTTY_Server", "RTTY-SVR", "rtty.server") {}
	};

	inline class RTTYS_server *RTTYS_server() { return RTTYS_server::instance(); }

} // namespace OpenWifi
