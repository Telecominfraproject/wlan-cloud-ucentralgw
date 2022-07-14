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
		client_registration,
		device_failure,
		device_registration
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

		RTTYS_Notification(const RTTYS_Notification_type &type,
						   const std::string &id,
						   const std::string &token,
						   RTTYS_Device_ConnectionHandler * device) :
															 type_(type),
															 id_(id),
															 token_(token),
															 device_(device) {
		}

		RTTYS_Notification_type			type_=RTTYS_Notification_type::unknown;
		std::string						id_;
		std::string 					token_;
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
			Created_ = std::chrono::high_resolution_clock::now();
		}

		inline void SetClient(std::unique_ptr<RTTYS_ClientConnection> Client) {
			ClientConnected_ = std::chrono::high_resolution_clock::now();
			Client_ = std::move(Client);
		}

		inline void SetDevice(std::unique_ptr<RTTYS_Device_ConnectionHandler> Device) {
			DeviceConnected_ = std::chrono::high_resolution_clock::now();
			Device_ = std::move(Device);
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
			ClientDisconnected_ = std::chrono::high_resolution_clock::now();
		}

		inline void DisconnectDevice() {
			DeviceDisconnected_ = std::chrono::high_resolution_clock::now();
		}

		[[nodiscard]] inline bool TooOld()  {
			std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
			if(ClientDisconnected_!=std::chrono::time_point<std::chrono::high_resolution_clock>{0s} && (now-ClientDisconnected_)>15s) {
				if(DeviceDisconnected_==std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
					DeviceDisconnected_ = std::chrono::high_resolution_clock::now();
				}
				return true;
			}
			if(DeviceDisconnected_!=std::chrono::time_point<std::chrono::high_resolution_clock>{0s} && (now-DeviceDisconnected_)>15s) {
				if(ClientDisconnected_==std::chrono::time_point<std::chrono::high_resolution_clock>{0s}) {
					ClientDisconnected_ = std::chrono::high_resolution_clock::now();
				}
				return true;
			}
//			std::cout << ClientDisconnected_ << " " << ClientConnected_ << " " << DeviceDisconnected_ << " " << DeviceConnected_ << std::endl;
			return false;
		}

		bool CompleteStartup() {
			if(Client_!= nullptr  && Client_->Valid())
				return Client_->CompleteStartup();
			return false;
		}

		bool SendToClient(const u_char *Buf, std::size_t Len) {
			if(Client_!= nullptr && Client_->Valid()) {
				Client_->SendData(Buf,Len);
				return true;
			}
			return false;
		}

		inline bool KeyStrokes(const u_char *buffer, std::size_t len) {
			if( Device_!= nullptr && Device_->Valid() )
				return Device_->KeyStrokes(buffer,len);
			return false;
		}

		inline bool WindowSize( int cols, int rows) {
			if(Device_!= nullptr && Device_->Valid())
				return Device_->WindowSize(cols,rows);
			return false;
		}

		[[nodiscard]] inline bool ValidClient() const {
			return Client_!= nullptr && Client_->Valid();
		}

		[[nodiscard]] inline bool ValidDevice() const {
			return Device_!= nullptr && Device_->Valid();
		}

		[[nodiscard]] inline bool Joined() const { return Joined_; }
		void Join() { Joined_=true; }

		inline bool SendToClient(const std::string &S) {
			if(Client_!= nullptr && Client_->Valid()) {
				Client_->SendData(S);
				return true;
			}
			return false;
		}

		[[nodiscard]] inline const std::string & UserName() const { return UserName_; }
		[[nodiscard]] inline const std::string & SerialNumber() const { return SerialNumber_; }

		[[nodiscard]] inline auto TimeDeviceConnected() const { return std::chrono::duration<double>{DeviceDisconnected_ - DeviceConnected_}.count(); }
		[[nodiscard]] inline auto TimeClientConnected() const { return std::chrono::duration<double>{ClientDisconnected_ - ClientConnected_}.count(); }

	  private:
		std::string 									Token_;
		std::string 									SerialNumber_;
		std::string 									UserName_;
		std::unique_ptr<RTTYS_ClientConnection> 		Client_;
		std::unique_ptr<RTTYS_Device_ConnectionHandler> Device_;
		std::string 							Id_;
		std::chrono::time_point<std::chrono::high_resolution_clock>
			Created_{0s},DeviceDisconnected_{0s},
			ClientDisconnected_{0s},DeviceConnected_{0s} ,ClientConnected_{0s};
		volatile bool									Joined_=false;
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

		inline void NotifyDeviceRegistration(const std::string &id, const std::string &token, RTTYS_Device_ConnectionHandler *device) {
			ResponseQueue_.enqueueNotification(new RTTYS_Notification(RTTYS_Notification_type::device_registration,id,token,device));
		}

		inline void NotifyClientRegistration(const std::string &id, RTTYS_ClientConnection *client) {
			ResponseQueue_.enqueueNotification(new RTTYS_Notification(RTTYS_Notification_type::client_registration,id,client));
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
