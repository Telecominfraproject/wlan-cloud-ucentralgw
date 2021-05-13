//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "kafka_service.h"

#include "uCentral.h"

namespace uCentral::Kafka {

	uCentral::Kafka::Service *uCentral::Kafka::Service::instance_ = nullptr;

	Service::Service() noexcept: uSubSystemServer("KAFKA", "KAFKA-SVR", "ucentral.kafka")
	{
	}

	int Start() {
		return Service::instance_->Start();
	}

	void Stop() {
		Service::instance_->Stop();
	}

	void Service::initialize(Poco::Util::Application & self) {
		uSubSystemServer::initialize(self);
		KafkaEnabled_ = uCentral::ServiceConfig::GetBool("ucentral.kafka.enable",false);
	}

#ifdef SMALL_BUILD

	int Service::Start() {
		return 0;
	}
	void Service::Stop() {
	}

#else

	int Service::Start() {
		if(!KafkaEnabled_)
			return 0;

		cppkafka::Configuration Config({
										   { "metadata.broker.list", uCentral::ServiceConfig::GetString("ucentral.kafka.brokerlist") }
									   });

		// Create a producer instance.
		Producer_ = std::make_unique<cppkafka::Producer>(Config);

		return 0;
	}

	void Service::Stop() {
		if(!KafkaEnabled_)
			return;
	}

#endif
} // namespace
