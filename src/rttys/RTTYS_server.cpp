//
// Created by stephane bourque on 2021-11-23.
//

#include "rttys/RTTYS_server.h"
#include "rttys/RTTYS_WebServer.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_ClientConnection.h"

#include "framework/MicroServiceFuncs.h"
#include "fmt/format.h"

namespace OpenWifi {

	int RTTYS_server::Start() {

		Internal_ = MicroServiceConfigGetBool("rtty.internal",false);
		if(Internal_) {
			int DSport = (int) MicroServiceConfigGetInt("rtty.port", 5912);
			int CSport = (int) MicroServiceConfigGetInt("rtty.viewport", 5913);
			RTTY_UIAssets_ = MicroServiceConfigPath("rtty.assets", "$OWGW_ROOT/rtty_ui");
			MaxConcurrentSessions_ = MicroServiceConfigGetInt("rtty.maxsessions",0);

			const auto & CertFileName = MicroServiceConfigPath("openwifi.restapi.host.0.cert","");
			const auto & KeyFileName = MicroServiceConfigPath("openwifi.restapi.host.0.key","");
			const auto & RootCa = MicroServiceConfigPath("openwifi.restapi.host.0.rootca","");

			NoSecurity_ = MicroServiceNoAPISecurity();

			if(NoSecurity_) {
				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, DSport);
				DeviceSocket_ = std::make_unique<Poco::Net::ServerSocket>(SockAddr, 64);
				DeviceReactor_.addEventHandler(*DeviceSocket_ ,
											   Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>
											   (*this, &RTTYS_server::onAccept));
			} else {
				auto DeviceSecureContext = Poco::AutoPtr<Poco::Net::Context>( new Poco::Net::Context(Poco::Net::Context::SERVER_USE,
																  KeyFileName, CertFileName, "",
																  Poco::Net::Context::VERIFY_RELAXED));
				Poco::Crypto::X509Certificate DeviceRoot(RootCa);
				DeviceSecureContext->addCertificateAuthority(DeviceRoot);
				DeviceSecureContext->disableStatelessSessionResumption();
				DeviceSecureContext->enableSessionCache();
				DeviceSecureContext->setSessionCacheSize(0);
				DeviceSecureContext->setSessionTimeout(10);
				DeviceSecureContext->enableExtendedCertificateVerification(true);
				SSL_CTX *SSLCtxDevice = DeviceSecureContext->sslContext();
				SSL_CTX_dane_enable(SSLCtxDevice);

				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, DSport);

				SecureDeviceSocket_ = std::make_unique<Poco::Net::SecureServerSocket>(SockAddr, 64, DeviceSecureContext);
				DeviceReactor_.addEventHandler(*SecureDeviceSocket_ ,
					Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>
					(*this, &RTTYS_server::onAccept));
			}

			DeviceReactorThread_.start(DeviceReactor_);
			Utils::SetThreadName(DeviceReactorThread_,"rt:devreactor");

			auto WebServerHttpParams = new Poco::Net::HTTPServerParams;
			WebServerHttpParams->setMaxThreads(50);
			WebServerHttpParams->setMaxQueued(200);
			WebServerHttpParams->setKeepAlive(true);
			WebServerHttpParams->setName("rt:dispatch");

