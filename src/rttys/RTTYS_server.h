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

	template <typename T> class MutexLockerDbg {
	  public:
		MutexLockerDbg(const std::string &name, T &L) :
 			name_(name),
 			L_(L)
		{
			std::cout << name_ << ":L:0:" << Poco::Thread::current()->name() << ":" << Poco::Thread::currentTid() << std::endl;
			L_.lock();
			std::cout << name_ << ":L:1:" << Poco::Thread::current()->name() << ":" << Poco::Thread::currentTid() << std::endl;
		}

		~MutexLockerDbg() {
			std::cout << name_ << ":U:0:" << Poco::Thread::current()->name() << ":" << Poco::Thread::currentTid() << std::endl;
			L_.unlock();
			std::cout << name_ << ":U:1:" << Poco::Thread::current()->name() << ":" << Poco::Thread::currentTid() << std::endl;
		}

	  private:
		std::string name_;
		T & L_;
	};

	enum class RTTYS_Notification_type {
		unknown,
		device_disconnection,
		client_disconnection,
		device_failure
	};

	class RTTYS_Notification: public Poco::Notification {
	  public:
		RTTYS_Notification(const RTTYS_Notification_type &type, const std::string &id,
						   RTTYS_Device_ConnectionHandler * device) :
		   	type_(type),
	   		id_(id),
			device_(device) {
		}

		RTTYS_Notification(const RTTYS_Notification_type &type, const std::string &id,
						   RTTYS_ClientConnection * client) :
			type_(type),
			id_(id),
		 	client_(client) {
		}

		RTTYS_Notification_type			type_=RTTYS_Notification_type::unknown;
		std::string						id_;
		RTTYS_Device_ConnectionHandler	*device_= nullptr;
		RTTYS_ClientConnection 			*client_ = nullptr;
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
		bool RegisterDevice(const std::string &Id, const std::string &Token, std::string & serial, RTTYS_Device_ConnectionHandler *Device);
		void DeRegisterDevice(const std::string &Id, RTTYS_Device_ConnectionHandler *Device, bool remove_websocket);
		bool CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber );
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

		using MyMutexType = std::recursive_mutex;
		using MyGuard = std::lock_guard<MyMutexType>;
		using MyUniqueLock = std::unique_lock<MyMutexType>;

		void run() final;

		inline void NotifyDeviceDisconnect(const std::string &id, RTTYS_Device_ConnectionHandler *device) {
			ResponseQueue_.enqueueNotification(new RTTYS_Notification(RTTYS_Notification_type::device_disconnection,id,device));
		}

		inline void NotifyDeviceFailure(const std::string &id, RTTYS_Device_ConnectionHandler *device) {
			ResponseQueue_.enqueueNotification(new RTTYS_Notification(RTTYS_Notification_type::device_failure,id,device));
		}

		inline void NotifyClientDisconnect(const std::string &id, RTTYS_ClientConnection *client) {
			ResponseQueue_.enqueueNotification(new RTTYS_Notification(RTTYS_Notification_type::client_disconnection,id,client));
		}

		struct EndPoint {
			std::string 							Token;
			RTTYS_ClientConnection 					*Client=nullptr;
			RTTYS_Device_ConnectionHandler 			*Device=nullptr;
			Poco::Net::WebSocket				    *WS_=nullptr;
			uint64_t 								TimeStamp = OpenWifi::Now();
			std::string 							UserName;
			std::string 							SerialNumber;
			uint64_t 								DeviceDisconnected = 0;
			uint64_t 								ClientDisconnected = 0;
			uint64_t 								DeviceConnected = 0;
			uint64_t 								ClientConnected = 0;
		};

		void CreateNewClient(Poco::Net::HTTPServerRequest &request,
							 Poco::Net::HTTPServerResponse &response, const std::string &id);

		void onTimer(Poco::Timer & timer);

		inline bool UseInternal() const {
			return Internal_;
		}

		inline Poco::Net::SocketReactor & ClientReactor() { return ClientReactor_; }

	  private:
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
		MyMutexType 								M_;

		explicit RTTYS_server() noexcept:
		SubSystemServer("RTTY_Server", "RTTY-SVR", "rtty.server")
			{
			}
	};

	inline RTTYS_server * RTTYS_server() { return RTTYS_server::instance(); }

} // namespace OpenWifi
