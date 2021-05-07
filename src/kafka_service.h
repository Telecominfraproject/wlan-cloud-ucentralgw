//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_KAFKA_SERVICE_H
#define UCENTRALGW_KAFKA_SERVICE_H

#include "SubSystemServer.h"

#include "cppkafka/cppkafka.h"

namespace uCentral::Kafka {

	int Start();
	void Stop();

class Service : public SubSystemServer {
  public:

	Service() noexcept;

	friend int uCentral::Kafka::Start();
	friend void uCentral::Kafka::Stop();

	void initialize(Poco::Util::Application & self) override;
	static Service *instance() {
		if(instance_== nullptr)
			instance_ = new Service;
		return instance_;
	}

  private:
	static Service *instance_;
	std::unique_ptr<cppkafka::Producer> 	Producer_;
	bool KafkaEnabled_ = false;

	int Start() override;
	void Stop() override;
};

}	// NameSpace

#endif // UCENTRALGW_KAFKA_SERVICE_H
