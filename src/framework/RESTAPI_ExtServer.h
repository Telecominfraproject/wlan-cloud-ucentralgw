//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "Poco/Net/HTTPServer.h"

#include "framework/SubSystemServer.h"
#include "framework/RESTAPI_Handler.h"

namespace OpenWifi {

	Poco::Net::HTTPRequestHandler * RESTAPI_ExtRouter(const std::string &Path, RESTAPIHandler::BindingMap &Bindings,
													 Poco::Logger & L, RESTAPI_GenericServerAccounting & S, uint64_t Id);

	class ExtRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
	  public:
		ExtRequestHandlerFactory() = default;
		Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &Request) override;
	  private:
		static inline std::atomic_uint64_t  NextTransactionId_ = 1;
	};

	class RESTAPI_ExtServer : public SubSystemServer {
	  public:
		static auto instance() {
			static auto instance_ = new RESTAPI_ExtServer;
			return instance_;
		}

		inline int Start() override {
			poco_information(Logger(),"Starting.");
			Server_.InitLogging();

			for(const auto & Svr: ConfigServersList_) {

				if(MicroServiceNoAPISecurity()) {
					poco_information(Logger(),fmt::format("Starting: {}:{}. Security has been disabled for APIs.", Svr.Address(), Svr.Port()));
				} else {
					poco_information(Logger(),fmt::format("Starting: {}:{} Keyfile:{} CertFile: {}", Svr.Address(), Svr.Port(),
													 Svr.KeyFile(),Svr.CertFile()));
					Svr.LogCert(Logger());
					if (!Svr.RootCA().empty())
						Svr.LogCas(Logger());
				}

				Poco::Net::HTTPServerParams::Ptr Params = new Poco::Net::HTTPServerParams;
				Params->setKeepAlive(true);
				Params->setName("ws:xrest");

				std::unique_ptr<Poco::Net::HTTPServer>  NewServer;
				if(MicroServiceNoAPISecurity()) {
					auto Sock{Svr.CreateSocket(Logger())};
					NewServer = std::make_unique<Poco::Net::HTTPServer>(new ExtRequestHandlerFactory, Pool_, Sock, Params);
				} else {
					auto Sock{Svr.CreateSecureSocket(Logger())};
					NewServer = std::make_unique<Poco::Net::HTTPServer>(new ExtRequestHandlerFactory, Pool_, Sock, Params);
				};
				NewServer->start();
				RESTServers_.push_back(std::move(NewServer));
			}
			return 0;
		}

		inline void Stop() override {
			poco_information(Logger(),"Stopping...");
			for( const auto & svr : RESTServers_ )
				svr->stopAll(true);
			Pool_.stopAll();
			Pool_.joinAll();
			RESTServers_.clear();
			poco_information(Logger(),"Stopped...");
		}

		inline void reinitialize([[maybe_unused]] Poco::Util::Application &self) override {
			MicroServiceLoadConfigurationFile();
			poco_information(Logger(),"Reinitializing.");
			Stop();
			Start();
		}

		Poco::Net::HTTPRequestHandler *CallServer(const std::string &Path, uint64_t Id);
		const Poco::ThreadPool & Pool() { return Pool_; }

	  private:
		std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   RESTServers_;
		Poco::ThreadPool	    Pool_{"x-rest",8,128};
		RESTAPI_GenericServerAccounting   Server_;

		RESTAPI_ExtServer() noexcept:
									   SubSystemServer("RESTAPI_ExtServer", "REST-XSRV", "openwifi.restapi")
		{
		}
	};

	inline auto RESTAPI_ExtServer() { return RESTAPI_ExtServer::instance(); };

}