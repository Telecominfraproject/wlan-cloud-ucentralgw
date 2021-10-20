//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//

#ifndef UCENTRALGW_KAFKAMANAGER_H
#define UCENTRALGW_KAFKAMANAGER_H

#include <queue>
#include <thread>

#include "SubSystemServer.h"
#include "OpenWifiTypes.h"

#include "cppkafka/cppkafka.h"

namespace OpenWifi {

	class KafkaManager : public SubSystemServer {
	  public:

		struct KMessage {
					std::string Topic,
								Key,
								PayLoad;
		};

		void initialize(Poco::Util::Application & self) override;
		static KafkaManager *instance() {
			if(instance_== nullptr)
				instance_ = new KafkaManager;
			return instance_;
		}

		void ProducerThr();
		void ConsumerThr();

		int Start() override;
		void Stop() override;

		void PostMessage(const std::string &topic, const std::string & key, const std::string &payload, bool WrapMessage = true);
		[[nodiscard]] std::string WrapSystemId(const std::string & PayLoad);
		[[nodiscard]] bool Enabled() { return KafkaEnabled_; }
		int RegisterTopicWatcher(const std::string &Topic, Types::TopicNotifyFunction & F);
		void UnregisterTopicWatcher(const std::string &Topic, int FunctionId);
		void WakeUp();
		void PartitionAssignment(const cppkafka::TopicPartitionList& partitions);
		void PartitionRevocation(const cppkafka::TopicPartitionList& partitions);

	  private:
		static KafkaManager 			*instance_;
		std::mutex 						ProducerMutex_;
		std::mutex						ConsumerMutex_;
		bool 							KafkaEnabled_ = false;
		std::atomic_bool 				ProducerRunning_ = false;
		std::atomic_bool 				ConsumerRunning_ = false;
		std::queue<KMessage>			Queue_;
		std::string 					SystemInfoWrapper_;
		std::unique_ptr<std::thread>	ConsumerThr_;
		std::unique_ptr<std::thread>	ProducerThr_;
		int                       		FunctionId_=1;
		Types::NotifyTable        		Notifiers_;
		std::unique_ptr<cppkafka::Configuration>    Config_;

		KafkaManager() noexcept;
	};

	inline KafkaManager * KafkaManager() { return KafkaManager::instance(); }
}	// NameSpace

#endif // UCENTRALGW_KAFKAMANAGER_H
