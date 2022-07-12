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
		auto now = OpenWifi::Now();
		MyGuard 	G(M_);
 		for(auto element=EndPoints_.begin();element!=EndPoints_.end();) {
			if(element->second.Client!=nullptr && !element->second.Client->Valid() && (now-element->second.ClientDisconnected)>15) {
				// std::cout << "Removing client:" << element->first << std::endl;
				delete element->second.Client;
				delete element->second.WS_;
				element->second.Client = nullptr;
				element->second.WS_ = nullptr;
			}

			if(element->second.Device!=nullptr && !element->second.Device->Valid() && (now-element->second.DeviceDisconnected)>15) {
				// std::cout << "Removing device:" << element->first << std::endl;
				delete element->second.Device;
				element->second.Device = nullptr;
			}

			if(element->second.Client==nullptr && element->second.Device==nullptr) {
				auto c = fmt::format("Removing {}. Device connection time: {}. Client connection time: {}",
									 element->first, element->second.DeviceDisconnected - element->second.DeviceConnected,
									 element->second.ClientDisconnected - element->second.ClientConnected);
				Logger().information(c);
				// std::cout << c << std::endl;
				element = EndPoints_.erase(element);
			} else {
				++element;
			}
		}

		for(auto &element:FailedDevices) {
			delete element;
		}
		FailedDevices.clear();
	}

	void RTTYS_server::CreateNewClient(Poco::Net::HTTPServerRequest &request,
								Poco::Net::HTTPServerResponse &response, const std::string &id) {

		auto WS = new Poco::Net::WebSocket(request, response);
		auto NewClient = new RTTYS_ClientConnection(WS, ClientReactor_, id);

		{
			MyGuard G(M_);
			auto conn = EndPoints_.find(id);
			if (conn == EndPoints_.end()) {
				EndPoint NewEP;
				NewEP.WS_ = WS;
				NewEP.Client = NewClient;
				NewEP.ClientConnected = NewEP.TimeStamp = OpenWifi::Now();
				EndPoints_[id] = NewEP;
			} else {
				conn->second.WS_ = WS;
				conn->second.Client = NewClient;
				conn->second.ClientConnected = conn->second.TimeStamp = OpenWifi::Now();
			}
		}
		if(NewClient->CompleteStartup()) {

		} else {
			Logger().information(fmt::format("Could not start Client {}.",id));
		}
	}

	void RTTYS_server::run() {
		Utils::SetThreadName("rtty-mgr");
		NotificationManagerRunning_ = true;
		Poco::AutoPtr<Poco::Notification> NextMsg(ResponseQueue_.waitDequeueNotification());
		while (NextMsg && NotificationManagerRunning_) {
			auto Resp = dynamic_cast<RTTYS_DisconnectNotification *>(NextMsg.get());
			if (Resp != nullptr) {
				MyGuard G(M_);
				auto It = EndPoints_.find(Resp->id_);
				if (It != EndPoints_.end()) {
					if (Resp->device_ && It->second.Client!= nullptr && It->second.Client->Valid()) {
						Logger().information(fmt::format("{}: Device disconnecting.", Resp->id_));
						It->second.Client->EndConnection(true);
					} else if(!Resp->device_ && It->second.Device!= nullptr && It->second.Device->Valid()) {
						Logger().information(fmt::format("{}: Client disconnecting.", Resp->id_));
						It->second.Device->EndConnection(true);
					}
				}
			}
			NextMsg = ResponseQueue_.waitDequeueNotification();
		}
	}

	void RTTYS_server::RegisterClient(const std::string &Id, RTTYS_ClientConnection *Client) {
		MyGuard 	G(M_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end()) {
			It->second.Client = Client;
			It->second.ClientConnected = OpenWifi::Now();
		}
	}

	bool RTTYS_server::RegisterDevice(const std::string &Id, const std::string &Token, std::string & serial, RTTYS_Device_ConnectionHandler *Device) {
		MyGuard 	G(M_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end()) {
			It->second.Device = Device;
			It->second.Token = Token;
			It->second.DeviceConnected = OpenWifi::Now();
			serial = It->second.SerialNumber;
			Logger().information(fmt::format("Creating session: {}, device:'{}'",Id,It->second.SerialNumber));
			return true;
		}
		return false;
	}

	bool RTTYS_server::SendToClient(const std::string &Id, const u_char *Buf, std::size_t Len) {
		MyGuard 	G(M_);
		try {
			auto It = EndPoints_.find(Id);
			if (It != EndPoints_.end() && It->second.Client != nullptr && It->second.Client->Valid()) {
				It->second.Client->SendData(Buf, Len);
				return true;
			}
			std::cout << "Invalid client - cannot send data" << std::endl;
		} catch(...) {
			std::cout << "Exception during sendclient" << std::endl;
		}
		return false;
	}

	bool RTTYS_server::ValidClient(const std::string &Id) {
		MyGuard 	G(M_);
		auto It = EndPoints_.find(Id);
		return It!=EndPoints_.end() && It->second.Client!= nullptr && It->second.Client->Valid();
	}

	bool RTTYS_server::SendToClient(const std::string &Id, const std::string &s) {
		MyGuard 	G(M_);
		try {
			auto It = EndPoints_.find(Id);
			if(It!=EndPoints_.end() && It->second.Client!= nullptr && It->second.Client->Valid()) {
				It->second.Client->SendData(s);
				return true;
			}
			std::cout << "Invalid client - cannot send data: " << std::endl;
		} catch(...) {
			std::cout << "Exception during sendclient" << std::endl;
		}
		return false;
	}

	void RTTYS_server::DeRegisterClient([[maybe_unused]] const std::string &Id, RTTYS_ClientConnection *Client) {
		MyGuard 	G(M_);
		Logger().information(fmt::format("{}: Deregistering.", Client->ID()));
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end() && It->second.Client==Client) {
			It->second.ClientDisconnected = OpenWifi::Now();
		}
		Logger().information(fmt::format("{}: Deregistered.", Client->ID()));
	}

	void RTTYS_server::DeRegisterDevice([[maybe_unused]] const std::string &Id, [[maybe_unused]] RTTYS_Device_ConnectionHandler *Device, [[maybe_unused]] bool remove_websocket) {
		MyGuard 	G(M_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end() && It->second.Device==Device) {
			It->second.DeviceDisconnected = OpenWifi::Now();
		}
		Logger().information(fmt::format("{}: Deregistered device.", Id));
	}

	bool RTTYS_server::SendKeyStrokes(const std::string &Id, const u_char *buffer, std::size_t s) {
		MyGuard 	G(M_);
		auto It=EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return false;
		}

		try {
			if (It->second.Device != nullptr)
				return It->second.Device->KeyStrokes(buffer, s);
		} catch (...) {

		}
		return false;
	}

	bool RTTYS_server::WindowSize(const std::string &Id, int cols, int rows) {
		MyGuard 	G(M_);
		auto It=EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return false;
		}

		if(It->second.Device!= nullptr)
			return It->second.Device->WindowSize(cols,rows);
		return false;
	}


	bool RTTYS_server::CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber ) {
		MyGuard 	G(M_);

		EndPoint E;
		E.Token = Token;
		E.TimeStamp = OpenWifi::Now();
		E.SerialNumber = SerialNumber;
		E.UserName = UserName;
		EndPoints_[Id] = E;
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
		Logger().information(fmt::format("User: {}, Serial: {} logged in.",It->second.UserName, It->second.SerialNumber ));
	}

	bool RTTYS_server::ValidEndPoint(const std::string &Id, const std::string &Token) {
		MyGuard 	G(M_);
		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return false;
		}
		uint64_t Now = OpenWifi::Now();
		return ((It->second.Token == Token) && ((Now-It->second.TimeStamp)<30));
	}

	bool RTTYS_server::IsDeviceRegistered( const std::string &Id, const std::string &Token, [[maybe_unused]] RTTYS_Device_ConnectionHandler *Conn) {
		MyGuard 	G(M_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end() || It->second.Token != Token )
			return false;
		return true;
	}

	bool RTTYS_server::Login(const std::string & Id) {
		MyGuard 	G(M_);
		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}

		if(It->second.Device!= nullptr) {
			return It->second.Device->Login();
		}

		return false;
	}

	bool RTTYS_server::Logout(const std::string & Id) {
		MyGuard 	G(M_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}
		if(It->second.Device!= nullptr)
			It->second.Device->Logout();
		return true;
	}

}