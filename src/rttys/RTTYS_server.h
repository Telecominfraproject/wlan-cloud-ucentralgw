//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Timer.h"
#include "rttys/RTTYS_device.h"

namespace OpenWifi {

	class RTTYS_Device_ConnectionHandler;
	class RTTYS_ClientConnection;

	class RTTYS_DisconnectNotification: public Poco::Notification {
	  public:
		RTTYS_DisconnectNotification(const std::string &id, bool device) :
				id_(id),
				device_(device) {

		}
		std::string			id_;
		bool				device_=false;
		RTTYS_Device_ConnectionHandler	*DeviceHandler_= nullptr;
		RTTYS_ClientConnection 			*ClientHandler = nullptr;
	};

	class RTTYS_server : public SubSystemServer, Poco::Runnable
	{
	  public:
		static auto instance() {
			static auto instance = new RTTYS_server;
			return instance;
		}

		int Start() final;
		void Stop() final;

		inline auto UIAssets() { return RTTY_UIAssets_; }

		void RegisterClient(const std::string &Id, RTTYS_ClientConnection *Client);
		void DeRegisterClient(const std::string &Id, RTTYS_ClientConnection *Client);
		bool RegisterDevice(const std::string &Id, const std::string &Token, RTTYS_Device_ConnectionHandler *Device);
		void DeRegisterDevice(const std::string &Id, RTTYS_Device_ConnectionHandler *Device, bool remove_websocket);
		bool CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber );
		std::string SerialNumber(const std::string & Id);
		void LoginDone(const std::string & Id);
		bool ValidEndPoint(const std::string &Id, const std::string &Token);
		bool IsDeviceRegistered( const std::string &Id, const std::string &Token, [[maybe_unused]] RTTYS_Device_ConnectionHandler *Conn);
		bool Login(const std::string & Id_);
		bool Logout(const std::string & Id_);
		bool SendKeyStrokes(const std::string &Id, const u_char *buffer, std::size_t s);
		bool WindowSize(const std::string &Id, int cols, int rows);
		bool SendToClient(const std::string &id, const u_char *Buf, std::size_t Len);
		bool SendToClient(const std::string &id, const std::string &s);
		bool ValidClient(const std::string &id);
		bool ValidId(const std::string &Id);
		inline void AddFailedDevice(RTTYS_Device_ConnectionHandler *Device) {
			std::lock_guard	G(M_);
			FailedDevices.push_back(Device);
		}

		void run() final;

		inline void DisconnectNotice(const std::string &id, bool device) {
			std::lock_guard		G(M_);
			ResponseQueue_.enqueueNotification(new RTTYS_DisconnectNotification(id,device));
		}

		struct EndPoint {
			std::string 							Token;
			mutable RTTYS_ClientConnection *		Client = nullptr;
			mutable RTTYS_Device_ConnectionHandler *	Device = nullptr;
			Poco::Net::WebSocket					* WS_ = nullptr;
			uint64_t 								TimeStamp = OpenWifi::Now();
			std::string 							UserName;
			std::string 							SerialNumber;
			mutable uint64_t 						DeviceDisconnected = 0;
			mutable uint64_t 						ClientDisconnected = 0;
			mutable uint64_t 						DeviceConnected = 0;
			mutable uint64_t 						ClientConnected = 0;

		};

		void CreateNewClient(Poco::Net::HTTPServerRequest &request,
							 Poco::Net::HTTPServerResponse &response, const std::string &id);

		void onTimer(Poco::Timer & timer);

		inline bool UseInternal() const {
			return Internal_;
		}

		inline Poco::Net::SocketReactor & ClientReactor() { return ClientReactor_; }

	  private:
		std::recursive_mutex						M_;
		Poco::Net::SocketReactor					ClientReactor_;
		Poco::Net::SocketReactor					DeviceReactor_;
		Poco::Thread								ClientReactorThread_;
		std::string 								RTTY_UIAssets_;
		bool			 							Internal_ = false;

		std::map<std::string, EndPoint> 			EndPoints_;			//	id, endpoint
		std::unique_ptr<Poco::Net::HTTPServer>		WebServer_;
		std::unique_ptr<Poco::Net::SocketAcceptor<RTTYS_Device_ConnectionHandler>>	DeviceAcceptor_;
		Poco::Thread								DeviceReactorThread_;
		Poco::NotificationQueue						ResponseQueue_;
		mutable bool 								NotificationManagerRunning_=false;
		Poco::Thread								NotificationManager_;

		Poco::Timer                     					Timer_;
		std::unique_ptr<Poco::TimerCallback<RTTYS_server>>  GCCallBack_;

		std::list<RTTYS_Device_ConnectionHandler *>	FailedDevices;

		explicit RTTYS_server() noexcept:
		SubSystemServer("RTTY_Server", "RTTY-SVR", "rtty.server")
			{
			}
	};

	inline RTTYS_server * RTTYS_server() { return RTTYS_server::instance(); }

} // namespace OpenWifi
