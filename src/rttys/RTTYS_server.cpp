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
				DeviceAcceptor_ = std::make_unique<Poco::Net::SocketAcceptor<RTTY_Device_ConnectionHandler>>(DeviceSocket,DeviceReactor_);
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
				DeviceAcceptor_ = std::make_unique<Poco::Net::SocketAcceptor<RTTY_Device_ConnectionHandler>>(DeviceSocket,DeviceReactor_);
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
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(new RTTY_Client_RequestHandlerFactory(Logger()), ClientSocket, WebServerHttpParams);
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
				WebServer_ = std::make_unique<Poco::Net::HTTPServer>(new RTTY_Client_RequestHandlerFactory(Logger()), ClientSocket, WebServerHttpParams);
			};
			WebServer_->start();
			ClientReactorThread_.start(ClientReactor_);
			Utils::SetThreadName(ClientReactorThread_,"rt:clntreactor");
		}

		GCCallBack_ = std::make_unique<Poco::TimerCallback<RTTYS_server>>(*this, &RTTYS_server::onTimer);
		Timer_.setStartInterval(10 * 1000);  // first run in 30 seconds
		Timer_.setPeriodicInterval(1 * 60 * 1000);
		Timer_.start(*GCCallBack_);

		return 0;
	}

	void RTTYS_server::Stop() {
		if(Internal_) {
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
		std::lock_guard	G(Mutex_);
		Utils::SetThreadName("rt:janitor");
		for(auto element=EndPoints_.begin();element!=EndPoints_.end();) {
			if(element->second.Client!=nullptr && !element->second.Client->Valid()) {
				std::cout << "Remove client:" << element->first << std::endl;
				delete element->second.Client;
				element->second.Client = nullptr;
				poco_debug(Logger(), fmt::format("Removing {}.",element->first));
				element = EndPoints_.erase(element);
			} else {
				++element;
			}
		}
	}

	void RTTYS_server::Register(const std::string &Id, RTTYS_ClientConnection *Client) {
		std::lock_guard	G(Mutex_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end()) {
			It->second.Client = Client;
			It->second.ClientConnected = OpenWifi::Now();
		}
	}

	bool RTTYS_server::Register(const std::string &Id, const std::string &Token, RTTY_Device_ConnectionHandler *Device) {
		std::lock_guard	G(Mutex_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end()) {
			It->second.Device = Device;
			It->second.Token = Token;
			It->second.DeviceConnected = OpenWifi::Now();
			Logger().information(fmt::format("Creating session: {}, device:'{}'",Id,It->second.SerialNumber));
			return true;
		}
		return false;
	}

	bool RTTYS_server::SendToClient(const std::string &Id, const u_char *Buf, std::size_t Len) {
		std::lock_guard	G(Mutex_);
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

	bool RTTYS_server::SendToClient(const std::string &Id, const std::string &s) {
		std::lock_guard	G(Mutex_);
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

	void RTTYS_server::DeRegister(const std::string &Id, RTTYS_ClientConnection *Client) {
		std::lock_guard	G(Mutex_);
		Logger().information(fmt::format("{}: Deregistering.", Client->ID()));
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end() && It->second.Client==Client) {
			if(It->second.Device!= nullptr) {
				if(!It->second.ShuttingDown) {
					It->second.ShuttingDown = true;
					if(It->second.Device!= nullptr)
						It->second.Device->Stop();
				} else {
					It->second.ShutdownComplete = true;
				}
			} else {
				if(!It->second.ShuttingDown) {
					It->second.ShuttingDown = true;
				} else {
					It->second.ShutdownComplete = true;
				}
			}
			It->second.ClientConnected=0;
		}
		Logger().information(fmt::format("{}: Deregistered.", Client->ID()));
	}

	void RTTYS_server::DeRegisterDevice(const std::string &Id, RTTY_Device_ConnectionHandler *Device, bool remove_websocket) {
		std::lock_guard	G(Mutex_);
		dump("D DEREG--> ", std::cout);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end() && It->second.Device==Device) {
			It->second.Device = nullptr;
			It->second.DeviceConnected = 0 ;
			if(It->second.Client!=nullptr) {
				if(remove_websocket) {
					It->second.ShuttingDown = true;
					if(It->second.Client!= nullptr)
						It->second.Client->Close();
				}
/*				if(!It->second.ShuttingDown) {
					It->second.ShuttingDown = true;
					It->second.Client->Close();
				} else {
					It->second.ShutdownComplete = true;
				}
*/
			} else {
				if(!It->second.ShuttingDown) {
					It->second.ShuttingDown = true;
				} else {
					It->second.ShutdownComplete = true;
				}
			}
		}

		Logger().information(fmt::format("{}: Deregistering device.", Device->SessionID()));
		if(Device!= nullptr) {
			for (auto i = EndPoints_.begin(); i != EndPoints_.end(); i++) {
				if (i->second.Device == Device) {
					EndPoints_.erase(i);
					break;
				}
			}
			dump("D DEREG--> ", std::cout);
		}
		Logger().information(fmt::format("{}: Deregistered device.", Device->SessionID()));
	}

	bool RTTYS_server::SendKeyStrokes(const std::string &Id, const u_char *buffer, std::size_t s) {
		std::lock_guard	G(Mutex_);
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
		std::lock_guard	G(Mutex_);
		auto It=EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return false;
		}

		if(It->second.Device!= nullptr)
			return It->second.Device->WindowSize(cols,rows);
		return false;
	}


	bool RTTYS_server::CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber ) {
		std::lock_guard	G(Mutex_);

		EndPoint E;
		E.ShuttingDown = false;
		E.Token = Token;
		E.TimeStamp = OpenWifi::Now();
		E.SerialNumber = SerialNumber;
		E.UserName = UserName;
		EndPoints_[Id] = E;
		return true;
	}

	std::string RTTYS_server::SerialNumber(const std::string & Id) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end())
			return "";
		return It->second.SerialNumber;
	}

	bool RTTYS_server::ValidId(const std::string &Token) {
		std::lock_guard	G(Mutex_);
		return EndPoints_.find(Token) != EndPoints_.end();
	}

	void RTTYS_server::LoginDone(const std::string & Id) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end())
			return;
		Logger().information(fmt::format("User: {}, Serial: {} logged in.",It->second.UserName, It->second.SerialNumber ));
	}

	bool RTTYS_server::ValidEndPoint(const std::string &Id, const std::string &Token) {
		std::lock_guard	G(Mutex_);
		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return false;
		}
		uint64_t Now = OpenWifi::Now();
		return ((It->second.Token == Token) && ((Now-It->second.TimeStamp)<30));
	}

	bool RTTYS_server::CanConnect( const std::string &Id, RTTYS_ClientConnection *Conn) {
		std::lock_guard	G(Mutex_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end() && It->second.Client==Conn) {
			It->second.ClientConnected = OpenWifi::Now();
			return true;
		}
		return false;
	}

	bool RTTYS_server::IsDeviceRegistered( const std::string &Id, const std::string &Token, [[maybe_unused]] RTTY_Device_ConnectionHandler *Conn) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end() || It->second.Token != Token )
			return false;
		return true;
	}

	uint64_t RTTYS_server::DeviceSessionID(const std::string & Id) {
		auto it = EndPoints_.find(Id);
		if(it==EndPoints_.end()) {
			return 0;
		} else {
			if(it->second.Device== nullptr) {
				return 0;
			} else {
				return it->second.Device->SessionID();
			}
		}
	}

	bool RTTYS_server::Login(const std::string & Id) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}

		if(It->second.Device!= nullptr) {
//			std::cout << "login " << Id << " session " << It->second.Device->SessionID() << std::endl;
//			It->second.Device->AddCommand(RTTY_Device_ConnectionHandler::msgTypeLogin);
//			std::cout << "login done" << Id << std::endl;
			return It->second.Device->Login();
		}

		return false;
	}

	bool RTTYS_server::Logout(const std::string & Id) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}
		if(It->second.Device!= nullptr)
			It->second.Device->Logout();
		return true;
	}

	bool RTTYS_server::Close(const std::string & Id) {
		std::lock_guard	G(Mutex_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}

		if(It->second.Device!= nullptr)
			It->second.Device->Stop();
		return true;
	}
}