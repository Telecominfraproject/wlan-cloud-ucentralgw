//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRAL_UCENTRALRESTAPISERVER_H
#define UCENTRAL_UCENTRALRESTAPISERVER_H

#include "SubSystemServer.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/NetException.h"

namespace OpenWifi {

    class RESTAPI_server : public SubSystemServer {

    public:
		int Start() override;
		void Stop() override;
        static RESTAPI_server *instance() {
            if (instance_ == nullptr) {
                instance_ = new RESTAPI_server;
            }
            return instance_;
        }

    private:
		static RESTAPI_server *instance_;
        std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   RESTServers_;
		Poco::ThreadPool	Pool_;
		RESTAPI_server() noexcept;
    };

class RESTAPIServerRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
	RESTAPIServerRequestHandlerFactory() :
            Logger_(RESTAPI_server::instance()->Logger()){}

        Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
    private:
        Poco::Logger    & Logger_;
    };

	inline RESTAPI_server * RESTAPI_server() { return RESTAPI_server::instance(); }
} //   namespace

#endif //UCENTRAL_UCENTRALRESTAPISERVER_H
