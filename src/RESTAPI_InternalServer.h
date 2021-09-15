//
// Created by stephane bourque on 2021-06-29.
//

#ifndef UCENTRALSEC_RESTAPI_INTERNALSERVER_H
#define UCENTRALSEC_RESTAPI_INTERNALSERVER_H

#include "SubSystemServer.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/NetException.h"
#include "RESTAPI_GenericServer.h"

namespace OpenWifi {

    class RESTAPI_InternalServer : public SubSystemServer {

    public:

        static RESTAPI_InternalServer *instance() {
            if (instance_ == nullptr) {
                instance_ = new RESTAPI_InternalServer;
            }
            return instance_;
        }

        int Start() override;
        void Stop() override;

    private:
        static RESTAPI_InternalServer *instance_;
        std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   RESTServers_;
        Poco::ThreadPool	Pool_;
		RESTAPI_GenericServer	Server_;

		RESTAPI_InternalServer() noexcept: SubSystemServer("RESTAPIInternalServer", "REST-ISRV", "openwifi.internal.restapi")
			{
			}
    };

    inline RESTAPI_InternalServer * RESTAPI_InternalServer() { return RESTAPI_InternalServer::instance(); };

    class InternalRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        explicit InternalRequestHandlerFactory(RESTAPI_GenericServer & Server) :
			Logger_(RESTAPI_InternalServer()->Logger()),
			Server_(Server)
		{}

        Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
    private:
        Poco::Logger    & Logger_;
		RESTAPI_GenericServer	& Server_;
    };


} //   namespace

#endif //UCENTRALSEC_RESTAPI_INTERNALSERVER_H