			if(NoSecurity_) {
				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, CSport);
				Poco::Net::ServerSocket ClientSocket(SockAddr, 64);
				ClientSocket.setNoDelay(true);
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(new RTTYS_Client_RequestHandlerFactory(Logger()), ClientSocket, WebServerHttpParams);
			} else {
				auto WebClientSecureContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE, KeyFileName, CertFileName,
										   "", Poco::Net::Context::VERIFY_RELAXED);
				Poco::Crypto::X509Certificate WebRoot(RootCa);
				WebClientSecureContext->addCertificateAuthority(WebRoot);
				WebClientSecureContext->disableStatelessSessionResumption();
				WebClientSecureContext->enableSessionCache();
				WebClientSecureContext->setSessionCacheSize(0);
				WebClientSecureContext->setSessionTimeout(10);
				WebClientSecureContext->enableExtendedCertificateVerification(true);
				SSL_CTX *SSLCtxClient = WebClientSecureContext->sslContext();
				SSL_CTX_dane_enable(SSLCtxClient);

				Poco::Net::IPAddress Addr(Poco::Net::IPAddress::wildcard(
					Poco::Net::Socket::supportsIPv6() ? Poco::Net::AddressFamily::IPv6
													  : Poco::Net::AddressFamily::IPv4));
				Poco::Net::SocketAddress SockAddr(Addr, CSport);
				Poco::Net::SecureServerSocket ClientSocket(SockAddr, 64, WebClientSecureContext);
				ClientSocket.setNoDelay(true);
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(new RTTYS_Client_RequestHandlerFactory(Logger()), ClientSocket, WebServerHttpParams);
			};
			WebServer_->start();
			ClientReactorThread_.start(ClientReactor_);
			Utils::SetThreadName(ClientReactorThread_,"rt:clntreactor");
		}


		GCCallBack_ = std::make_unique<Poco::TimerCallback<RTTYS_server>>(*this, &RTTYS_server::onTimer);
		Timer_.setStartInterval(30 * 1000);  // first run in 30 seconds
		Timer_.setPeriodicInterval(5 * 1000);
		Timer_.start(*GCCallBack_, MicroServiceTimerPool() );
		NotificationManager_.start(*this);

		return 0;
	}

	void RTTYS_server::Stop() {
		Timer_.stop();
		if(Internal_) {
			NotificationManagerRunning_ = false;
			ResponseQueue_.wakeUpAll();
			NotificationManager_.wakeUp();
			NotificationManager_.join();
			WebServer_->stopAll(true);
			WebServer_->stop();
			ClientReactor_.stop();
			ClientReactorThread_.join();
			DeviceReactor_.removeEventHandler(NoSecurity_ ? *DeviceSocket_ : *SecureDeviceSocket_, Poco::NObserver<RTTYS_server, Poco::Net::ReadableNotification>
											  (*this, &RTTYS_server::onAccept));
			DeviceReactor_.stop();
			DeviceReactorThread_.join();
			NotificationManagerRunning_ = false;
		}
	}

	void RTTYS_server::onAccept(const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {
		Poco::Net::SocketAddress	Client;
		Poco::Net::StreamSocket NewSocket = pNf->socket().impl()->acceptConnection(Client);
		auto TID = ++CurrentTID_;
		auto NewDevice = std::make_shared<RTTYS_Device_ConnectionHandler>(NewSocket, DeviceReactor_, TID);
		NotifyDeviceConnection(NewDevice, TID);
	}

	bool IsTooOld (const std::pair<std::shared_ptr<RTTYS_Device_ConnectionHandler>,std::uint64_t> &p) {
		return ((p.second-Utils::Now())>(5*60));
	}

	void RTTYS_server::onTimer([[maybe_unused]] Poco::Timer & timer) {
		poco_trace(Logger(),"Removing stale connections.");
		Utils::SetThreadName("rt:janitor");
		static auto LastStats = Utils::Now();

		while(!LocalMutex_.try_lock() && NotificationManagerRunning_) {
			std::cout << "Spin lock 2" << std::endl;
			Poco::Thread::yield();
			Poco::Thread::trySleep(100);
		}

		if(!NotificationManagerRunning_)
			return;

		std::cout << "In timeout" << std::endl;
		for(auto element=EndPoints_.begin();element!=EndPoints_.end();) {
			if(element->second->TooOld()) {
				auto c = fmt::format("Removing {}. Serial: {} Device connection time: {}s. Client connection time: {}s",
									 element->first,
									 element->second->SerialNumber(),
									 element->second->TimeDeviceConnected(),
									 element->second->TimeClientConnected());
				Logger().information(c);
				TotalConnectedClientTime_ += element->second->TimeClientConnected();
				TotalConnectedDeviceTime_ += element->second->TimeDeviceConnected();
				std::cout << "Start Erasing connection" << std::endl;
				element = EndPoints_.erase(element);
				std::cout << "Done Erasing connection" << std::endl;
			} else {
				++element;
			}
		}

		std::cout << __LINE__ << std::endl;
		FailedDevices.clear();
		std::cout << __LINE__ << std::endl;
		FailedClients.clear();
		std::cout << __LINE__ << std::endl;
		for(auto device=ConnectingDevices_.begin();device!=ConnectingDevices_.end();) {
			if(device->second.second-Utils::Now() > (5*60)) {
				std::cout << __LINE__ << std::endl;
				// device->second.first->EndConnection(10);
				std::cout << __LINE__ << std::endl;
				device = ConnectingDevices_.erase(device);
				std::cout << __LINE__ << std::endl;
			} else {
				++device;
			}
		}

		LocalMutex_.unlock();
		std::cout << "Out timeout" << std::endl;

		if(Utils::Now()-LastStats>(60*5)) {
			LastStats = Utils::Now();
			Logger().information(fmt::format("Statistics: Total connections:{} Total Device Connection Time: {}s  Total Client Connection Time: {}s Device failures: {} Client failures: {}",
				TotalEndPoints_,
				TotalConnectedDeviceTime_,
				TotalConnectedClientTime_,
				FailedNumDevices_,
				FailedNumClients_));
		}
	}

	void RTTYS_server::CreateNewClient(	Poco::Net::HTTPServerRequest &request,
										Poco::Net::HTTPServerResponse &response,
									   	const std::string &id) {

		auto NewClient = std::make_shared<RTTYS_ClientConnection>(request, response, ClientReactor_, id);
		NotifyClientRegistration(id,NewClient);
	}

	void RTTYS_server::CrossConnect(std::unique_ptr<RTTYS_EndPoint> &Conn) {
		if(Conn->GetClient()!=nullptr) {
			Conn->GetClient()->SetDevice(Conn->GetDevice());
		}

		if(Conn->GetDevice()!= nullptr) {
			Conn->GetDevice()->SetWsClient((Conn->GetClient()));
		}

		if(Conn->GetDevice()!= nullptr && Conn->GetClient()!= nullptr) {
			Conn->Join();
		}
	}

	void RTTYS_server::run() {
		Utils::SetThreadName("rt:manager");
		NotificationManagerRunning_ = true;
		while (NotificationManagerRunning_) {
			Poco::AutoPtr<Poco::Notification> NextNotification = ResponseQueue_.waitDequeueNotification() ;
			auto NotificationPtr = dynamic_cast<RTTYS_Notification *>(NextNotification.get());
			if(!NotificationManagerRunning_ || NotificationPtr==nullptr) {
				break;
			}

			while(!LocalMutex_.try_lock() && NotificationManagerRunning_) {
				Poco::Thread::yield();
				Poco::Thread::trySleep(100);
				std::cout << "Spin lock 1" << std::endl;
			}
			if(!NotificationManagerRunning_) {
				break;
			}

			NotificationDetails Notification = NotificationPtr->Data_;

			auto It = EndPoints_.find(Notification.id_);
			if (It != EndPoints_.end()) {
				switch (Notification.type_) {
				case RTTYS_Notification_type::device_disconnection: {
					It->second->DisconnectDevice();
				} break;
				case RTTYS_Notification_type::client_disconnection: {
					It->second->DisconnectClient();
				} break;
				case RTTYS_Notification_type::device_registration: {
					auto Device = ConnectingDevices_.find(Notification.TID_);
					if(Device!=end(ConnectingDevices_)) {
						//	 set the connection device
						It->second->SetDevice(Device->second.first);
						ConnectingDevices_.erase(Notification.TID_);
						CrossConnect(It->second);
						if (It->second->Joined()) {
							It->second->Login();
						}
					}
				} break;
				case RTTYS_Notification_type::client_registration: {
					It->second->SetClient(Notification.client_);
					CrossConnect(It->second);
					if(It->second->Joined()) {
						It->second->Login();
					}
				} break;
				case RTTYS_Notification_type::device_connection: {
					Notification.device_->CompleteConnection();
				} break;
				case RTTYS_Notification_type::unknown: {
				} break;
				};
			} else {
				if(Notification.type_==RTTYS_Notification_type::device_connection) {
					ConnectingDevices_[Notification.TID_] = std::make_pair(Notification.device_,Utils::Now());
					Notification.device_->CompleteConnection();
				} else if(Notification.type_==RTTYS_Notification_type::device_registration) {
					FailedNumDevices_++;
					FailedDevices.push_back(Notification.device_);
				} else if(Notification.type_==RTTYS_Notification_type::client_registration) {
					FailedNumClients_++;
					FailedClients.push_back(Notification.client_);
				}
			}
			LocalMutex_.unlock();
		}
		NotificationManagerRunning_ = false;
	}

	bool RTTYS_server::CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber ) {
		// std::unique_lock 	Lock(LocalMutex_);

		while(!LocalMutex_.try_lock() && NotificationManagerRunning_) {
			std::cout << "Spin lock 3" << std::endl;
			Poco::Thread::yield();
			Poco::Thread::trySleep(100);
		}

		if(!NotificationManagerRunning_)
			return false;

		if(MaxConcurrentSessions_!=0 && EndPoints_.size()==MaxConcurrentSessions_) {
			LocalMutex_.unlock();
			return false;
		}

		EndPoints_[Id] = std::make_unique<RTTYS_EndPoint>(Token, SerialNumber, UserName );
		++TotalEndPoints_;
		LocalMutex_.unlock();
		return true;
	}

	bool RTTYS_server::ValidId(const std::string &Token) {

		while(!LocalMutex_.try_lock() && NotificationManagerRunning_) {
			Poco::Thread::yield();
			Poco::Thread::trySleep(100);
			std::cout << "Spin lock 4" << std::endl;
		}

		if(!NotificationManagerRunning_)
			return false;

		auto ret = EndPoints_.find(Token) != EndPoints_.end();
		LocalMutex_.unlock();
		return ret;
	}

}