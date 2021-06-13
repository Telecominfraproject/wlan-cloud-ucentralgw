//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#include "kafka_service.h"

#include "Daemon.h"

namespace uCentral {

	class KafkaManager *KafkaManager::instance_ = nullptr;

	KafkaManager::KafkaManager() noexcept: SubSystemServer("KAFKA", "KAFKA-SVR", "ucentral.kafka")
	{
	}

	void KafkaManager::initialize(Poco::Util::Application & self) {
		SubSystemServer::initialize(self);
		KafkaEnabled_ = Daemon()->ConfigGetBool("ucentral.kafka.enable",false);
	}

#ifdef SMALL_BUILD

	int KafkaManager::Start() {
		return 0;
	}
	void KafkaManager::Stop() {
	}

#else

	int KafkaManager::Start() {
		if(!KafkaEnabled_)
			return 0;

		cppkafka::Configuration Config({
										   { "metadata.broker.list", Daemon()->ConfigGetString("ucentral.kafka.brokerlist") } ,
										   { "enable.auto.commit", Daemon()->ConfigGetBool("ucentral.kafka.auto.commit", false)}
									   });
		SystemInfoWrapper_ = "{ \"system\" : { \"id\" : " + std::to_string(Daemon()->ConfigGetInt("ucentral.system.id")) +
							 " , \"host\" : \"" + Daemon()->ConfigGetString("ucentral.system.uri") + "\" } , \"payload\" : " ;

		// Create a producer instance.
		Producer_ = std::make_unique<cppkafka::Producer>(Config);
		Th_.start(*this);
		return 0;
	}

	void KafkaManager::Stop() {
		if(KafkaEnabled_) {
			Running_ = false;
			Th_.wakeUp();
			Th_.join();
			return;
		}
	}

	void KafkaManager::run() {
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

	std::string KafkaManager::WrapSystemId(const std::string & PayLoad) {
		return std::move( SystemInfoWrapper_ + PayLoad + "}");
	}

	void KafkaManager::PostMessage(std::string topic, std::string key, std::string PayLoad) {
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
