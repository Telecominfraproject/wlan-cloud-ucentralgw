//
// Created by stephane bourque on 2021-11-23.
//

#pragma once

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Timer.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_ClientConnection.h"

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

	class RTTYS_EndPoint {
	  public:
		RTTYS_EndPoint(const std::string &Token, const std::string &SerialNumber, const std::string &UserName ):
 			Token_(Token),
			SerialNumber_(SerialNumber),
			UserName_(UserName)
		{
			Created_ = OpenWifi::Now();
		}

		inline void SetClient(std::unique_ptr<RTTYS_ClientConnection> Client) {
			ClientConnected_ = OpenWifi::Now();
			Client_ = std::move(Client);
		}

		inline void SetDevice(const std::string &Token, std::string &serial, std::unique_ptr<RTTYS_Device_ConnectionHandler> Device) {
			DeviceConnected_ = OpenWifi::Now();
			Device_ = std::move(Device);
			serial = SerialNumber_;
			Token_ = Token;
		}

		inline bool Login() {
			if(Device_!= nullptr) {
				return Device_->Login();
			}
			return false;
		}

		RTTYS_EndPoint & operator=(RTTYS_EndPoint Other) {
			Other.Client_ = std::move(Client_);
			Other.Device_ = std::move(Device_);
			return *this;
		}

		inline void DisconnectClient() {
			ClientDisconnected_ = OpenWifi::Now();
		}

		inline void DisconnectDevice() {
			DeviceDisconnected_ = OpenWifi::Now();
		}

		inline void SendClientDisconnection() {
			if(Client_!= nullptr)
				Client_->EndConnection(true);
		}

		inline void SendDeviceDisconnection() {
			if(Device_!= nullptr)
				Device_->EndConnection(true);
		}

		inline bool TooOld() const {
			if(ClientDisconnected_ && (ClientDisconnected_-ClientConnected_)>15)
				return true;
			if(DeviceDisconnected_ && (DeviceDisconnected_-DeviceConnected_)>15)
				return true;
			std::cout << ClientDisconnected_ << " " << ClientConnected_ << " " << DeviceDisconnected_ << " " << DeviceConnected_ << std::endl;
			return false;
		}

		bool CompleteStartup() {
			return Client_->CompleteStartup();
		}

		bool SendToClient(const u_char *Buf, std::size_t Len) {
			if(Client_!= nullptr && Client_->Valid()) {
				Client_->SendData(Buf,Len);
				return true;
			}
			return false;
		}

		inline bool KeyStrokes(const u_char *buffer, std::size_t len) {
			if( Device_!= nullptr )
				return Device_->KeyStrokes(buffer,len);
			return false;
		}

		inline bool WindowSize( int cols, int rows) {
			if(Device_!= nullptr)
				return Device_->WindowSize(cols,rows);
			return false;
		}

		inline bool ValidClient() const {
			return Client_!= nullptr && Client_->Valid();
		}

		inline bool SendToClient(const std::string &S) {
			if(Client_!= nullptr && Client_->Valid()) {
				Client_->SendData(S);
				return true;
			}
			return false;
		}

		[[nodiscard]] inline const std::string & UserName() const { return UserName_; }
		[[nodiscard]] inline const std::string & SerialNumber() const { return SerialNumber_; }

		inline auto TimeDeviceConnected() const { return DeviceDisconnected_ - DeviceConnected_; }
		inline auto TimeClientConnected() const { return ClientDisconnected_ - ClientConnected_; }

	  private:
		std::string 							Token_;
		std::string 							SerialNumber_;
		std::string 							UserName_;
		std::unique_ptr<RTTYS_ClientConnection> 		Client_;
		std::unique_ptr<RTTYS_Device_ConnectionHandler> Device_;
		std::string 							Id_;
		uint64_t 								Created_=0;
		uint64_t 								DeviceDisconnected_ = 0;
		uint64_t 								ClientDisconnected_ = 0;
		uint64_t 								DeviceConnected_ = 0;
		uint64_t 								ClientConnected_ = 0;
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

		bool RegisterDevice(const std::string &Id, const std::string &Token, std::string & serial, RTTYS_Device_ConnectionHandler *Device);
		bool CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber );
		void LoginDone(const std::string & Id);
		bool Login(const std::string & Id_);
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

		std::map<std::string,std::unique_ptr<RTTYS_EndPoint>> 		EndPoints_;			//	id, endpoint
		std::unique_ptr<Poco::Net::HTTPServer>		WebServer_;
		std::unique_ptr<Poco::Net::SocketAcceptor<RTTYS_Device_ConnectionHandler>>	DeviceAcceptor_;
		Poco::Thread								DeviceReactorThread_;
		Poco::NotificationQueue						ResponseQueue_;
		mutable bool 								NotificationManagerRunning_=false;
		Poco::Thread								NotificationManager_;

		Poco::Timer                     					Timer_;
		std::unique_ptr<Poco::TimerCallback<RTTYS_server>>  GCCallBack_;
		std::list<std::unique_ptr<RTTYS_Device_ConnectionHandler>>	FailedDevices;
		MyMutexType 								M_;

		explicit RTTYS_server() noexcept:
		SubSystemServer("RTTY_Server", "RTTY-SVR", "rtty.server")
			{
			}
	};

	inline RTTYS_server * RTTYS_server() { return RTTYS_server::instance(); }

} // namespace OpenWifi
