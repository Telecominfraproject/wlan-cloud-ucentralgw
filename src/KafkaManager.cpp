//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//
#include <thread>

#include "KafkaManager.h"

#include "Daemon.h"

namespace uCentral {

	class KafkaManager *KafkaManager::instance_ = nullptr;

	KafkaManager::KafkaManager() noexcept:
		SubSystemServer("KafkaManager", "KAFKA-SVR", "ucentral.kafka")
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
		Running_ = true;
		ProducerThr_ = std::make_unique<std::thread>(Producer,this);
		ProducerThr_->detach();
		ConsumerThr_ = std::make_unique<std::thread>(Consumer,this);
		ConsumerThr_->detach();
		return 0;
	}

	void KafkaManager::Stop() {
		if(KafkaEnabled_) {
			Running_ = false;
			ConsumerThr_->join();
			ProducerThr_->join();
			return;
		}
	}

	void KafkaManager::Producer(KafkaManager *Mgr) {
		cppkafka::Configuration Config({
										   { "metadata.broker.list", Daemon()->ConfigGetString("ucentral.kafka.brokerlist") } ,
										   { "enable.auto.commit", Daemon()->ConfigGetBool("ucentral.kafka.auto.commit", false)}
									   });
		Mgr->SystemInfoWrapper_ = 	R"lit({ "system" : { "id" : )lit" +
								  	std::to_string(Daemon()->ConfigGetInt("ucentral.system.id")) +
									R"lit( , "host" : ")lit" + Daemon()->ConfigGetString("ucentral.system.uri") +
									R"lit(" } , "payload" : ")lit" ;

		cppkafka::Producer	Producer(Config);

		while(Mgr->Running_) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			if(!Mgr->Running_)
				break;
			{
				SubMutexGuard G(Mgr->ProducerMutex_);
				while (!Mgr->Queue_.empty() && Mgr->Running_) {
					const auto M = Mgr->Queue_.front();
					// std::cout << "Producing Topic: " << M.Topic << " Key: "  << M.Key <<std::endl;
					Producer.produce(
						cppkafka::MessageBuilder(M.Topic).key(M.Key).payload(M.PayLoad));
					Mgr->Queue_.pop();
				}
				// Producer_->flush();

			}
		}
	}

	void KafkaManager::Consumer(KafkaManager *Mgr ) {
		cppkafka::Configuration Config({
										   { "group.id", 1 },
										   { "enable.auto.commit", Daemon()->ConfigGetBool("ucentral.kafka.auto.commit",false) },
										   { "metadata.broker.list", Daemon()->ConfigGetString("ucentral.kafka.brokerlist") }
									   });

		cppkafka::Consumer Consumer(Config);

		Consumer.set_assignment_callback([Mgr](const cppkafka::TopicPartitionList& partitions) {
		  Mgr->Logger_.information(Poco::format("Got assigned: %Lu...",(uint64_t )partitions.front().get_partition()));
		});
		Consumer.set_revocation_callback([Mgr](const cppkafka::TopicPartitionList& partitions) {
		  Mgr->Logger_.information(Poco::format("Got revoked: %Lu...",(uint64_t )partitions.front().get_partition()));
		});

		std::vector<std::string>    Topics;
		for(const auto &i:Mgr->Notifiers_)
			Topics.push_back(i.first);

		Consumer.subscribe(Topics);
		while(Mgr->Running_) {
			cppkafka::Message Msg = Consumer.poll(std::chrono::milliseconds(2000));
			if (Msg) {
				if (Msg.get_error()) {
					if (!Msg.is_eof()) {
						Mgr->Logger_.error(
							Poco::format("Error: %s", Msg.get_error().to_string()));
					}
				} else {
					SubMutexGuard G(Mgr->ConsumerMutex_);
					auto It = Mgr->Notifiers_.find(Msg.get_topic());
					if (It != Mgr->Notifiers_.end()) {
						Types::TopicNotifyFunctionList &FL = It->second;
						for (auto &F : FL)
							F.first(Msg.get_key(), Msg.get_payload());
					}
					Consumer.commit(Msg);
				}
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

	int KafkaManager::RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction &F) {
		if(!Running_) {
			SubMutexGuard G(Mutex_);
			auto It = Notifiers_.find(Topic);
			if(It == Notifiers_.end()) {
				Types::TopicNotifyFunctionList L;
				L.emplace(L.end(),std::make_pair(F,FunctionId_));
				Notifiers_[Topic] = std::move(L);
			} else {
				It->second.emplace(It->second.end(),std::make_pair(F,FunctionId_));
			}
			return FunctionId_++;
		} else {
			return 0;
		}
	}

	void KafkaManager::UnregisterTopicWatcher(const std::string &Topic, int Id) {
		if(!Running_) {
			SubMutexGuard G(Mutex_);
			auto It = Notifiers_.find(Topic);
			if(It != Notifiers_.end()) {
				Types::TopicNotifyFunctionList & L = It->second;
				for(auto it=L.begin(); it!=L.end(); it++)
					if(it->second == Id) {
						L.erase(it);
						break;
					}
			}
		}
	}

#endif
} // namespace
