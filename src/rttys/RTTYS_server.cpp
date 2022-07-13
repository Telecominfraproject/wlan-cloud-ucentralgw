//
// Created by stephane bourque on 2021-11-23.
//

#include "rttys/RTTYS_server.h"
#include "rttys/RTTYS_WebServer.h"
#include "rttys/RTTYS_device.h"
#include "rttys/RTTYS_ClientConnection.h"

namespace OpenWifi {

	int RTTYS_server::Start() {

		Internal_ = MicroService::instance().ConfigGetBool("rtty.internal",false);
		if(Internal_) {
			int DSport = (int) MicroService::instance().ConfigGetInt("rtty.port", 5912);
			int CSport = (int) MicroService::instance().ConfigGetInt("rtty.viewport", 5913);
			RTTY_UIAssets_ = MicroService::instance().ConfigPath("rtty.assets", "$OWGW_ROOT/rtty_ui");

			const auto & CertFileName = MicroService::instance().ConfigPath("openwifi.restapi.host.0.cert");
			const auto & KeyFileName = MicroService::instance().ConfigPath("openwifi.restapi.host.0.key");
			const auto & RootCa = MicroService::instance().ConfigPath("openwifi.restapi.host.0.rootca");

			auto TcpServerParams = new Poco::Net::TCPServerParams();
			TcpServerParams->setMaxThreads(50);
			TcpServerParams->setMaxQueued(100);
			TcpServerParams->setThreadIdleTime(Poco::Timespan(10,0));

			if(MicroService::instance().NoAPISecurity()) {
				Poco::Net::ServerSocket DeviceSocket(DSport, 64);
				DeviceAcceptor_ = std::make_unique<Poco::Net::SocketAcceptor<RTTYS_Device_ConnectionHandler>>(DeviceSocket,DeviceReactor_);
			} else {
				auto DeviceSecureContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE,
																  KeyFileName, CertFileName, "",
																  Poco::Net::Context::VERIFY_RELAXED);
				Poco::Crypto::X509Certificate DeviceRoot(RootCa);
				DeviceSecureContext->addCertificateAuthority(DeviceRoot);
				DeviceSecureContext->disableStatelessSessionResumption();
				DeviceSecureContext->enableSessionCache();
				DeviceSecureContext->setSessionCacheSize(0);
				DeviceSecureContext->setSessionTimeout(10);
				DeviceSecureContext->enableExtendedCertificateVerification(true);
				SSL_CTX *SSLCtxDevice = DeviceSecureContext->sslContext();
				SSL_CTX_dane_enable(SSLCtxDevice);

				Poco::Net::SecureServerSocket DeviceSocket(DSport, 64, DeviceSecureContext);
				DeviceAcceptor_ = std::make_unique<Poco::Net::SocketAcceptor<RTTYS_Device_ConnectionHandler>>(DeviceSocket,DeviceReactor_);
			}
			DeviceReactorThread_.start(DeviceReactor_);
			Utils::SetThreadName(DeviceReactorThread_,"rt:devreactor");

			auto WebServerHttpParams = new Poco::Net::HTTPServerParams;
			WebServerHttpParams->setMaxThreads(50);
			WebServerHttpParams->setMaxQueued(200);
			WebServerHttpParams->setKeepAlive(true);

			if(MicroService::instance().NoAPISecurity()) {
				Poco::Net::ServerSocket ClientSocket(CSport, 64);
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

				Poco::Net::SecureServerSocket ClientSocket(CSport, 64, WebClientSecureContext);
				ClientSocket.setNoDelay(true);
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(new RTTYS_Client_RequestHandlerFactory(Logger()), ClientSocket, WebServerHttpParams);
			};
			WebServer_->start();
			ClientReactorThread_.start(ClientReactor_);
			Utils::SetThreadName(ClientReactorThread_,"rt:clntreactor");
		}

		GCCallBack_ = std::make_unique<Poco::TimerCallback<RTTYS_server>>(*this, &RTTYS_server::onTimer);
		Timer_.setStartInterval(30 * 1000);  // first run in 30 seconds
		Timer_.setPeriodicInterval(20 * 1000);
		Timer_.start(*GCCallBack_);

		NotificationManager_.start(*this);

