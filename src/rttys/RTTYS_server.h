//
// Created by stephane bourque on 2021-11-23.
//

#ifndef UCENTRALGW_RTTYS_SERVER_H
#define UCENTRALGW_RTTYS_SERVER_H

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"

#include "RTTYS_device.h"
#include "rttys/RTTY_ClientConnection.h"

namespace OpenWifi {

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
		inline auto UI() { return UI_; }

		inline void Register(const std::string &id, RTTY_ClientConnection *Conn) {
			std::lock_guard	G(Mutex_);
			std::cout << "Registering client:" << id << std::endl;
			Clients_[id] = Conn;
		}

		inline void DeRegister(const std::string &id, RTTY_ClientConnection *Conn) {
			std::lock_guard	G(Mutex_);
			std::cout << "DeRegistering client: '" << id << "'" << std::endl;
			auto It = Clients_.find(id);
			if(It==Clients_.end())
				return;
			if(It->second!=Conn)
				return;
			Clients_.erase(It);
		}

		inline RTTY_ClientConnection * GetClient(const std::string &id) {
			std::lock_guard	G(Mutex_);
			auto It = Clients_.find(id);
			if(It==Clients_.end()) {
				std::cout << "Cannot find client '" << id << "' ... " << std::endl;
				return nullptr;
			}
			return It->second;
		}

		inline void Register(const std::string &id, RTTY_Device_ConnectionHandler *Conn) {
			std::lock_guard	G(Mutex_);
			std::cout << "Registering device:" << id << std::endl;
			Devices_[id] = Conn;
		}

		inline void DeRegister(const std::string &id, RTTY_Device_ConnectionHandler *Conn) {
			std::lock_guard	G(Mutex_);
			std::cout << "DeRegistering device:" << id << std::endl;
			auto It = Devices_.find(id);
			if(It==Devices_.end())
				return;
			if(It->second!=Conn)
				return;
			Devices_.erase(It);
		}

		inline RTTY_Device_ConnectionHandler * GetDevice(const std::string &id) {
			std::lock_guard	G(Mutex_);
			auto It = Devices_.find(id);
			if(It==Devices_.end()) {
				std::cout << "Cannot find device '" << id << "' ... " << std::endl;
 				return nullptr;
			}
			return It->second;
		}

	  private:
		Poco::Net::SocketReactor	DeviceReactor_;
		Poco::Net::SocketReactor	ClientReactor_;
		Poco::Thread				DeviceReactorThread_;
		Poco::Thread				ClientReactorThread_;
		std::string 				RTTY_UIAssets_;
		std::string 				RTTY_UIuri_;
		std::string 				UI_;

		std::map<std::string, RTTY_ClientConnection *>		Clients_;
		std::map<std::string, RTTY_Device_ConnectionHandler *> Devices_;

		std::unique_ptr<Poco::Net::SocketAcceptor<RTTY_Device_ConnectionHandler>>	DeviceAcceptor_;
		std::unique_ptr<Poco::Net::HTTPServer>		WebServer_;

		explicit RTTYS_server() noexcept:
		SubSystemServer("RTTY_Server", "RTTY-SVR", "rtty.server")
			{
			}
	};

	inline RTTYS_server * RTTYS_server() { return RTTYS_server::instance(); }

} // namespace OpenWifi

#endif // UCENTRALGW_RTTYS_SERVER_H
