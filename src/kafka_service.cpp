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
		return Service::instance()->Start();
	}

	void Stop() {
		Service::instance()->Stop();
	}

	bool Enabled() {
		return Service::instance()->Enabled();
	}

	void PostMessage(std::string Topic, std::string Key, std::string Payload) {
		Service::instance()->PostMessage(std::move(Topic), std::move(Key), std::move(Payload));
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
		SystemInfoWrapper_ = "{ \"system\" : { \"id\" : " + std::to_string(uCentral::ServiceConfig::GetInt("ucentral.system.id")) +
							 " , \"host\" : \"" + uCentral::ServiceConfig::GetString("ucentral.system.uri") + "\" } , \"payload\" : " ;

		// Create a producer instance.
		Producer_ = std::make_unique<cppkafka::Producer>(Config);
		Th_.start(*this);
		return 0;
	}

	void Service::Stop() {
		if(KafkaEnabled_) {
			Running_ = false;
			Th_.wakeUp();
			Th_.join();
			return;
		}
	}

	void Service::run() {
		Running_ = true ;
		while(Running_) {
			Poco::Thread::trySleep(3000);
			if(!Running_)
				break;
			{
				SubMutexGuard G(Mutex_);

				while (!Queue_.empty()) {
					const auto M = Queue_.front();
					// std::cout << "Producing Topic: " << M.Topic << " Key: "  << M.Key <<std::endl;
					Producer_->produce(
						cppkafka::MessageBuilder(M.Topic).key(M.Key).payload(M.PayLoad));
					Queue_.pop();
				}
				// Producer_->flush();

			}
		}
	}

	std::string Service::WrapSystemId(const std::string & PayLoad) {
		return std::move( SystemInfoWrapper_ + PayLoad + "}");
	}

	void Service::PostMessage(std::string topic, std::string key, std::string PayLoad) {
		if(KafkaEnabled_  && Running_) {
			SubMutexGuard G(Mutex_);

			KMessage M{
				.Topic = std::move(topic), .Key = std::move(key), .PayLoad = std::move(WrapSystemId(PayLoad))};
			// std::cout << "Posting Topic: " << M.Topic << " Key: "  << M.Key << " Payload: " << M.PayLoad << std::endl;
			Queue_.push(std::move(M));
		}
	}

#endif
} // namespace
