//
// Created by stephane bourque on 2021-06-04.
//

#ifndef UCENTRALGW_AWSNLBHEALTHCHECK_H
#define UCENTRALGW_AWSNLBHEALTHCHECK_H

#include "Poco/Thread.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/ServerSocket.h"

class AwsNLBHealthCheck : public Poco::Runnable {
  public:
	void run() override;
	int Start();
	void Stop();
  private:
	std::atomic_bool 		Running_=false;
	uint 					Port_=0;
	Poco::Thread			Th_;
	Poco::Net::ServerSocket	Sock_;
};

#endif // UCENTRALGW_AWSNLBHEALTHCHECK_H
