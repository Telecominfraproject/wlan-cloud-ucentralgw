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


namespace uCentral::RESTAPI {
    int Start();
    void Stop();

    class Service : public SubSystemServer {

    public:
        Service() noexcept;

        friend int Start();
        friend void Stop();

        static Service *instance() {
            if (instance_ == nullptr) {
                instance_ = new Service;
            }
            return instance_;
        }

    private:
		static Service *instance_;

        int Start() override;
        void Stop() override;

        std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   RESTServers_;
		Poco::ThreadPool	Pool_;
    };

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        RequestHandlerFactory() :
            Logger_(Service::instance()->Logger()){}

        Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
    private:
        Poco::Logger    & Logger_;
    };


} //   namespace

#endif //UCENTRAL_UCENTRALRESTAPISERVER_H
