//
// Created by stephane bourque on 2021-04-19.
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
	int Start() override;
	void Stop() override;

	std::unique_ptr<cppkafka::Producer> 	producer_;
	static Service *instance_;

	bool KafkaEnabled_ = false;
};

}	// NameSpace

#endif // UCENTRALGW_KAFKA_SERVICE_H
