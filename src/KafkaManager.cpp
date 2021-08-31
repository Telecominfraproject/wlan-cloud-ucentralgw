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
#include "Utils.h"

namespace OpenWifi {

	class KafkaManager *KafkaManager::instance_ = nullptr;

	KafkaManager::KafkaManager() noexcept:
		SubSystemServer("KafkaManager", "KAFKA-SVR", "openwifi.kafka")
	{
	}

	void KafkaManager::initialize(Poco::Util::Application & self) {
		SubSystemServer::initialize(self);
		KafkaEnabled_ = Daemon()->ConfigGetBool("openwifi.kafka.enable",false);
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
		ProducerThr_ = std::make_unique<std::thread>([this]() { this->ProducerThr(); });
		ConsumerThr_ = std::make_unique<std::thread>([this]() { this->ConsumerThr(); });
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

	void KafkaManager::ProducerThr() {
		cppkafka::Configuration Config({
										   { "client.id", Daemon()->ConfigGetString("openwifi.kafka.client.id") },
										   { "metadata.broker.list", Daemon()->ConfigGetString("openwifi.kafka.brokerlist") }
									   });
		SystemInfoWrapper_ = 	R"lit({ "system" : { "id" : )lit" +
								  	std::to_string(Daemon()->ID()) +
									R"lit( , "host" : ")lit" + Daemon()->PrivateEndPoint() +
									R"lit(" } , "payload" : )lit" ;
		cppkafka::Producer	Producer(Config);
		ProducerRunning_ = true;
		while(ProducerRunning_) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			try
			{
				SubMutexGuard G(ProducerMutex_);
				auto Num=0;
				while (!Queue_.empty()) {
					const auto M = Queue_.front();
					Producer.produce(
						cppkafka::MessageBuilder(M.Topic).key(M.Key).payload(M.PayLoad));
					Queue_.pop();
					Num++;
				}
				if(Num)
					Producer.flush();
			} catch (const cppkafka::HandleException &E ) {
				Logger_.warning(Poco::format("Caught a Kafka exception (producer): %s",std::string{E.what()}));
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
			}
		}
	}

	void KafkaManager::PartitionAssignment(const cppkafka::TopicPartitionList& partitions) {
		Logger_.information(Poco::format("Partition assigned: %Lu...",(uint64_t )partitions.front().get_partition()));
	}
	void KafkaManager::PartitionRevocation(const cppkafka::TopicPartitionList& partitions) {
		Logger_.information(Poco::format("Partition revocation: %Lu...",(uint64_t )partitions.front().get_partition()));
	}

	void KafkaManager::ConsumerThr() {
		cppkafka::Configuration Config({
										   { "client.id", Daemon()->ConfigGetString("openwifi.kafka.client.id") },
										   { "metadata.broker.list", Daemon()->ConfigGetString("openwifi.kafka.brokerlist") },
										   { "group.id", Daemon()->ConfigGetString("openwifi.kafka.group.id") },
										   { "enable.auto.commit", Daemon()->ConfigGetBool("openwifi.kafka.auto.commit",false) },
										   { "auto.offset.reset", "latest" } ,
										   { "enable.partition.eof", false }
									   });

		cppkafka::TopicConfiguration topic_config = {
			{ "auto.offset.reset", "smallest" }
		};

		// Now configure it to be the default topic config
		Config.set_default_topic_configuration(topic_config);

		cppkafka::Consumer Consumer(Config);
		Consumer.set_assignment_callback([this](cppkafka::TopicPartitionList& partitions) {
			if(!partitions.empty()) {
				Logger_.information(Poco::format("Partition assigned: %Lu...",
												 (uint64_t)partitions.front().get_partition()));
			}
		});
		Consumer.set_revocation_callback([this](const cppkafka::TopicPartitionList& partitions) {
			if(!partitions.empty()) {
				Logger_.information(Poco::format("Partition revocation: %Lu...",
												 (uint64_t)partitions.front().get_partition()));
			}
		});

        bool AutoCommit = Daemon()->ConfigGetBool("openwifi.kafka.auto.commit",false);
        auto BatchSize = Daemon()->ConfigGetInt("openwifi.kafka.consumer.batchsize",20);

        Types::StringVec    Topics;
		for(const auto &i:Notifiers_)
			Topics.push_back(i.first);

		Consumer.subscribe(Topics);

		ConsumerRunning_ = true;
		while(ConsumerRunning_) {
			try {
				std::vector<cppkafka::Message> MsgVec = Consumer.poll_batch(BatchSize, std::chrono::milliseconds(200));
				for(auto const &Msg:MsgVec) {
                    if (!Msg)
                        continue;
                    if (Msg.get_error()) {
                        if (!Msg.is_eof()) {
                            Logger_.error(Poco::format("Error: %s", Msg.get_error().to_string()));
                        }if(!AutoCommit)
                            Consumer.async_commit(Msg);
                        continue;
                    }
                    SubMutexGuard G(ConsumerMutex_);
                    auto It = Notifiers_.find(Msg.get_topic());
                    if (It != Notifiers_.end()) {
                        Types::TopicNotifyFunctionList &FL = It->second;
                        std::string Key{Msg.get_key()};
                        std::string Payload{Msg.get_payload()};
                        for (auto &F : FL) {
                            std::thread T(F.first, Key, Payload);
                            T.detach();
                        }
                    }
                    if (!AutoCommit)
                        Consumer.async_commit(Msg);
                }
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

	void KafkaManager::PostMessage(const std::string &topic, const std::string & key, const std::string &PayLoad, bool WrapMessage ) {
		if(KafkaEnabled_) {
			SubMutexGuard G(Mutex_);
			KMessage M{
				.Topic = topic,
				.Key = key,
				.PayLoad = WrapMessage ? WrapSystemId(PayLoad) : PayLoad };
			Queue_.push(M);
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
