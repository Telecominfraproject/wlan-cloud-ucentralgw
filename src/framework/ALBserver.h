//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "framework/SubSystemServer.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"

namespace OpenWifi {

	class ALBRequestHandler: public Poco::Net::HTTPRequestHandler {
	  public:
		explicit ALBRequestHandler(Poco::Logger & L, uint64_t id)
                : Logger_(L), id_(id) {
        }

		void handleRequest([[maybe_unused]] Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response) override;

	  private:
		Poco::Logger 	& Logger_;
		uint64_t 		id_;
	};

	class ALBRequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
	{
	  public:
		explicit ALBRequestHandlerFactory(Poco::Logger & L);
		ALBRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;

	  private:
		Poco::Logger	                            &Logger_;
		inline static std::atomic_uint64_t 			req_id_=1;
	};

	class ALBHealthCheckServer : public SubSystemServer {
	  public:
		ALBHealthCheckServer();

		static auto instance() {
			static auto instance = new ALBHealthCheckServer;
			return instance;
		}

		int Start() override;
		void Stop() override;

	  private:
		std::unique_ptr<Poco::Net::HTTPServer>   	Server_;
		std::unique_ptr<Poco::Net::ServerSocket> 	Socket_;
		int                                     	Port_ = 0;
		mutable std::atomic_bool                    Running_=false;
	};

	inline auto ALBHealthCheckServer() { return ALBHealthCheckServer::instance(); }

} // namespace OpenWifi

