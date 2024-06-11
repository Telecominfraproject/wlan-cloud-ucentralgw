//
// Created by stephane bourque on 2022-10-25.
//

#include "KafkaManager.h"

#include "fmt/format.h"
#include "framework/MicroServiceFuncs.h"
#include "cppkafka/utils/consumer_dispatcher.h"

namespace OpenWifi {

	void KafkaLoggerFun([[maybe_unused]] cppkafka::KafkaHandleBase &handle, int level,
						const std::string &facility, const std::string &message) {
		switch ((cppkafka::LogLevel)level) {
		case cppkafka::LogLevel::LogNotice: {
			poco_notice(KafkaManager()->Logger(),
						fmt::format("kafka-log: facility: {} message: {}", facility, message));
		} break;
		case cppkafka::LogLevel::LogDebug: {
			poco_debug(KafkaManager()->Logger(),
					   fmt::format("kafka-log: facility: {} message: {}", facility, message));
		} break;
		case cppkafka::LogLevel::LogInfo: {
			poco_information(KafkaManager()->Logger(),
							 fmt::format("kafka-log: facility: {} message: {}", facility, message));
		} break;
		case cppkafka::LogLevel::LogWarning: {
			poco_warning(KafkaManager()->Logger(),
						 fmt::format("kafka-log: facility: {} message: {}", facility, message));
		} break;
		case cppkafka::LogLevel::LogAlert:
		case cppkafka::LogLevel::LogCrit: {
			poco_critical(KafkaManager()->Logger(),
						  fmt::format("kafka-log: facility: {} message: {}", facility, message));
		} break;
		case cppkafka::LogLevel::LogErr:
		case cppkafka::LogLevel::LogEmerg:
		default: {
			poco_error(KafkaManager()->Logger(),
					   fmt::format("kafka-log: facility: {} message: {}", facility, message));
		} break;
		}
	}

	inline void KafkaErrorFun([[maybe_unused]] cppkafka::KafkaHandleBase &handle, int error,
							  const std::string &reason) {
		poco_error(KafkaManager()->Logger(),
				   fmt::format("kafka-error: {}, reason: {}", error, reason));
	}

	inline void AddKafkaSecurity(cppkafka::Configuration &Config) {
		auto CA = MicroServiceConfigGetString("openwifi.kafka.ssl.ca.location", "");
		auto Certificate =
			MicroServiceConfigGetString("openwifi.kafka.ssl.certificate.location", "");
		auto Key = MicroServiceConfigGetString("openwifi.kafka.ssl.key.location", "");
		auto Password = MicroServiceConfigGetString("openwifi.kafka.ssl.key.password", "");

		if (CA.empty() || Certificate.empty() || Key.empty())
			return;

		Config.set("ssl.ca.location", CA);
		Config.set("ssl.certificate.location", Certificate);
		Config.set("ssl.key.location", Key);
		if (!Password.empty())
			Config.set("ssl.key.password", Password);
	}

	void KafkaManager::initialize(Poco::Util::Application &self) {
		SubSystemServer::initialize(self);
		KafkaEnabled_ = MicroServiceConfigGetBool("openwifi.kafka.enable", false);
	}

