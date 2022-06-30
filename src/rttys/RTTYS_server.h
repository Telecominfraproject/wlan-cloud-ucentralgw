//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Timer.h"

namespace OpenWifi {

	class RTTY_Device_ConnectionHandler;
	class RTTYS_ClientConnection;

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

		void Register(const std::string &Id, RTTYS_ClientConnection *Client);
		void DeRegister(const std::string &Id, RTTYS_ClientConnection *Client);
		bool Register(const std::string &Id, const std::string &Token, RTTY_Device_ConnectionHandler *Device);
		void DeRegisterDevice(const std::string &Id, RTTY_Device_ConnectionHandler *Device);
		bool CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber );
		std::string SerialNumber(const std::string & Id);
		void LoginDone(const std::string & Id);
		bool ValidEndPoint(const std::string &Id, const std::string &Token);
		bool CanConnect( const std::string &Id, RTTYS_ClientConnection *Conn);
		bool IsDeviceRegistered( const std::string &Id, const std::string &Token, [[maybe_unused]] RTTY_Device_ConnectionHandler *Conn);
		bool Login(const std::string & Id_);
		bool Logout(const std::string & Id_);
		bool Close(const std::string & Id_);
		uint64_t DeviceSessionID(const std::string & Id);
		bool SendKeyStrokes(const std::string &Id, const u_char *buffer, std::size_t s);
		bool WindowSize(const std::string &Id, int cols, int rows);
		bool SendToClient(const std::string &id, const u_char *Buf, std::size_t Len);
		bool SendToClient(const std::string &id, const std::string &s);
		bool ValidId(const std::string &Id);

		struct EndPoint {
			std::string 							Token;
			mutable RTTYS_ClientConnection *		Client = nullptr;
			mutable RTTY_Device_ConnectionHandler *	Device = nullptr;
			uint64_t 								TimeStamp = OpenWifi::Now();
			mutable uint64_t 								DeviceConnected = 0;
			mutable uint64_t 								ClientConnected = 0;
			std::string 							UserName;
			std::string 							SerialNumber;
			mutable bool 									ShuttingDown = false;
			mutable bool 									ShutdownComplete = false;
		};

		void onTimer(Poco::Timer & timer);

		inline bool UseInternal() const {
			return Internal_;
		}

		inline void dump([[maybe_unused]] const char *ID, [[maybe_unused]] std::ostream &s) {
/*			for(const auto &[id,point]:EndPoints_) {
				s << ID << "  ID: " << id << "  C:" << (point.Client == nullptr) << "  D:" << (point.Device== nullptr)
				  << " Shutting down: " << point.ShuttingDown
				  << " Shutdown: " << point.ShutdownComplete << std::endl;
			}
*/
		}

	  private:
		// std::recursive_mutex		M_;
		Poco::Net::SocketReactor	ClientReactor_;
		Poco::Thread				ClientReactorThread_;
		std::string 				RTTY_UIAssets_;
		std::atomic_bool 			Internal_ = false;

		std::map<std::string, EndPoint> 			EndPoints_;			//	id, endpoint
		std::unique_ptr<Poco::Net::HTTPServer>		WebServer_;
		std::unique_ptr<Poco::Net::TCPServer>		DeviceAcceptor_;

		Poco::Timer                     					Timer_;
		std::unique_ptr<Poco::TimerCallback<RTTYS_server>>  GCCallBack_;

		explicit RTTYS_server() noexcept:
		SubSystemServer("RTTY_Server", "RTTY-SVR", "rtty.server")
			{
			}
	};

	inline RTTYS_server * RTTYS_server() { return RTTYS_server::instance(); }

} // namespace OpenWifi