		return 0;
	}

	void RTTYS_server::Stop() {
		if(Internal_) {
			NotificationManagerRunning_=false;
			ResponseQueue_.wakeUpAll();
			NotificationManager_.wakeUp();
			NotificationManager_.join();
			Timer_.stop();
			WebServer_->stopAll();
			WebServer_->stop();
			DeviceAcceptor_->unregisterAcceptor();
			DeviceReactor_.stop();
			DeviceReactorThread_.join();
			ClientReactor_.stop();
			ClientReactorThread_.join();
		}
	}

	void RTTYS_server::onTimer([[maybe_unused]] Poco::Timer & timer) {
		poco_debug(Logger(),"Removing stale connections.");
		Utils::SetThreadName("rt:janitor");
//		MutexLockerDbg	L(__func__ ,M_);

		MyGuard G(M_);
 		for(auto element=EndPoints_.begin();element!=EndPoints_.end();) {
			if(element->second->TooOld()) {
				// std::cout << element->second.DeviceDisconnected << " " << element->second.DeviceConnected << " "
				//	<< element->second.ClientDisconnected << " " << element->second.ClientConnected << std::endl;
				auto c = fmt::format("Removing {}. Device connection time: {}ms. Client connection time: {}ms",
									 element->first, element->second->TimeDeviceConnected(),
									 element->second->TimeClientConnected());
				Logger().information(c);
				// std::cout << c << std::endl;
				element = EndPoints_.erase(element);
			} else {
				++element;
			}
		}
		FailedDevices.clear();
	}

	void RTTYS_server::CreateNewClient(Poco::Net::HTTPServerRequest &request,
								Poco::Net::HTTPServerResponse &response, const std::string &id) {

		MyGuard G(M_);
		auto ep = EndPoints_.find(id);
		if(ep == EndPoints_.end())
			return;

		auto NewClient = std::make_unique<RTTYS_ClientConnection>(request, response, ClientReactor_, id);
		ep->second->SetClient(std::move(NewClient));
		ep->second->CompleteStartup();
	}

	void RTTYS_server::run() {
		Utils::SetThreadName("rtty-mgr");
		NotificationManagerRunning_ = true;
		Poco::AutoPtr<Poco::Notification> NextNotification(ResponseQueue_.waitDequeueNotification());
		while (NextNotification && NotificationManagerRunning_) {
			auto Notification = dynamic_cast<RTTYS_Notification *>(NextNotification.get());
			if (Notification != nullptr) {
				M_.lock();
				if(Notification->type_==RTTYS_Notification_type::device_failure) {
					auto ptr = std::unique_ptr<RTTYS_Device_ConnectionHandler>{Notification->device_};
					FailedDevices.push_back(std::move(ptr));
					M_.unlock();
				} else {
					auto It = EndPoints_.find(Notification->id_);
					if (It != EndPoints_.end()) {
						switch (Notification->type_) {
						case RTTYS_Notification_type::device_disconnection: {
							It->second->DisconnectDevice();
							M_.unlock();
						} break;
						case RTTYS_Notification_type::client_disconnection: {
							It->second->DisconnectClient();
							M_.unlock();
						} break;
						case RTTYS_Notification_type::device_failure: {
							auto ptr = std::unique_ptr<RTTYS_Device_ConnectionHandler>{Notification->device_};
							FailedDevices.push_back(std::move(ptr));
							M_.unlock();
						} break;
						case RTTYS_Notification_type::unknown: {
							M_.unlock();
						} break;
						};
					} else {
						M_.unlock();
					}
				}
			}
			NextNotification = ResponseQueue_.waitDequeueNotification();
		}
	}

	bool RTTYS_server::RegisterDevice(const std::string &Id, const std::string &Token, std::string & serial, RTTYS_Device_ConnectionHandler *Device) {
		// MutexLockerDbg MM(__func__ ,M_);
		MyGuard 	G(M_);
		auto ep = EndPoints_.find(Id);
		if(ep==EndPoints_.end())
			return false;

		auto d = std::unique_ptr<RTTYS_Device_ConnectionHandler>{Device};
		ep->second->SetDevice( Token, serial, std::move(d));
		return true;
	}

	bool RTTYS_server::SendToClient(const std::string &Id, const u_char *Buf, std::size_t Len) {
		// MutexLockerDbg MM(__func__ ,M_);
		MyGuard 	G(M_);
		try {
			auto It = EndPoints_.find(Id);
			if (It != EndPoints_.end()) {
				return It->second->SendToClient(Buf,Len);
			}
		} catch(const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {

		}
		return false;
	}

	bool RTTYS_server::SendToClient(const std::string &Id, const std::string &s) {
		// MutexLockerDbg MM(__func__ ,M_);
		MyGuard 	G(M_);
		try {
			auto It = EndPoints_.find(Id);
			if (It != EndPoints_.end()) {
				return It->second->SendToClient(s);
			}
		} catch(const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {

		}
		return false;
	}

	bool RTTYS_server::ValidClient(const std::string &Id) {
//		MutexLockerDbg MM(__func__ ,M_);
		MyGuard 	G(M_);
		auto It = EndPoints_.find(Id);
		return It!=EndPoints_.end() && It->second->ValidClient();
	}

	bool RTTYS_server::SendKeyStrokes(const std::string &Id, const u_char *buffer, std::size_t len) {
//		MutexLockerDbg MM(__func__ ,M_);
		MyGuard 	G(M_);

		auto It=EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return false;
		}

		try {
			return It->second->KeyStrokes(buffer, len);
		} catch(const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
		}
		return false;
	}

	bool RTTYS_server::WindowSize(const std::string &Id, int cols, int rows) {
//		MutexLockerDbg MM(__func__ ,M_);
		MyGuard 	G(M_);
		auto It=EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return false;
		}
		try {
			return It->second->WindowSize(cols,rows);
		} catch(const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
		}
		return false;
	}


	bool RTTYS_server::CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber ) {
//		MutexLockerDbg MM(__func__ ,M_);
		MyGuard 	G(M_);

		auto NewEP = std::make_unique<RTTYS_EndPoint>(Token, SerialNumber, UserName );
		EndPoints_[Id] = std::move(NewEP);
		return true;
	}

	bool RTTYS_server::ValidId(const std::string &Token) {
		MyGuard 	G(M_);
		return EndPoints_.find(Token) != EndPoints_.end();
	}

	void RTTYS_server::LoginDone(const std::string & Id) {
		MyGuard 	G(M_);

		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end())
			return;
		Logger().information(fmt::format("User: {}, Serial: {} logged in.",It->second->UserName(), It->second->SerialNumber() ));
	}

	bool RTTYS_server::Login(const std::string & Id) {
//		MutexLockerDbg MM(__func__ ,M_);
		MyGuard 	G(M_);
		auto ep = EndPoints_.find(Id);
		if(ep == EndPoints_.end()) {
			return false;
		}

		try {
			return ep->second->Login();
		} catch(const Poco::Exception &E) {
			Logger().log(E);
		} catch (...) {
		}
		return false;
	}
}