	inline void KafkaProducer::run() {
		Poco::Logger &Logger_ =
			Poco::Logger::create("KAFKA-PRODUCER", KafkaManager()->Logger().getChannel());
		poco_information(Logger_, "Starting...");

		Utils::SetThreadName("Kafka:Prod");
		cppkafka::Configuration Config(
			{{"client.id", MicroServiceConfigGetString("openwifi.kafka.client.id", "")},
			 {"metadata.broker.list",MicroServiceConfigGetString("openwifi.kafka.brokerlist", "")} // ,
			 // {"send.buffer.bytes", KafkaManager()->KafkaManagerMaximumPayloadSize() }
			}
 		);

		AddKafkaSecurity(Config);

		Config.set_log_callback(KafkaLoggerFun);
		Config.set_error_callback(KafkaErrorFun);

		KafkaManager()->SystemInfoWrapper_ =
			R"lit({ "system" : { "id" : )lit" + std::to_string(MicroServiceID()) +
			R"lit( , "host" : ")lit" + MicroServicePrivateEndPoint() +
			R"lit(" } , "payload" : )lit";

		cppkafka::Producer Producer(Config);
		Running_ = true;

		Poco::AutoPtr<Poco::Notification> Note(Queue_.waitDequeueNotification());
		while (Note && Running_) {
			try {
				auto Msg = dynamic_cast<KafkaMessage *>(Note.get());
				if (Msg != nullptr) {
					auto NewMessage = cppkafka::MessageBuilder(Msg->Topic());
					NewMessage.key(Msg->Key());
					NewMessage.partition(0);
					NewMessage.payload(Msg->Payload());
					Producer.produce(NewMessage);
					Producer.poll((std::chrono::milliseconds) 0);
				}
			} catch (const cppkafka::HandleException &E) {
				poco_warning(Logger_,
							 fmt::format("Caught a Kafka exception (producer): {}", E.what()));
			} catch (const Poco::Exception &E) {
				Logger_.log(E);
			} catch (...) {
				poco_error(Logger_, "std::exception");
			}
			if (Queue_.size() == 0) {
				// message queue is empty, flush all previously sent messages
				Producer.flush();
			}
			Note = Queue_.waitDequeueNotification();
		}
		poco_information(Logger_, "Stopped...");
	}

	inline void KafkaConsumer::run() {
		Utils::SetThreadName("Kafka:Cons");

		Poco::Logger &Logger_ =
			Poco::Logger::create("KAFKA-CONSUMER", KafkaManager()->Logger().getChannel());

		poco_information(Logger_, "Starting...");

		cppkafka::Configuration Config(
			{{"client.id", MicroServiceConfigGetString("openwifi.kafka.client.id", "")},
			 {"metadata.broker.list", MicroServiceConfigGetString("openwifi.kafka.brokerlist", "")},
			 {"group.id", MicroServiceConfigGetString("openwifi.kafka.group.id", "")},
			 {"enable.auto.commit", MicroServiceConfigGetBool("openwifi.kafka.auto.commit", false)},
			 {"auto.offset.reset", "latest"},
			 {"enable.partition.eof", false}});

		AddKafkaSecurity(Config);

		Config.set_log_callback(KafkaLoggerFun);
		Config.set_error_callback(KafkaErrorFun);

		cppkafka::TopicConfiguration topic_config = {{"auto.offset.reset", "smallest"}};

		// Now configure it to be the default topic config
		Config.set_default_topic_configuration(topic_config);

		cppkafka::Consumer Consumer(Config);
		Consumer.set_assignment_callback([&](cppkafka::TopicPartitionList &partitions) {
			if (!partitions.empty()) {
				poco_information(Logger_, fmt::format("Partition assigned: {}...",
													  partitions.front().get_partition()));
			}
		});
		Consumer.set_revocation_callback([&](const cppkafka::TopicPartitionList &partitions) {
			if (!partitions.empty()) {
				poco_information(Logger_, fmt::format("Partition revocation: {}...",
													  partitions.front().get_partition()));
			}
		});

		// bool AutoCommit = MicroServiceConfigGetBool("openwifi.kafka.auto.commit", false);
		// auto BatchSize = MicroServiceConfigGetInt("openwifi.kafka.consumer.batchsize", 100);

		Types::StringVec Topics;
		std::for_each(Topics_.begin(),Topics_.end(),
					  [&](const std::string & T) { Topics.emplace_back(T); });
		Consumer.subscribe(Topics);

		Running_ = true;
		std::vector<cppkafka::Message> MsgVec;

		Dispatcher_ = std::make_unique<cppkafka::ConsumerDispatcher>(Consumer);

		Dispatcher_->run(
			// Callback executed whenever a new message is consumed
			[&](cppkafka::Message msg) {
				// Print the key (if any)
				std::lock_guard G(ConsumerMutex_);
				auto It = Notifiers_.find(msg.get_topic());
				if (It != Notifiers_.end()) {
					const auto &FL = It->second;
					for (const auto &[CallbackFunc, _] : FL) {
						try {
							CallbackFunc(msg.get_key(), msg.get_payload());
						} catch(const Poco::Exception &E) {

						} catch(...) {

						}
					}
				}
				Consumer.commit(msg);
			},
			// Whenever there's an error (other than the EOF soft error)
			[&Logger_](cppkafka::Error error) {
				poco_warning(Logger_,fmt::format("Error: {}", error.to_string()));
			},
			// Whenever EOF is reached on a partition, print this
			[&Logger_](cppkafka::ConsumerDispatcher::EndOfFile, const cppkafka::TopicPartition& topic_partition) {
				poco_debug(Logger_,fmt::format("Partition {} EOF", topic_partition.get_partition()));
			}
		);

		Consumer.unsubscribe();
		poco_information(Logger_, "Stopped...");
	}

	void KafkaProducer::Start() {
		if (!Running_) {
			Running_ = true;
			Worker_.start(*this);
		}
	}

	void KafkaProducer::Stop() {
		if (Running_) {
			Running_ = false;
			Queue_.wakeUpAll();
			Worker_.join();
		}
	}

	void KafkaProducer::Produce(const char *Topic, const std::string &Key,
								const std::string &Payload) {
		std::lock_guard G(Mutex_);
		Queue_.enqueueNotification(new KafkaMessage(Topic, Key, Payload));
	}

	void KafkaConsumer::Start() {
		if (!Running_) {
			Worker_.start(*this);
		}
	}

	void KafkaConsumer::Stop() {
		if (Running_) {
			Running_ = false;
			if(Dispatcher_) {
				Dispatcher_->stop();
			}
			Worker_.join();
		}
	}

	std::uint64_t KafkaConsumer::RegisterTopicWatcher(const std::string &Topic,
											   Types::TopicNotifyFunction &F) {
		std::lock_guard G(ConsumerMutex_);
		auto It = Notifiers_.find(Topic);
		if (It == Notifiers_.end()) {
			Types::TopicNotifyFunctionList L;
			L.emplace(L.end(), std::make_pair(F, FunctionId_));
			Notifiers_[Topic] = std::move(L);
		} else {
			It->second.emplace(It->second.end(), std::make_pair(F, FunctionId_));
		}
		Topics_.insert(Topic);
		return FunctionId_++;
	}

	void KafkaConsumer::UnregisterTopicWatcher(const std::string &Topic, int Id) {
		std::lock_guard G(ConsumerMutex_);
		auto It = Notifiers_.find(Topic);
		if (It != Notifiers_.end()) {
			Types::TopicNotifyFunctionList &L = It->second;
			for (auto it = L.begin(); it != L.end(); it++)
				if (it->second == Id) {
					L.erase(it);
					break;
				}
		}
	}

	int KafkaManager::Start() {
		if (!KafkaEnabled_)
			return 0;
		MaxPayloadSize_ = MicroServiceConfigGetInt("openwifi.kafka.max.payload", 250000);
		ConsumerThr_.Start();
		ProducerThr_.Start();
		return 0;
	}

	void KafkaManager::Stop() {
		if (KafkaEnabled_) {
			poco_information(Logger(), "Stopping...");
			ProducerThr_.Stop();
			ConsumerThr_.Stop();
			poco_information(Logger(), "Stopped...");
			return;
		}
	}

	void KafkaManager::PostMessage(const char *topic, const std::string &key,
								   const std::string & PayLoad, bool WrapMessage) {
		if (KafkaEnabled_) {
			ProducerThr_.Produce(topic, key, WrapMessage ? WrapSystemId(PayLoad) : PayLoad);
		}
	}

	void KafkaManager::PostMessage(const char *topic, const std::string &key,
					 const Poco::JSON::Object &Object, bool WrapMessage) {
		if (KafkaEnabled_) {
			std::ostringstream ObjectStr;
			Object.stringify(ObjectStr);
			ProducerThr_.Produce(topic, key, WrapMessage ? WrapSystemId(ObjectStr.str()) : ObjectStr.str());
		}
	}

	[[nodiscard]] std::string KafkaManager::WrapSystemId(const std::string & PayLoad) {
		return fmt::format(	R"lit({{ "system" : {{ "id" : {}, "host" : "{}" }}, "payload" : {} }})lit",
						   MicroServiceID(), MicroServicePrivateEndPoint(), PayLoad ) ;
	}

	void KafkaManager::PartitionAssignment(const cppkafka::TopicPartitionList &partitions) {
		poco_information(
			Logger(), fmt::format("Partition assigned: {}...", partitions.front().get_partition()));
	}

	void KafkaManager::PartitionRevocation(const cppkafka::TopicPartitionList &partitions) {
		poco_information(Logger(), fmt::format("Partition revocation: {}...",
											   partitions.front().get_partition()));
	}

} // namespace OpenWifi