//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"
#include "Poco/JSON/Object.h"
#include "framework/KafkaTopics.h"
#include "framework/OpenWifiTypes.h"
#include "framework/SubSystemServer.h"
#include "framework/utils.h"

#include "cppkafka/cppkafka.h"

namespace OpenWifi {

	class KafkaMessage : public Poco::Notification {
	  public:
		KafkaMessage(const char * Topic, const std::string &Key, const std::string &Payload)
			: Topic_(Topic), Key_(Key), Payload_(Payload) {}

		inline const char * Topic() { return Topic_; }
		inline const std::string &Key() { return Key_; }
		inline const std::string &Payload() { return Payload_; }

	  private:
		const char *Topic_;
		std::string Key_;
		std::string Payload_;
	};

	class KafkaProducer : public Poco::Runnable {
	  public:
		void run() override;
		void Start();
		void Stop();
		void Produce(const char *Topic, const std::string &Key, const std::string & Payload);

	  private:
		std::mutex Mutex_;
		Poco::Thread Worker_;
		mutable std::atomic_bool Running_ = false;
		Poco::NotificationQueue Queue_;
	};

	class KafkaConsumer : public Poco::Runnable {
	  public:
		void Start();
		void Stop();

	  private:
		std::mutex 				ConsumerMutex_;
		Types::NotifyTable 		Notifiers_;
		Poco::Thread 			Worker_;
		mutable std::atomic_bool Running_ = false;
		uint64_t 				FunctionId_ = 1;
		std::unique_ptr<cppkafka::ConsumerDispatcher> 	Dispatcher_;
		std::set<std::string>	Topics_;

		void run() override;
		friend class KafkaManager;
		std::uint64_t RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction &F);
		void UnregisterTopicWatcher(const std::string &Topic, int Id);
	};

	class KafkaManager : public SubSystemServer {
	  public:
		friend class KafkaConsumer;
		friend class KafkaProducer;

		inline void initialize(Poco::Util::Application &self) override;

		static auto instance() {
			static auto instance_ = new KafkaManager;
			return instance_;
		}

		int Start() override;
		void Stop() override;

		void PostMessage(const char *topic, const std::string &key,
						 const std::string &PayLoad, bool WrapMessage = true);
		void PostMessage(const char *topic, const std::string &key,
						 const Poco::JSON::Object &Object, bool WrapMessage = true);

		[[nodiscard]] std::string WrapSystemId(const std::string & PayLoad);
		[[nodiscard]] inline bool Enabled() const { return KafkaEnabled_; }
		inline std::uint64_t RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction &F) {
			return ConsumerThr_.RegisterTopicWatcher(Topic,F);
		}
		inline void UnregisterTopicWatcher(const std::string &Topic, uint64_t Id) {
			return ConsumerThr_.UnregisterTopicWatcher(Topic,Id);
		}

		std::uint64_t KafkaManagerMaximumPayloadSize() const { return MaxPayloadSize_; }

	  private:
		bool KafkaEnabled_ = false;
		std::string SystemInfoWrapper_;
		KafkaProducer ProducerThr_;
		KafkaConsumer ConsumerThr_;
		std::uint64_t MaxPayloadSize_ = 250000;

		void PartitionAssignment(const cppkafka::TopicPartitionList &partitions);
		void PartitionRevocation(const cppkafka::TopicPartitionList &partitions);

		KafkaManager() noexcept : SubSystemServer("KafkaManager", "KAFKA-SVR", "openwifi.kafka") {}
	};

	inline auto KafkaManager() { return KafkaManager::instance(); }

} // namespace OpenWifi
