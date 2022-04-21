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
			int DSport = MicroService::instance().ConfigGetInt("rtty.port", 5912);
			int CSport = MicroService::instance().ConfigGetInt("rtty.viewport", 5913);
			RTTY_UIAssets_ =
				MicroService::instance().ConfigPath("rtty.assets", "$OWGW_ROOT/rtty_ui");

			auto CertFileName = MicroService::instance().ConfigPath("openwifi.restapi.host.0.cert");
			auto KeyFileName = MicroService::instance().ConfigPath("openwifi.restapi.host.0.key");
			auto RootCa = MicroService::instance().ConfigPath("openwifi.restapi.host.0.rootca");
			Poco::Crypto::X509Certificate Root(RootCa);

			auto DeviceSecureContext = new Poco::Net::Context(Poco::Net::Context::SERVER_USE,
									   	KeyFileName, CertFileName, "",
										Poco::Net::Context::VERIFY_RELAXED);

			DeviceSecureContext->addCertificateAuthority(Root);
			DeviceSecureContext->disableStatelessSessionResumption();
			DeviceSecureContext->enableSessionCache();
			DeviceSecureContext->setSessionCacheSize(0);
			DeviceSecureContext->setSessionTimeout(10);
			DeviceSecureContext->enableExtendedCertificateVerification(true);
			SSL_CTX *SSLCtxDevice = DeviceSecureContext->sslContext();
			SSL_CTX_dane_enable(SSLCtxDevice);

			Poco::Net::SecureServerSocket DeviceSocket(DSport, 64, DeviceSecureContext);
			Poco::Net::TCPServerParams* pParams = new Poco::Net::TCPServerParams();
			pParams->setMaxThreads(50);
			pParams->setMaxQueued(100);
			// pParams->setThreadIdleTime(100);

			DeviceAcceptor_ = std::make_unique<Poco::Net::TCPServer>(new Poco::Net::TCPServerConnectionFactoryImpl<RTTY_Device_ConnectionHandler>(), DeviceSocket, pParams);
			DeviceAcceptor_->start();

			auto ClientSecureContext =
				new Poco::Net::Context(Poco::Net::Context::SERVER_USE, KeyFileName, CertFileName,
									   "", Poco::Net::Context::VERIFY_RELAXED);
			ClientSecureContext->addCertificateAuthority(Root);
			ClientSecureContext->disableStatelessSessionResumption();
			ClientSecureContext->enableSessionCache();
			ClientSecureContext->setSessionCacheSize(0);
			ClientSecureContext->setSessionTimeout(10);
			ClientSecureContext->enableExtendedCertificateVerification(true);
			SSL_CTX *SSLCtxClient = ClientSecureContext->sslContext();
			SSL_CTX_dane_enable(SSLCtxClient);

			Poco::Net::SecureServerSocket ClientSocket(CSport, 64, ClientSecureContext);
			ClientSocket.setNoDelay(true);

			auto HttpParams = new Poco::Net::HTTPServerParams;
			HttpParams->setMaxThreads(50);
			HttpParams->setMaxQueued(200);
			HttpParams->setKeepAlive(true);

			WebServer_ = std::make_unique<Poco::Net::HTTPServer>(
				new RTTY_Client_RequestHandlerFactory(ClientReactor_), ClientSocket, HttpParams);
			ClientReactorThread_.setName("RTTYWebServerClientThread");
			ClientReactorThread_.start(ClientReactor_);
			WebServer_->start();
		}

		return 0;
	}

	void RTTYS_server::Stop() {
		if(Internal_) {
			WebServer_->stopAll();
			DeviceAcceptor_->stop();
			ClientReactor_.stop();
			ClientReactorThread_.join();
		}
	}

	void RTTYS_server::Register(const std::string &Id, RTTYS_ClientConnection *Client) {
		std::lock_guard	G(M_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end()) {
			It->second.Client = Client;
			It->second.ClientConnected = OpenWifi::Now();
		}
	}

	void RTTYS_server::DeRegister(const std::string &Id, RTTYS_ClientConnection *Client) {
		std::lock_guard	G(M_);
		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end())
			return;
		if(It->second.Client!=Client)
			return;
		It->second.Client = nullptr;
		It->second.Done = true;
		It->second.ClientConnected = 0 ;
	}

	RTTYS_ClientConnection * RTTYS_server::GetClient(const std::string &Id) {
		std::lock_guard	G(M_);
		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return nullptr;
		}
		return It->second.Client;
	}

	bool RTTYS_server::Register(const std::string &Id, const std::string &Token, RTTY_Device_ConnectionHandler *Device) {
		std::lock_guard	G(M_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end()) {
			std::cout << "Updating connection" << std::endl;
			/* if(It->second.Device!= nullptr) {
				std::cout << "Removing old device connection" << std::endl;
				delete It->second.Device;
			}
			 */
			It->second.Device = Device;
			It->second.Token = Token;
			It->second.DeviceConnected = OpenWifi::Now();
			Logger().information(fmt::format("Creating session: {}, device:'{}'",Id,It->second.SerialNumber));
			return true;
		}
		return false;
	}

	void RTTYS_server::DeRegister(const std::string &Id, RTTY_Device_ConnectionHandler *Device) {
		std::lock_guard	G(M_);
		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end() && It->second.Device==Device) {
			It->second.Device = nullptr;
			It->second.DeviceConnected = 0 ;
			return;
		}
	}

	RTTY_Device_ConnectionHandler * RTTYS_server::GetDevice(const std::string &id) {
		std::lock_guard	G(M_);
		auto It = EndPoints_.find(id);
		if(It==EndPoints_.end()) {
			return nullptr;
		}
		return It->second.Device;
	}

	bool RTTYS_server::CreateEndPoint(const std::string &Id, const std::string & Token, const std::string & UserName, const std::string & SerialNumber ) {
		std::lock_guard	G(M_);

		EndPoint E;
		E.Done = false;
		E.Token = Token;
		E.TimeStamp = std::time(nullptr);
		E.SerialNumber = SerialNumber;
		E.UserName = UserName;
		EndPoints_[Id] = E;
		return true;
	}

	std::string RTTYS_server::SerialNumber(const std::string & Id) {
		std::lock_guard	G(M_);

		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end())
			return "";
		return It->second.SerialNumber;
	}

	void RTTYS_server::LoginDone(const std::string & Id) {
		std::lock_guard	G(M_);

		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end())
			return;
		Logger().information(fmt::format("User: {}, Serial: {} logged in.",It->second.UserName, It->second.SerialNumber ));
	}

	bool RTTYS_server::ValidEndPoint(const std::string &Id, const std::string &Token) {
		std::lock_guard	G(M_);
		auto It = EndPoints_.find(Id);
		if(It==EndPoints_.end()) {
			return false;
		}
		uint64_t Now = std::time(nullptr);
		return ((It->second.Token == Token) && ((Now-It->second.TimeStamp)<30));
	}

	bool RTTYS_server::CanConnect( const std::string &Id, RTTYS_ClientConnection *Conn) {
		std::lock_guard	G(M_);

		auto It = EndPoints_.find(Id);
		if(It!=EndPoints_.end() && It->second.Client==Conn) {
			It->second.ClientConnected = std::time(nullptr);
			return true;
		}
		return false;
	}

	bool RTTYS_server::IsDeviceRegistered( const std::string &Id, const std::string &Token, [[maybe_unused]] RTTY_Device_ConnectionHandler *Conn) {
		std::lock_guard	G(M_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end() || It->second.Token != Token )
			return false;
		return true;
	}

	uint64_t RTTYS_server::DeviceSessionID(const std::string & Id) {
		auto it = EndPoints_.find(Id);
		if(it==EndPoints_.end()) {
			std::cout << "No ID found" << std::endl;
			return 0;
		} else {
			if(it->second.Device== nullptr) {
				std::cout << "No device for ID found" << std::endl;
				return 0;
			} else {
				return it->second.Device->SessionID();
			}
		}
	}


	bool RTTYS_server::Login(const std::string & Id) {
		std::lock_guard	G(M_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			std::cout << "cannot find login " << Id << std::endl;
			return false;
		}

		if(It->second.Device!= nullptr) {
			std::cout << "login " << Id << " session " << It->second.Device->SessionID() << std::endl;
			It->second.Device->AddCommand(RTTY_Device_ConnectionHandler::msgTypeLogin);
			std::cout << "login done" << Id << std::endl;
			return true;
		}

		std::cout << "no device so cannot login " << Id << std::endl;
		return false;
	}

	bool RTTYS_server::Logout(const std::string & Id) {
		std::lock_guard	G(M_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}
		if(It->second.Device!= nullptr)
			It->second.Device->Logout();
		return true;
	}

	bool RTTYS_server::Close(const std::string & Id) {
		std::lock_guard	G(M_);

		auto It = EndPoints_.find(Id);
		if(It == EndPoints_.end()) {
			return false;
		}

		if(It->second.Device!= nullptr)
			delete It->second.Client;
		It->second.Client = nullptr;
		return true;
	}


}