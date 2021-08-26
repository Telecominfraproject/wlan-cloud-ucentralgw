//
// Created by stephane bourque on 2021-06-04.
//

#ifndef UCENTRALGW_ALBHEALTHCHECKSERVER_H
#define UCENTRALGW_ALBHEALTHCHECKSERVER_H

#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Poco/Thread.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Logger.h"

#include "Daemon.h"
#include "SubSystemServer.h"

namespace OpenWifi {

	class ALBRequestHandler: public Poco::Net::HTTPRequestHandler
			/// Return a HTML document with the current date and time.
		{
		  public:
			ALBRequestHandler(Poco::Logger & L)
				: Logger_(L)
			{
			}

			void handleRequest(Poco::Net::HTTPServerRequest& Request, Poco::Net::HTTPServerResponse& Response)
			{
				Logger_.information(Poco::format("ALB-REQUEST(%s): New ALB request.",Request.clientAddress().toString()));
				Response.setChunkedTransferEncoding(true);
				Response.setContentType("text/html");
				Response.setDate(Poco::Timestamp());
				Response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				Response.setKeepAlive(true);
				Response.set("Connection","keep-alive");
				Response.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
				std::ostream &Answer = Response.send();
				Answer << "uCentralGW Alive and kicking!" ;
			}

	  private:
		Poco::Logger 	& Logger_;
	};

	class ALBRequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
		{
		  public:
			explicit ALBRequestHandlerFactory(Poco::Logger & L):
				Logger_(L)
			{
			}

			ALBRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override
			{
				if (request.getURI() == "/")
					return new ALBRequestHandler(Logger_);
				else
					return nullptr;
			}

		  private:
			Poco::Logger	&Logger_;
		};

    class ALBHealthCheckServer : public SubSystemServer {
        public:
            ALBHealthCheckServer() noexcept:
                    SubSystemServer("ALBHealthCheckServer", "ALB-SVR", "alb")
            {
            }

            static ALBHealthCheckServer *instance() {
                if (instance_ == nullptr) {
                    instance_ = new ALBHealthCheckServer;
                }
                return instance_;
            }

            int Start() {
                if(Daemon()->ConfigGetBool("alb.enable",false)) {
                    Port_ = (int)Daemon()->ConfigGetInt("alb.port",15015);
                    Socket_ = std::make_unique<Poco::Net::ServerSocket>(Port_);
                    auto Params = new Poco::Net::HTTPServerParams;
                    Server_ = std::make_unique<Poco::Net::HTTPServer>(new ALBRequestHandlerFactory(Logger_), *Socket_, Params);
                    Server_->start();
                }

                return 0;
            }

            void Stop() {
                if(Server_)
                    Server_->stop();
            }

          private:
            static ALBHealthCheckServer *instance_;
            std::unique_ptr<Poco::Net::HTTPServer>   	Server_;
            std::unique_ptr<Poco::Net::ServerSocket> 	Socket_;
            int                                     	Port_ = 0;
        };

    inline ALBHealthCheckServer * ALBHealthCheckServer() { return ALBHealthCheckServer::instance(); }
    inline class ALBHealthCheckServer * ALBHealthCheckServer::instance_ = nullptr;
}

#endif // UCENTRALGW_ALBHEALTHCHECKSERVER_H
