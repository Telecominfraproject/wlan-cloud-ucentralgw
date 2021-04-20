//
// Created by stephane bourque on 2021-04-19.
//

#include "kafka_service.h"

#include "uCentral.h"

namespace uCentral::Kafka {

	uCentral::Kafka::Service *uCentral::Kafka::Service::instance_ = nullptr;

	Service::Service() noexcept:
		SubSystemServer("KAFKA", "KAFKA-SVR", "ucentral.kafka")
	{
	}

	int Start() {
		return Service::instance_->Start();
	};

	void Stop() {
		Service::instance_->Stop();
	}

	void Service::initialize(Poco::Util::Application & self) {
		 KafkaEnabled_ = uCentral::ServiceConfig::getBool("ucentral.kafka.enable",false);
	}

	int Service::Start() {
		if(!KafkaEnabled_)
			return 0;

		cppkafka::Configuration Config({
										   { "metadata.broker.list", uCentral::ServiceConfig::getString("ucentral.kafka.brokerlist") }
									   });

		// Create a producer instance.
		producer_ = std::make_unique<cppkafka::Producer>(Config);


		return 0;
	}

	void Service::Stop() {
		if(!KafkaEnabled_)
			return;
	}

}; // namespace
