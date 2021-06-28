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
		ProducerThr_ = std::make_unique<std::thread>([this]() { this->Producer(); });
		ConsumerThr_ = std::make_unique<std::thread>([this]() { this->Consumer(); });
		return 0;
	}

	void KafkaManager::Stop() {
		if(KafkaEnabled_) {
			ProducerRunning_ = ConsumerRunning_ = false;
			ProducerThr_->join();
			ConsumerThr_->join();
			return;
		}
	}

	void KafkaManager::Producer() {
		cppkafka::Configuration Config({
										   { "metadata.broker.list", Daemon()->ConfigGetString("ucentral.kafka.brokerlist") }
									   });
		SystemInfoWrapper_ = 	R"lit({ "system" : { "id" : )lit" +
								  	std::to_string(Daemon()->ID()) +
									R"lit( , "host" : ")lit" + Daemon()->PrivateEndPoint() +
									R"lit(" } , "payload" : ")lit" ;
		cppkafka::Producer	Producer(Config);
		ProducerRunning_ = true;
		while(ProducerRunning_) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			try
			{
				SubMutexGuard G(ProducerMutex_);
				while (!Queue_.empty()) {
					const auto M = Queue_.front();
					Producer.produce(
						cppkafka::MessageBuilder(M.Topic).key(M.Key).payload(M.PayLoad));
					Queue_.pop();
				}
				Producer.flush();
			} catch (const cppkafka::HandleException &E ) {
				Logger_.warning(Poco::format("Caught a Kafka exception (producer): %s",std::string{E.what()}));
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
			}
		}
	}

	void KafkaManager::Consumer() {
		cppkafka::Configuration Config({
										   { "group.id", Daemon()->ConfigGetString("ucentral.kafka.group.id") },
										   { "enable.auto.commit", Daemon()->ConfigGetBool("ucentral.kafka.auto.commit",false) },
										   { "metadata.broker.list", Daemon()->ConfigGetString("ucentral.kafka.brokerlist") },
										   { "auto.offset.reset", "earliest" } ,
										   { "enable.partition.eof", false }
									   });

		cppkafka::Consumer Consumer(Config);
		Consumer.set_assignment_callback([=](const cppkafka::TopicPartitionList& partitions) {
			std::cout << "Partition assigned: " << partitions.front().get_partition() << std::endl;
		  	Logger_.information(Poco::format("Got assigned: %Lu...",(uint64_t )partitions.front().get_partition()));
		});
		Consumer.set_revocation_callback([this](const cppkafka::TopicPartitionList& partitions) {
			std::cout << "Partition revocation: " << partitions.front().get_partition() << std::endl;
		  Logger_.information(Poco::format("Got revoked: %Lu...",(uint64_t )partitions.front().get_partition()));
		});

		Types::StringVec    Topics;
		for(const auto &i:Notifiers_)
			Topics.push_back(i.first);

		Consumer.subscribe(Topics);

		ConsumerRunning_ = true;
		while(ConsumerRunning_) {
			try {
				cppkafka::Message Msg = Consumer.poll(std::chrono::milliseconds(200));
				if (!Msg)
					continue;;
				if (Msg.get_error()) {
					if (!Msg.is_eof()) {
						Logger_.error(Poco::format("Error: %s", Msg.get_error().to_string()));
					}
					Consumer.commit(Msg);
					continue;
				}
				SubMutexGuard G(ConsumerMutex_);
				auto It = Notifiers_.find(Msg.get_topic());
				if (It != Notifiers_.end()) {
					Types::TopicNotifyFunctionList &FL = It->second;
					for (auto &F : FL) {
						std::string Key{Msg.get_key()};
						std::string Payload{Msg.get_payload()};
						std::thread T(F.first, Key, Payload);
						T.detach();
					}
				}
				Consumer.commit(Msg);
			} catch (const cppkafka::HandleException &E) {
				Logger_.warning(Poco::format("Caught a Kafka exception (consumer): %s",std::string{E.what()}));
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
			}
		}
	}

	std::string KafkaManager::WrapSystemId(const std::string & PayLoad) {
		return std::move( SystemInfoWrapper_ + PayLoad + "}");
	}

	void KafkaManager::PostMessage(std::string topic, std::string key, std::string PayLoad, bool WrapMessage ) {
		if(KafkaEnabled_) {
			SubMutexGuard G(Mutex_);
			KMessage M{
				.Topic = std::move(topic),
				.Key = std::move(key),
				.PayLoad = std::move(WrapMessage ? WrapSystemId(PayLoad) : PayLoad )};
			Queue_.push(std::move(M));
		}
	}

	int KafkaManager::RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction &F) {
		if(KafkaEnabled_) {
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
		if(KafkaEnabled_) {
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
