//
// Created by stephane bourque on 2021-11-23.
//

#ifndef UCENTRALGW_RTTYS_SERVER_H
#define UCENTRALGW_RTTYS_SERVER_H

#include "framework/MicroService.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"

#include "RTTYS_device.h"

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

	  private:
		std::map<std::string,std::string>	Registrations_;

		Poco::Net::SocketReactor	DeviceReactor_;
		Poco::Net::SocketReactor	ClientReactor_;
		Poco::Thread				DeviceReactorThread_;
		Poco::Thread				ClientReactorThread_;
		std::string 				RTTY_UIAssets_;
		std::string 				RTTY_UIuri_;
		std::string 				UI_;

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
