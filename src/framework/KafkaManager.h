//
// Created by stephane bourque on 2022-10-25.
//

#pragma once

#include "Poco/Notification.h"
#include "Poco/NotificationQueue.h"

#include "framework/SubSystemServer.h"
#include "framework/OpenWifiTypes.h"
#include "framework/utils.h"
#include "framework/KafkaTopics.h"

#include "cppkafka/cppkafka.h"

namespace OpenWifi {

	class KafkaMessage: public Poco::Notification {
	  public:
		KafkaMessage( const std::string &Topic, const std::string &Key, const std::string & Payload) :
			Topic_(Topic), Key_(Key), Payload_(Payload) {
		}

		inline const std::string & Topic() { return Topic_; }
		inline const std::string & Key() { return Key_; }
		inline const std::string & Payload() { return Payload_; }

	  private:
		std::string	Topic_;
		std::string	Key_;
		std::string	Payload_;
	};

	class KafkaProducer : public Poco::Runnable {
        public:
            void run () override;
            void Start();
            void Stop();
            void Produce(const std::string &Topic, const std::string &Key, const std::string &Payload);

	  private:
            std::recursive_mutex  		Mutex_;
            Poco::Thread        		Worker_;
            mutable std::atomic_bool    Running_=false;
            Poco::NotificationQueue		Queue_;
    };

	class KafkaConsumer : public Poco::Runnable {
	  public:
		void run() override;
		void Start();
		void Stop();

	  private:
		std::recursive_mutex  	    Mutex_;
		Poco::Thread        	    Worker_;
		mutable std::atomic_bool    Running_=false;
	};

	class KafkaDispatcher : public Poco::Runnable {
	  public:
		void Start();
		void Stop();
		auto RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction &F);
		void UnregisterTopicWatcher(const std::string &Topic, int Id);
		void Dispatch(const std::string &Topic, const std::string &Key, const std::string &Payload);
		void run() override;
		void Topics(std::vector<std::string> &T);

	  private:
		std::recursive_mutex  		Mutex_;
		Types::NotifyTable      	Notifiers_;
		Poco::Thread        		Worker_;
		mutable std::atomic_bool    Running_=false;
		uint64_t          			FunctionId_=1;
		Poco::NotificationQueue		Queue_;
	};

	class KafkaManager : public SubSystemServer {
	  public:

		friend class KafkaConsumer;
		friend class KafkaProducer;

		inline void initialize(Poco::Util::Application & self) override;

		static auto instance() {
			static auto instance_ = new KafkaManager;
			return instance_;
		}

		int Start() override;
		void Stop() override;

		void PostMessage(const std::string &topic, const std::string & key, const std::string &PayLoad, bool WrapMessage = true  );
		void Dispatch(const std::string &Topic, const std::string & Key, const std::string &Payload);
		[[nodiscard]] std::string WrapSystemId(const std::string & PayLoad);
		[[nodiscard]] inline bool Enabled() const { return KafkaEnabled_; }
		uint64_t RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction &F);
		void UnregisterTopicWatcher(const std::string &Topic, uint64_t Id);
		void Topics(std::vector<std::string> &T);

	  private:
		bool 				KafkaEnabled_ = false;
		std::string 		SystemInfoWrapper_;
		KafkaProducer       ProducerThr_;
        KafkaConsumer       ConsumerThr_;
		KafkaDispatcher     Dispatcher_;

		void PartitionAssignment(const cppkafka::TopicPartitionList& partitions);
		void PartitionRevocation(const cppkafka::TopicPartitionList& partitions);

		KafkaManager() noexcept:
			SubSystemServer("KafkaManager", "KAFKA-SVR", "openwifi.kafka") {
		}
	};

	inline auto KafkaManager() { return KafkaManager::instance(); }

} // namespace OpenWifi

