//
// Created by stephane bourque on 2021-06-04.
//

#ifndef UCENTRALGW_AWSNLBHEALTHCHECK_H
#define UCENTRALGW_AWSNLBHEALTHCHECK_H

#include "Poco/Thread.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"

namespace uCentral::NLBHealthCheck {

    class NLBConnection: public Poco::Net::TCPServerConnection
    {
        public:
            NLBConnection(const Poco::Net::StreamSocket& s): TCPServerConnection(s)
            {
            }
            void run();
    };

    typedef Poco::Net::TCPServerConnectionFactoryImpl<NLBConnection> TCPFactory;

    class Service {
    public:
        int Start();
        void Stop();
    private:
        std::unique_ptr<Poco::Net::TCPServer>   Srv_;
        int                                     Port_ = 0;
    };
}

#endif // UCENTRALGW_AWSNLBHEALTHCHECK_H
